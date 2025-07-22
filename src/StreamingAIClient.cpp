#include "StreamingAIClient.h"
#include "ConsoleMessageRepository.h"
#include "AuthenticationManager.h"
#include "AICommandDispatcher.h"
#include "Application.h"
#include "ElementModel.h"
#include "SelectionManager.h"
#include "Element.h"
#include "CanvasElement.h"
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QFile>
#include <QUrl>
#include <QUrlQuery>
#include <QDebug>
#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QUuid>
#include <QRegularExpression>

StreamingAIClient::StreamingAIClient(AuthenticationManager *authManager, Application *app, QObject *parent)
    : QObject(parent), m_webSocket(std::make_unique<QWebSocket>()), m_authManager(authManager), m_application(app), m_commandDispatcher(std::make_unique<AICommandDispatcher>(app, this)), m_isConnected(false), m_connectionAckReceived(false), m_reconnectAttempts(0)
{
    // Connect to token refresh
    if (m_authManager)
    {
        connect(m_authManager, &AuthenticationManager::tokensRefreshed,
                this, &StreamingAIClient::onTokensRefreshed,
                Qt::UniqueConnection);
    }

    // Command dispatcher logs
    connect(m_commandDispatcher.get(), &AICommandDispatcher::commandExecuted,
            this, [](const QString &description, bool success)
            {
        if (success) ConsoleMessageRepository::instance()->addInfo(description); });
    connect(m_commandDispatcher.get(), &AICommandDispatcher::commandError,
            this, [](const QString &error)
            { ConsoleMessageRepository::instance()->addError(QString("Command error: %1").arg(error)); });

    // WebSocket signals
    connect(m_webSocket.get(), &QWebSocket::connected, this, &StreamingAIClient::onConnected);
    connect(m_webSocket.get(), &QWebSocket::disconnected, this, &StreamingAIClient::onDisconnected);
    connect(m_webSocket.get(), &QWebSocket::textMessageReceived, this, &StreamingAIClient::onTextMessageReceived);
    connect(m_webSocket.get(), &QWebSocket::errorOccurred, this, &StreamingAIClient::onError);

    // Ping timer
    m_pingTimer = new QTimer(this);
    m_pingTimer->setInterval(PING_INTERVAL_MS);
    connect(m_pingTimer, &QTimer::timeout, this, &StreamingAIClient::sendPing);
    
    // Loading indicator timer
    m_loadingTimer = new QTimer(this);
    m_loadingTimer->setInterval(LOADING_ANIMATION_MS);
    connect(m_loadingTimer, &QTimer::timeout, this, &StreamingAIClient::updateLoadingIndicator);
    m_loadingDots = 0;
}

StreamingAIClient::~StreamingAIClient()
{
    disconnectFromWebSocket();
}

void StreamingAIClient::sendMessage(const QString &description)
{
    m_pendingMessage = description;
    m_accumulatedResponse.clear();
    m_accumulatedCommands.clear();

    QString authToken = getAuthToken();
    
    // Check if we have a valid token, if not trigger authentication
    if (authToken.isEmpty() && m_authManager) {
        ConsoleMessageRepository::instance()->addError("Authentication required. Please log in.");
        // Trigger authentication or token refresh
        if (!m_authManager->isAuthenticated()) {
            m_authManager->login();
        } else {
            // Try to refresh the token
            m_authManager->refreshAccessToken();
        }
        return;
    }

    if (!m_isConnected)
    {
        connectToWebSocket();
    }
    else
    {
        createConversation();
    }
}

bool StreamingAIClient::isConnected() const
{
    return m_isConnected && m_connectionAckReceived;
}

void StreamingAIClient::handleUserContinuationResponse(bool accepted, const QString &feedback)
{
    // Hide the prompt options
    ConsoleMessageRepository::instance()->setShowAIPrompt(false);
    
    if (!m_currentConversationId.isEmpty() && !m_pendingContinuationContext.isEmpty())
    {
        QString message;
        if (accepted) {
            // User accepted, continue with the planned action
            message = "Continue: " + m_pendingContinuationContext;
            ConsoleMessageRepository::instance()->addInfo("Continuing with AI plan...");
        } else {
            // User provided feedback, adjust the plan
            message = "User feedback: " + feedback + ". Please adjust the plan for: " + m_pendingContinuationContext;
            ConsoleMessageRepository::instance()->addInfo("Adjusting AI plan based on feedback...");
        }
        
        // Send the continuation or feedback message
        sendConversationMessage(m_currentConversationId, message);
        
        // Clear the pending context
        m_pendingContinuationContext.clear();
    }
}

void StreamingAIClient::connectToWebSocket()
{
    QString url = getWebSocketUrl();
    if (url.isEmpty())
    {
        emit errorOccurred("Failed to construct WebSocket URL");
        return;
    }

    // Silently connect to AI service

    QNetworkRequest request;
    request.setUrl(QUrl(url));
    // AWS AppSync uses graphql-ws protocol
    request.setRawHeader("Sec-WebSocket-Protocol", "graphql-ws");
    
    // Add additional headers that might be needed
    request.setRawHeader("Origin", "app://cubit");
    request.setRawHeader("User-Agent", "Cubit/1.0");

    qDebug() << "Opening WebSocket with URL:" << url;
    qDebug() << "WebSocket protocol: graphql-ws";
    m_webSocket->open(request);
}

void StreamingAIClient::disconnectFromWebSocket()
{
    m_pingTimer->stop();
    if (m_webSocket->state() != QAbstractSocket::UnconnectedState)
    {
        m_webSocket->close();
    }
}

QString StreamingAIClient::getWebSocketUrl() const
{
    QString endpoint = getApiEndpoint();
    if (endpoint.isEmpty())
    {
        qWarning() << "No AppSync endpoint found!";
        return {};
    }

    qDebug() << "[DEBUG] Original AppSync endpoint:" << endpoint;

    QString wsEndpoint = endpoint;
    wsEndpoint.replace("https://", "wss://");
    wsEndpoint.replace("appsync-api", "appsync-realtime-api");
    if (wsEndpoint.endsWith("/graphql"))
        wsEndpoint.chop(8);
    wsEndpoint += "/graphql";

    qDebug() << "[DEBUG] Realtime WebSocket endpoint:" << wsEndpoint;

    QJsonObject authHeader;
    QUrl endpointUrl(endpoint);
    authHeader["host"] = endpointUrl.host();

    QString token = getAuthToken();
    if (!token.isEmpty())
        authHeader["Authorization"] = token;
    else
        qWarning() << "[DEBUG] No Cognito token available for websocket!";

    qDebug() << "[DEBUG] WebSocket authHeader JSON:"
             << QJsonDocument(authHeader).toJson(QJsonDocument::Compact);

    QByteArray headerJson = QJsonDocument(authHeader).toJson(QJsonDocument::Compact);
    QString encodedHeader = QString::fromUtf8(headerJson.toBase64());
    QString encodedPayload = "e30="; // "{}" -> "e30="

    QString finalUrl = QString("%1?header=%2&payload=%3")
                           .arg(wsEndpoint)
                           .arg(encodedHeader)
                           .arg(encodedPayload);

    QByteArray decodedHeader = QByteArray::fromBase64(encodedHeader.toUtf8());
    qDebug() << "[DEBUG] Decoded Base64 header JSON (sanity check):" << decodedHeader;
    qDebug() << "[DEBUG] Final WebSocket URL:" << finalUrl;

    return finalUrl;
}

QString StreamingAIClient::getApiEndpoint() const
{
    QFile file("amplify_outputs.json");
    if (!file.open(QIODevice::ReadOnly))
    {
        qWarning() << "Failed to open amplify_outputs.json";
        return QString();
    }
    QByteArray data = file.readAll();
    QJsonDocument doc = QJsonDocument::fromJson(data);
    if (doc.isNull())
    {
        qWarning() << "Failed to parse amplify_outputs.json";
        return QString();
    }
    QJsonObject root = doc.object();
    if (root.contains("data") && root["data"].isObject())
    {
        QJsonObject dataObj = root["data"].toObject();
        if (dataObj.contains("url"))
            return dataObj["url"].toString();
    }
    return QString();
}

QString StreamingAIClient::getAuthToken() const
{
    if (m_authManager && m_authManager->isAuthenticated())
    {
        QString idToken = m_authManager->getIdToken();
        if (!idToken.isEmpty())
        {
            // Using Cognito ID token for authentication
            return idToken;
        }
    }
    qDebug() << "No Cognito ID token available!";
    return QString();
}

void StreamingAIClient::onConnected()
{
    // WebSocket connected
    m_isConnected = true;
    m_reconnectAttempts = 0;
    sendConnectionInit();
}

void StreamingAIClient::onDisconnected()
{
    // WebSocket disconnected
    m_isConnected = false;
    m_connectionAckReceived = false;
    m_pingTimer->stop();
    emit disconnected();

    if (!m_pendingMessage.isEmpty() && m_reconnectAttempts < MAX_RECONNECT_ATTEMPTS)
    {
        m_reconnectAttempts++;
        // Attempting reconnection
        QTimer::singleShot(1000, this, &StreamingAIClient::connectToWebSocket);
    }
}

void StreamingAIClient::onTextMessageReceived(const QString &message)
{
    // qDebug() << "WebSocket received:" << message;
    QJsonDocument doc = QJsonDocument::fromJson(message.toUtf8());
    if (doc.isNull())
    {
        qWarning() << "Received invalid JSON from WebSocket";
        return;
    }
    QJsonObject obj = doc.object();
    QString type = obj["type"].toString();

    if (type == "connection_ack")
        handleConnectionAck();
    else if (type == "start_ack") {
        // Subscription acknowledged
        // Log the subscription ID for debugging
        QString subId = obj["id"].toString();
        // qDebug() << "Subscription started with ID:" << subId;
    }
    else if (type == "data")
        handleSubscriptionData(obj["payload"].toObject());
    else if (type == "complete")
        handleComplete(obj["id"].toString());
    else if (type == "error" || type == "connection_error") {
        handleError(obj["payload"].toObject());
        // Additional error logging
        qDebug() << "Full error message:" << QJsonDocument(obj).toJson(QJsonDocument::Compact);
        
        // Check if this is a subscription-specific error
        QString errorType = "";
        QString errorMessage = "";
        if (obj["payload"].isObject()) {
            QJsonObject payload = obj["payload"].toObject();
            errorType = payload["errorType"].toString();
            errorMessage = payload["message"].toString();
        }
        
        if (errorType == "UnsupportedOperation" && errorMessage.contains("not supported through the realtime channel")) {
            ConsoleMessageRepository::instance()->addError("Subscription format error: The subscription request format is not compatible with AWS AppSync realtime channel");
            qDebug() << "This typically means the subscription message format is incorrect for AppSync";
        }
    }
    else if (type == "ka")
    {
    } // Keepalive
    else
        qDebug() << "Unknown WebSocket message type:" << type;
}

void StreamingAIClient::onError(QAbstractSocket::SocketError error)
{
    Q_UNUSED(error)
    QString errorMsg = QString("WebSocket error: %1").arg(m_webSocket->errorString());
    ConsoleMessageRepository::instance()->addError(errorMsg);
    emit errorOccurred(errorMsg);
}

void StreamingAIClient::onTokensRefreshed()
{
    // Tokens refreshed, reconnecting
    qDebug() << "Tokens refreshed in StreamingAIClient, reconnecting WebSocket";
    disconnectFromWebSocket();
    
    // If we have a pending message, reconnect and retry
    if (!m_pendingMessage.isEmpty()) {
        connectToWebSocket();
    }
}

void StreamingAIClient::sendPing()
{
    if (m_isConnected)
        m_webSocket->ping();
}

void StreamingAIClient::sendConnectionInit()
{
    QJsonObject message;
    message["type"] = "connection_init";
    QJsonDocument doc(message);
    QString jsonMessage = doc.toJson(QJsonDocument::Compact);
    // qDebug() << "Sending connection_init:" << jsonMessage;
    m_webSocket->sendTextMessage(jsonMessage);
}

void StreamingAIClient::handleConnectionAck()
{
    m_connectionAckReceived = true;
    m_pingTimer->start();
    emit connected();

    if (!m_pendingMessage.isEmpty())
    {
        createConversation();
    }
}

void StreamingAIClient::createConversation()
{
    // Creating conversation
    QString mutation = R"(
        mutation CreateConversation($input: CreateConversationCubitChatInput!) {
            createConversationCubitChat(input: $input) {
                id
                createdAt
                updatedAt
            }
        }
    )";

    QNetworkAccessManager *manager = new QNetworkAccessManager(this);
    QNetworkRequest request((QUrl(getApiEndpoint())));
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");

    QString authToken = getAuthToken();
    if (!authToken.isEmpty())
        request.setRawHeader("Authorization", authToken.toUtf8());

    QJsonObject variables;
    QJsonObject input;
    variables["input"] = input;
    QJsonObject body;
    body["query"] = mutation;
    body["variables"] = variables;
    QNetworkReply *reply = manager->post(request, QJsonDocument(body).toJson(QJsonDocument::Compact));

    connect(reply, &QNetworkReply::finished, this, [this, reply, manager]()
            {
        if (reply->error() == QNetworkReply::NoError) {
            QByteArray responseData = reply->readAll();
            qDebug() << "CreateConversation response:" << responseData;
            QJsonDocument responseDoc = QJsonDocument::fromJson(responseData);
            QJsonObject response = responseDoc.object();
            if (response.contains("data")) {
                QJsonObject data = response["data"].toObject();
                if (data.contains("createConversationCubitChat")) {
                    QJsonObject conv = data["createConversationCubitChat"].toObject();
                    m_currentConversationId = conv["id"].toString();

                    // First subscribe to responses, then send message
                    subscribeToResponses(m_currentConversationId);
                    
                    // Add a small delay to ensure subscription is established
                    QTimer::singleShot(500, this, [this]() {
                        sendConversationMessage(m_currentConversationId, m_pendingMessage);
                    });
                }
            }
        } else {
            ConsoleMessageRepository::instance()->addError(
                QString("Failed to create conversation: %1").arg(reply->errorString()));
        }
        reply->deleteLater();
        manager->deleteLater(); });
}

void StreamingAIClient::subscribeToResponses(const QString &conversationId)
{
    m_currentSubscriptionId = QUuid::createUuid().toString(QUuid::WithoutBraces);

    // Using the correct subscription name from amplify_outputs.json
    QString query = R"(
        subscription OnCreateAssistantResponseCubitChat($conversationId: ID!) {
            onCreateAssistantResponseCubitChat(conversationId: $conversationId) {
                id
                owner
                conversationId
                associatedUserMessageId
                contentBlockIndex
                contentBlockText
                contentBlockDeltaIndex
                contentBlockDoneAtIndex
                stopReason
                errors {
                    message
                    errorType
                }
                p
            }
        }
    )";

    // Pass conversationId directly as per the schema
    QJsonObject vars;
    vars["conversationId"] = conversationId;
    
    sendSubscription(m_currentSubscriptionId, query, vars);
    startLoadingIndicator();
    emit streamingStarted();
}

void StreamingAIClient::sendSubscription(const QString &subId, const QString &query, const QJsonObject &vars)
{
    // AWS AppSync uses a specific message format for subscriptions
    QJsonObject msg;
    msg["id"] = subId;
    msg["type"] = "start";
    
    // The payload contains the GraphQL subscription data
    QJsonObject payload;
    
    // Create the data object with query and variables
    QJsonObject data;
    data["query"] = query;
    data["variables"] = vars;
    
    // For AppSync, the data must be stringified - this is critical!
    QString dataStr = QJsonDocument(data).toJson(QJsonDocument::Compact);
    payload["data"] = dataStr;
    
    // Add authorization in extensions - this MUST be in the payload
    QJsonObject extensions;
    QJsonObject authorization;
    
    // Get the host from the API endpoint
    QUrl endpointUrl(getApiEndpoint());
    authorization["host"] = endpointUrl.host();
    
    // Add the authorization token
    QString token = getAuthToken();
    if (!token.isEmpty()) {
        authorization["Authorization"] = token;
    } else {
        qWarning() << "No authorization token available for subscription!";
    }
    
    extensions["authorization"] = authorization;
    payload["extensions"] = extensions;
    
    msg["payload"] = payload;
    
    QString msgJson = QJsonDocument(msg).toJson(QJsonDocument::Compact);
    // qDebug() << "Sending subscription:" << msgJson;
    // qDebug() << "Subscription data field:" << dataStr;
    // qDebug() << "Authorization host:" << authorization["host"].toString();
    // qDebug() << "Has auth token:" << !token.isEmpty();
    
    // Subscribing to AI responses
    
    m_webSocket->sendTextMessage(msgJson);
}

void StreamingAIClient::sendConversationMessage(const QString &conversationId, const QString &message)
{
    // Sending message to conversation

    // Using the Amplify Conversation API send message format
    QString mutation = R"(
        mutation SendMessageCubitChat($conversationId: ID!, $content: [AmplifyAIContentBlockInput!]) {
            CubitChat(conversationId: $conversationId, content: $content) {
                conversationId
                id
                content { text }
                role
                createdAt
            }
        }
    )";

    // Create content block with canvas state included
    QJsonObject textBlock;
    textBlock["text"] = message + "\n\nCanvas State: " + getCanvasState();
    QJsonArray contentArr;
    contentArr.append(textBlock);

    QJsonObject vars;
    vars["conversationId"] = conversationId;
    vars["content"] = contentArr;
    
    QJsonObject body;
    body["query"] = mutation;
    body["variables"] = vars;

    QNetworkAccessManager *manager = new QNetworkAccessManager(this);
    QNetworkRequest req((QUrl(getApiEndpoint())));
    req.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
    QString token = getAuthToken();
    if (!token.isEmpty())
        req.setRawHeader("Authorization", token.toUtf8());

    QNetworkReply *reply = manager->post(req, QJsonDocument(body).toJson(QJsonDocument::Compact));
    connect(reply, &QNetworkReply::finished, this, [this, reply, manager]()
            {
        QByteArray resp = reply->readAll();
        // qDebug() << "SendMessage response:" << resp;
        if (reply->error() != QNetworkReply::NoError) {
            ConsoleMessageRepository::instance()->addError(
                QString("Failed to send message: %1\nResponse: %2").arg(reply->errorString()).arg(QString(resp)));
        } else {
            // Message sent successfully
            // parse for userMessageId
            QJsonDocument doc = QJsonDocument::fromJson(resp);
            QJsonObject root = doc.object();
            if (root.contains("errors") && root["errors"].isArray()) {
                // Handle GraphQL errors
                QJsonArray errors = root["errors"].toArray();
                for (const auto& error : errors) {
                    QJsonObject errObj = error.toObject();
                    ConsoleMessageRepository::instance()->addError(
                        QString("GraphQL Error: %1").arg(errObj["message"].toString()));
                }
            } else if (root.contains("data")) {
                QJsonObject dataObj = root["data"].toObject();
                if (dataObj.contains("CubitChat")) {
                    QJsonObject msgObj = dataObj["CubitChat"].toObject();
                    QString userMsgId = msgObj["id"].toString();
                    // The Amplify backend will automatically trigger AI response
                    // Message sent, AI response will stream automatically
                }
            }
        }
        reply->deleteLater();
        manager->deleteLater(); });
}

// Removed triggerAIResponse - backend Lambda now handles AI streaming automatically

void StreamingAIClient::handleSubscriptionData(const QJsonObject &payload)
{
    // qDebug() << "Handling subscription data:" << QJsonDocument(payload).toJson(QJsonDocument::Compact);
    if (!payload.contains("data"))
        return;
    QJsonObject data = payload["data"].toObject();
    
    // Handle Amplify Conversation API streaming response
    if (data.contains("onCreateAssistantResponseCubitChat"))
    {
        QJsonObject resp = data["onCreateAssistantResponseCubitChat"].toObject();
        
        // Extract text content from contentBlockText
        QString chunk = resp["contentBlockText"].toString();
        int contentBlockIndex = resp["contentBlockIndex"].toInt(-1);
        int contentBlockDoneAtIndex = resp["contentBlockDoneAtIndex"].toInt(-1);
        QString stopReason = resp["stopReason"].toString();
        
        // Check if there are errors
        if (resp.contains("errors") && resp["errors"].isArray()) {
            QJsonArray errors = resp["errors"].toArray();
            for (const auto& error : errors) {
                QJsonObject errObj = error.toObject();
                QString errorMsg = QString("AI Error: %1 (%2)")
                    .arg(errObj["message"].toString())
                    .arg(errObj["errorType"].toString());
                ConsoleMessageRepository::instance()->addError(errorMsg);
            }
            return;
        }
        
        // Process the streaming response
        // Only consider it complete when we receive the final stopReason
        bool isComplete = !stopReason.isEmpty();
        processStreamingResponse(chunk, contentBlockIndex, isComplete);
    }
}

void StreamingAIClient::processStreamingResponse(const QString &chunk, int idx, bool isComplete)
{
    Q_UNUSED(idx)
    if (!chunk.isEmpty())
    {
        m_accumulatedResponse += chunk;
        // Remove the legacy command accumulation since we'll parse from the full response
        emit responseChunkReceived(chunk);
    }
    
    if (isComplete)
    {
        finalizeResponse();
        emit streamingCompleted();
    }
}

void StreamingAIClient::finalizeResponse()
{
    stopLoadingIndicator();
    
    if (!m_accumulatedResponse.isEmpty())
    {
        ConsoleMessageRepository::instance()->addOutput(QString("AI: %1").arg(m_accumulatedResponse));
        emit responseReceived(m_accumulatedResponse);
    }
    
    // Try to parse commands from the accumulated response
    // The AI might return commands in different formats:
    // 1. As a JSON object with a "commands" field containing a stringified JSON array
    // 2. As a direct JSON array
    if (!m_accumulatedResponse.isEmpty())
    {
        // Clean up the response to handle malformed JSON
        QString cleanedResponse = m_accumulatedResponse;
        
        // Find the JSON object boundaries more carefully
        int jsonStartIdx = -1;
        int jsonEndIdx = -1;
        int braceCount = 0;
        bool inString = false;
        bool escapeNext = false;
        
        for (int i = 0; i < cleanedResponse.length(); i++) {
            QChar ch = cleanedResponse[i];
            
            if (escapeNext) {
                escapeNext = false;
                continue;
            }
            
            if (ch == '\\') {
                escapeNext = true;
                continue;
            }
            
            if (ch == '"' && !escapeNext) {
                inString = !inString;
                continue;
            }
            
            if (!inString) {
                if (ch == '{') {
                    if (jsonStartIdx == -1) jsonStartIdx = i;
                    braceCount++;
                } else if (ch == '}') {
                    braceCount--;
                    if (braceCount == 0 && jsonStartIdx != -1) {
                        jsonEndIdx = i;
                        break;  // Found complete JSON object
                    }
                }
            }
        }
        
        if (jsonStartIdx != -1 && jsonEndIdx > jsonStartIdx)
        {
            QString jsonStr = cleanedResponse.mid(jsonStartIdx, jsonEndIdx - jsonStartIdx + 1);
            QJsonParseError parseError;
            QJsonDocument doc = QJsonDocument::fromJson(jsonStr.toUtf8(), &parseError);
            
            if (parseError.error != QJsonParseError::NoError) {
                qWarning() << "JSON parse error:" << parseError.errorString() << "at offset" << parseError.offset;
                qWarning() << "JSON string:" << jsonStr;
                
                // Try to fix common JSON issues
                jsonStr.replace("\\n", " ");  // Remove newlines that might break JSON
                doc = QJsonDocument::fromJson(jsonStr.toUtf8(), &parseError);
                
                if (parseError.error != QJsonParseError::NoError) {
                    return;
                }
            }
            
            if (doc.isObject())
            {
                QJsonObject obj = doc.object();
                bool commandsExecuted = false;
                
                if (obj.contains("commands") && obj["commands"].isString())
                {
                    // Parse the stringified commands array
                    QString commandsStr = obj["commands"].toString();
                    
                    // Try to fix common JSON corruption issues
                    // Sometimes the AI splits IDs or values across JSON boundaries
                    if (commandsStr.contains("\"parentId\":\"") && !commandsStr.contains("\"parentId\":\"\"")) {
                        // Find incomplete parentId patterns
                        QRegularExpression incompleteParentId("\"parentId\":\"[0-9]+$");
                        if (incompleteParentId.match(commandsStr).hasMatch()) {
                            // Try to find the rest of the ID in the continuation
                            QRegularExpression findRestOfId("^[0-9]+\"");
                            int pos = commandsStr.indexOf(incompleteParentId);
                            if (pos != -1) {
                                // Look for numbers after the partial ID
                                QString afterPartial = commandsStr.mid(pos + incompleteParentId.match(commandsStr).capturedLength());
                                QRegularExpression numbers("^[0-9]+");
                                auto match = numbers.match(afterPartial);
                                if (match.hasMatch()) {
                                    // Fix by adding quotes around the complete ID
                                    QString beforeId = commandsStr.left(pos);
                                    QString partialId = incompleteParentId.match(commandsStr).captured();
                                    QString restOfId = match.captured();
                                    QString afterId = afterPartial.mid(match.capturedLength());
                                    commandsStr = beforeId + partialId + restOfId + "\"" + afterId;
                                }
                            }
                        }
                    }
                    
                    // Also try to fix split element types like "setProperty912\\"
                    commandsStr.replace(QRegularExpression("\"setProperty[0-9]+\\\\+\""), "\"setProperty\"");
                    
                    QJsonParseError parseError;
                    QJsonDocument cmdDoc = QJsonDocument::fromJson(commandsStr.toUtf8(), &parseError);
                    
                    if (cmdDoc.isArray())
                    {
                        QJsonArray cmds = cmdDoc.array();
                        if (!cmds.isEmpty())
                        {
                            qDebug() << "Executing" << cmds.size() << "commands from AI";
                            m_commandDispatcher->executeCommands(cmds);
                            emit commandsReceived(cmds);
                            commandsExecuted = true;
                        }
                    }
                    else
                    {
                        qWarning() << "Failed to parse commands string as JSON array:" << commandsStr;
                        qWarning() << "Parse error:" << parseError.errorString() << "at" << parseError.offset;
                        
                        // Try one more time with aggressive cleanup
                        commandsStr.replace("\\\\", "\\");
                        commandsStr.replace("\n", " ");
                        cmdDoc = QJsonDocument::fromJson(commandsStr.toUtf8(), &parseError);
                        
                        if (cmdDoc.isArray()) {
                            QJsonArray cmds = cmdDoc.array();
                            if (!cmds.isEmpty()) {
                                qDebug() << "Executing" << cmds.size() << "commands from AI (after cleanup)";
                                m_commandDispatcher->executeCommands(cmds);
                                emit commandsReceived(cmds);
                                commandsExecuted = true;
                            }
                        }
                    }
                }
                
                // Check if AI wants to continue
                bool shouldContinue = obj["shouldContinue"].toBool(false);
                QString continuationContext = obj["continuationContext"].toString();
                
                if (shouldContinue && !continuationContext.isEmpty())
                {
                    // Execute commands first if we haven't already
                    if (!commandsExecuted && obj.contains("commands") && obj["commands"].isString())
                    {
                        // Parse and execute commands before showing prompt
                        QString commandsStr = obj["commands"].toString();
                        QJsonParseError parseError;
                        QJsonDocument cmdDoc = QJsonDocument::fromJson(commandsStr.toUtf8(), &parseError);
                        
                        if (cmdDoc.isArray())
                        {
                            QJsonArray cmds = cmdDoc.array();
                            if (!cmds.isEmpty())
                            {
                                qDebug() << "Executing" << cmds.size() << "commands from AI before continuation prompt";
                                m_commandDispatcher->executeCommands(cmds);
                                emit commandsReceived(cmds);
                            }
                        }
                    }
                    
                    ConsoleMessageRepository::instance()->addInfo("AI will continue with: " + continuationContext);
                    
                    // Show the AI prompt options to the user
                    ConsoleMessageRepository::instance()->setShowAIPrompt(true);
                    
                    // Store the continuation context for later use
                    m_pendingContinuationContext = continuationContext;
                    
                    // Don't automatically continue - wait for user input
                }
            }
        }
        
        // If no JSON object found, try the legacy approach with accumulated commands
        else if (!m_accumulatedCommands.isEmpty())
        {
            int startIdx = m_accumulatedCommands.indexOf('[');
            int endIdx = m_accumulatedCommands.lastIndexOf(']');
            if (startIdx != -1 && endIdx > startIdx)
            {
                QString jsonPart = m_accumulatedCommands.mid(startIdx, endIdx - startIdx + 1);
                QJsonDocument doc = QJsonDocument::fromJson(jsonPart.toUtf8());
                if (doc.isArray())
                {
                    QJsonArray cmds = doc.array();
                    if (!cmds.isEmpty())
                    {
                        qDebug() << "Executing" << cmds.size() << "commands from AI (legacy format)";
                        m_commandDispatcher->executeCommands(cmds);
                        emit commandsReceived(cmds);
                    }
                }
            }
        }
    }
    
    m_pendingMessage.clear();
    m_accumulatedResponse.clear();
    m_accumulatedCommands.clear();
    // Don't clear m_pendingContinuationContext here - it needs to persist for user response
}

void StreamingAIClient::handleComplete(const QString &subId)
{
    if (subId == m_currentSubscriptionId)
    {
        // Subscription completed
        m_currentSubscriptionId.clear();
    }
}

void StreamingAIClient::handleError(const QJsonObject &payload)
{
    stopLoadingIndicator();
    
    QString msg = "WebSocket error: ";
    if (payload.contains("errors") && payload["errors"].isArray())
    {
        QStringList msgs;
        for (auto errVal : payload["errors"].toArray())
        {
            if (errVal.isObject())
            {
                auto errObj = errVal.toObject();
                QString m = errObj["message"].toString();
                if (errObj.contains("errorCode"))
                    m += QString(" (code: %1)").arg(errObj["errorCode"].toInt());
                msgs.append(m);
            }
        }
        msg += msgs.join(", ");
    }
    else if (payload.contains("message"))
    {
        msg += payload["message"].toString();
    }
    else
        msg += "Unknown error";
    ConsoleMessageRepository::instance()->addError(msg);
    emit errorOccurred(msg);
}

QString StreamingAIClient::getCanvasState() const
{
    QJsonObject state;
    if (!m_application)
        return QJsonDocument(state).toJson(QJsonDocument::Compact);

    if (m_application->activeCanvas())
        state["canvasMode"] = m_application->activeCanvas()->viewMode();

    auto selMgr = m_application->activeCanvas() ? m_application->activeCanvas()->selectionManager() : nullptr;
    if (selMgr)
    {
        QJsonArray selected;
        for (Element *e : selMgr->selectedElements())
        {
            QJsonObject obj;
            obj["id"] = e->getId();
            obj["name"] = e->getName();
            obj["type"] = e->metaObject()->className();
            selected.append(obj);
        }
        state["selectedElements"] = selected;
    }

    auto elemModel = m_application->activeCanvas() ? m_application->activeCanvas()->elementModel() : nullptr;
    if (elemModel)
    {
        QJsonArray all;
        for (int i = 0; i < elemModel->rowCount(); ++i)
        {
            Element *e = elemModel->elementAt(i);
            if (!e)
                continue;
            QJsonObject obj;
            obj["id"] = e->getId();
            obj["name"] = e->getName();
            obj["type"] = e->metaObject()->className();
            if (auto ce = qobject_cast<CanvasElement *>(e))
            {
                obj["x"] = ce->x();
                obj["y"] = ce->y();
                obj["width"] = ce->width();
                obj["height"] = ce->height();
            }
            all.append(obj);
        }
        state["elements"] = all;
    }

    return QJsonDocument(state).toJson(QJsonDocument::Compact);
}

void StreamingAIClient::startLoadingIndicator()
{
    m_loadingDots = 0;
    m_loadingMessageId = ConsoleMessageRepository::instance()->addMessageWithId("AI is thinking", ConsoleMessageRepository::Info);
    m_loadingTimer->start();
}

void StreamingAIClient::stopLoadingIndicator()
{
    m_loadingTimer->stop();
    if (!m_loadingMessageId.isEmpty()) {
        ConsoleMessageRepository::instance()->removeMessage(m_loadingMessageId);
        m_loadingMessageId.clear();
    }
}

void StreamingAIClient::updateLoadingIndicator()
{
    if (m_loadingMessageId.isEmpty()) return;
    
    m_loadingDots = (m_loadingDots + 1) % 4;
    QString dots;
    for (int i = 0; i < m_loadingDots; ++i) dots += ".";
    QString spaces;
    for (int i = 0; i < (3 - m_loadingDots); ++i) spaces += " ";
    
    ConsoleMessageRepository::instance()->updateMessage(
        m_loadingMessageId, 
        QString("AI is thinking%1%2").arg(dots).arg(spaces)
    );
}
