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
#include <QTextStream>
#include <QCoreApplication>
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
    else if (m_currentConversationId.isEmpty())
    {
        // Create a new conversation only if we don't have one
        createConversation();
    }
    else
    {
        // Use existing conversation
        sendConversationMessage(m_currentConversationId, m_pendingMessage);
    }
}

bool StreamingAIClient::isConnected() const
{
    return m_isConnected && m_connectionAckReceived;
}

void StreamingAIClient::handleUserContinuationResponse(bool accepted, const QString &feedback) {
    ConsoleMessageRepository::instance()->setShowAIPrompt(false);

    if (!m_currentConversationId.isEmpty() && !m_pendingContinuationContext.isEmpty()) {
        if (accepted && m_pendingContinuationContext == "PLAN_CONFIRMED") {
            // ✅ User approved the plan → start executing first step
            m_currentStepIndex = 0;
            executeNextPlanStep();
        } 
        else if (accepted && m_pendingContinuationContext.startsWith("EXEC_STEP")) {
            // ✅ Continue next step after manual approval
            executeNextPlanStep();
        } 
        else {
            // ❌ User rejected → ask AI to adjust
            QString message = QString("User feedback: %1. Please adjust the plan.").arg(feedback);
            sendConversationMessage(m_currentConversationId, message);
        }

        m_pendingContinuationContext.clear();
    }
}

void StreamingAIClient::clearConversation()
{
    m_currentConversationId.clear();
    m_currentSubscriptionId.clear();
    m_pendingContinuationContext.clear();
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

    // qDebug() << "Opening WebSocket with URL:" << url;
    // qDebug() << "WebSocket protocol: graphql-ws";
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

    // qDebug() << "[DEBUG] Original AppSync endpoint:" << endpoint;

    QString wsEndpoint = endpoint;
    wsEndpoint.replace("https://", "wss://");
    wsEndpoint.replace("appsync-api", "appsync-realtime-api");
    if (wsEndpoint.endsWith("/graphql"))
        wsEndpoint.chop(8);
    wsEndpoint += "/graphql";

    // qDebug() << "[DEBUG] Realtime WebSocket endpoint:" << wsEndpoint;

    QJsonObject authHeader;
    QUrl endpointUrl(endpoint);
    authHeader["host"] = endpointUrl.host();

    QString token = getAuthToken();
    if (!token.isEmpty())
        authHeader["Authorization"] = token;
    else
        qWarning() << "[DEBUG] No Cognito token available for websocket!";

    // qDebug() << "[DEBUG] WebSocket authHeader JSON:"
    //          << QJsonDocument(authHeader).toJson(QJsonDocument::Compact);

    QByteArray headerJson = QJsonDocument(authHeader).toJson(QJsonDocument::Compact);
    QString encodedHeader = QString::fromUtf8(headerJson.toBase64());
    QString encodedPayload = "e30="; // "{}" -> "e30="

    QString finalUrl = QString("%1?header=%2&payload=%3")
                           .arg(wsEndpoint)
                           .arg(encodedHeader)
                           .arg(encodedPayload);

    QByteArray decodedHeader = QByteArray::fromBase64(encodedHeader.toUtf8());
    // qDebug() << "[DEBUG] Decoded Base64 header JSON (sanity check):" << decodedHeader;
    // qDebug() << "[DEBUG] Final WebSocket URL:" << finalUrl;

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
    // qDebug() << "No Cognito ID token available!";
    return QString();
}

QString StreamingAIClient::getToolRegistryUrl() const
{
    return "https://k72mo3oun7sefawjhvilq2ne5a0ybfgr.lambda-url.us-west-2.on.aws/";
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
        // qDebug() << "Full error message:" << QJsonDocument(obj).toJson(QJsonDocument::Compact);
        
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
            // qDebug() << "This typically means the subscription message format is incorrect for AppSync";
        }
    }
    else if (type == "ka")
    {
    } // Keepalive
    else
    {
        // qDebug() << "Unknown WebSocket message type:" << type;
    }
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
    // qDebug() << "Tokens refreshed in StreamingAIClient, reconnecting WebSocket";
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
            // qDebug() << "CreateConversation response:" << responseData;
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
                        // Send CubitAI rules as the first message
                        // sendCubitAIRules(m_currentConversationId);  // Disabled - no longer sending rules
                        
                        // Check if this is just initialization (INIT_AI_WITH_RULES)
                        if (m_pendingMessage == "INIT_AI_WITH_RULES") {
                            // Don't send any user message, just the rules
                            m_pendingMessage.clear();
                        } else {
                            // Then send the user's message after a brief delay
                            QTimer::singleShot(200, this, [this]() {
                                sendConversationMessage(m_currentConversationId, m_pendingMessage);
                            });
                        }
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

void StreamingAIClient::sendCubitAIRules(const QString &conversationId)
{
    // Read the CubitAI rules from file
    QString rulesPath = QCoreApplication::applicationDirPath() + "/../amplify/data/prompts/CubitAIRules.txt";
    QFile rulesFile(rulesPath);
    QString rulesContent;
    
    if (rulesFile.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QTextStream in(&rulesFile);
        rulesContent = in.readAll();
        rulesFile.close();
    } else {
        // If file not found, try alternative path (development environment)
        rulesPath = "amplify/data/prompts/CubitAIRules.txt";
        QFile altRulesFile(rulesPath);
        if (altRulesFile.open(QIODevice::ReadOnly | QIODevice::Text)) {
            QTextStream in(&altRulesFile);
            rulesContent = in.readAll();
            altRulesFile.close();
        } else {
            qWarning() << "Could not load CubitAI rules file from:" << rulesPath;
            // Fall back to a minimal rules string
            rulesContent = "You are CubitAI. Always respond in strict JSON with {message, commands, shouldContinue, continuationContext}.";
        }
    }
    
    // Send the rules as the first message
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

    QJsonObject textBlock;
    textBlock["text"] = rulesContent;
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
    connect(reply, &QNetworkReply::finished, this, [reply, manager]() {
        if (reply->error() != QNetworkReply::NoError) {
            qWarning() << "Failed to send CubitAI rules:" << reply->errorString();
        }
        reply->deleteLater();
        manager->deleteLater();
    });
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
    connect(reply, &QNetworkReply::finished, this, [reply, manager]()
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
        // int contentBlockDoneAtIndex = resp["contentBlockDoneAtIndex"].toInt(-1);
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

    if (m_accumulatedResponse.isEmpty()) return;

    // --- Safeguard: Prevent plan injection during EXECUTION ---
    if (m_pendingContinuationContext == "EXEC_STEP") {
        // If AI accidentally replanned, remove plan text
        if (m_accumulatedResponse.contains("PLAN:")) {
            qDebug() << "⚠️ AI tried to replan in EXECUTION mode. Stripping plan text.";
            // Remove any "PLAN:" lines and "WAITING FOR PLAN CONFIRMATION"
            QString filtered = m_accumulatedResponse;
            filtered.remove(QRegularExpression("PLAN:.*WAITING FOR PLAN CONFIRMATION", QRegularExpression::DotMatchesEverythingOption));
            filtered.remove("WAITING FOR PLAN CONFIRMATION");
            m_accumulatedResponse = filtered.trimmed();
            
            // Skip console output if nothing left after filtering
            if (m_accumulatedResponse.isEmpty()) {
                ConsoleMessageRepository::instance()->addInfo("AI response contained only plan text during execution, skipping.");
                return;
            }
        }
    }

    ConsoleMessageRepository::instance()->addOutput(QString("AI: %1").arg(m_accumulatedResponse));
    emit responseReceived(m_accumulatedResponse);

    // --- 1) Detect PLAN phase ---
    if (m_accumulatedResponse.contains("PLAN:") ||
        m_accumulatedResponse.contains("WAITING FOR PLAN CONFIRMATION")) {
        m_pendingPlanSteps = extractPlanSteps(m_accumulatedResponse);

        if (!m_pendingPlanSteps.isEmpty()) {
            ConsoleMessageRepository::instance()->addInfo(
                QString("AI proposed a plan with %1 steps.").arg(m_pendingPlanSteps.size())
            );
            // Show confirm/reject prompt
            ConsoleMessageRepository::instance()->setShowAIPrompt(true);
            m_pendingContinuationContext = "PLAN_CONFIRMED";
        } else {
            ConsoleMessageRepository::instance()->addInfo("AI produced a plan but no steps were detected.");
        }
        
        // ✅ Remove everything up to WAITING FOR PLAN CONFIRMATION
        m_accumulatedResponse.remove(
            QRegularExpression("PLAN:.*WAITING FOR PLAN CONFIRMATION",
            QRegularExpression::DotMatchesEverythingOption)
        );
        
        // Clear the buffer completely after handling the plan
        m_accumulatedResponse.clear();
        
        // ✅ Stop here, don't attempt JSON parsing
        return;
    }

    // --- 2) Detect tool registry request ---
    if (m_accumulatedResponse.startsWith("Request tool list for category")) {
        QString requestedCategory = m_accumulatedResponse.section(':', 1).trimmed();
        fetchToolRegistry(requestedCategory);
        m_accumulatedResponse.clear(); // ✅ Clear buffer after handling tool request
        return; // ✅ Stop here, wait for tool list
    }

    // --- Handle EXECUTION mode responses ---
    if (m_pendingContinuationContext == "EXEC_STEP") {
        // Check if response contains JSON
        int startIdx = m_accumulatedResponse.indexOf('{');
        int endIdx   = m_accumulatedResponse.lastIndexOf('}');
        
        // If JSON is present, prioritize it over tool requests
        if (startIdx >= 0 && endIdx > startIdx) {
            QString onlyJson = m_accumulatedResponse.mid(startIdx, endIdx - startIdx + 1);
            m_accumulatedResponse = onlyJson; // Extract only the JSON part
            // Continue to JSON parsing below
        }
        // Only check for tool request if no JSON was found
        else if (m_accumulatedResponse.contains("Request tool list for category")) {
            // Find the tool request line
            int toolReqStart = m_accumulatedResponse.indexOf("Request tool list for category");
            if (toolReqStart >= 0) {
                // Extract from "Request" to the end of that line
                int lineEnd = m_accumulatedResponse.indexOf('\n', toolReqStart);
                QString toolRequest;
                if (lineEnd > 0) {
                    toolRequest = m_accumulatedResponse.mid(toolReqStart, lineEnd - toolReqStart);
                } else {
                    toolRequest = m_accumulatedResponse.mid(toolReqStart);
                }
                
                // Extract category from the tool request
                QString category = toolRequest.section(':', 1).trimmed();
                if (!category.isEmpty()) {
                    fetchToolRegistry(category);
                    m_accumulatedResponse.clear(); // ✅ Clear buffer after handling tool request
                    return;
                }
            }
        } else {
            ConsoleMessageRepository::instance()->addInfo("AI execution step had no JSON or tool request, ignoring.");
            m_accumulatedResponse.clear(); // ✅ Clear buffer to prevent accumulation
            return;
        }
    }

    // --- 3) Only parse JSON if it looks like JSON ---
    QString trimmed = m_accumulatedResponse.trimmed();
    if (!(trimmed.startsWith("{") || trimmed.startsWith("["))) {
        ConsoleMessageRepository::instance()->addInfo("AI response is plain text, skipping JSON parse.");
        return;
    }

    // --- Now safe to parse JSON ---
    QJsonParseError parseError;
    QJsonDocument doc = QJsonDocument::fromJson(trimmed.toUtf8(), &parseError);

    if (parseError.error != QJsonParseError::NoError) {
        ConsoleMessageRepository::instance()->addError(
            QString("Failed to parse AI response as JSON: %1 at position %2")
            .arg(parseError.errorString())
            .arg(parseError.offset)
        );
        qWarning() << "Raw response:" << trimmed;
        requestJsonRepair(trimmed, parseError.errorString());
        return;
    }

    if (!doc.isObject()) {
        ConsoleMessageRepository::instance()->addError("AI response is not a valid JSON object");
        requestJsonRepair(trimmed, "Response is not a JSON object");
        return;
    }

    QJsonObject responseObj = doc.object();

    // Extract message if present
    QString message = responseObj["message"].toString();

    // Check if AI is requesting tool list (in JSON message field)
    if (message.startsWith("Request tool list for category")) {
        QString category = message.section(':', 1).trimmed();
        fetchToolRegistry(category);
        return;
    }

    // Extract and parse commands
    if (responseObj.contains("commands") && responseObj["commands"].isString()) {
        QString commandsStr = responseObj["commands"].toString();
        
        if (commandsStr != "[]") {
            QJsonParseError cmdParseError;
            QJsonDocument cmdDoc = QJsonDocument::fromJson(commandsStr.toUtf8(), &cmdParseError);
            
            if (cmdParseError.error == QJsonParseError::NoError && cmdDoc.isArray()) {
                QJsonArray cmds = cmdDoc.array();
                if (!cmds.isEmpty()) {
                    m_commandDispatcher->executeCommands(cmds);
                    emit commandsReceived(cmds);
                    
                    // ✅ After handling JSON/tool request, clear buffer so it won't re-run
                    m_accumulatedResponse.clear();
                }
            } else {
                ConsoleMessageRepository::instance()->addError(
                    QString("Failed to parse commands array: %1").arg(cmdParseError.errorString())
                );
                QString errorDetail = QString("Commands array parsing failed: %1. Invalid commands string: %2")
                    .arg(cmdParseError.errorString())
                    .arg(commandsStr);
                requestJsonRepair(m_accumulatedResponse, errorDetail);
                return;
            }
        }
    }

    // Check if AI wants to continue
    bool shouldContinue = responseObj["shouldContinue"].toBool(false);
    QString continuationContext = responseObj["continuationContext"].toString();

    if (shouldContinue && !m_pendingPlanSteps.isEmpty()) {
        // ✅ Auto-loop next step
        executeNextPlanStep();
    } 
    else if (shouldContinue && !continuationContext.isEmpty()) {
        // Legacy behavior for non-step continuation
        ConsoleMessageRepository::instance()->addInfo("AI wants to continue: " + continuationContext);
        ConsoleMessageRepository::instance()->setShowAIPrompt(true);
        m_pendingContinuationContext = continuationContext;
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

QStringList StreamingAIClient::extractPlanSteps(const QString &aiResponse) {
    QStringList steps;
    // Regex matches lines like "1. Do something"
    QRegularExpression stepRegex(R"(^\s*\d+\.\s+(.*)$)", QRegularExpression::MultilineOption);
    QRegularExpressionMatchIterator it = stepRegex.globalMatch(aiResponse);
    while (it.hasNext()) {
        auto match = it.next();
        QString step = match.captured(1).trimmed();
        if (!step.isEmpty()) steps.append(step);
    }
    return steps;
}

void StreamingAIClient::executeNextPlanStep() {
    if (m_currentStepIndex >= m_pendingPlanSteps.size()) {
        ConsoleMessageRepository::instance()->addInfo("✅ All steps executed. Asking AI to review & refine...");
        sendConversationMessage(
            m_currentConversationId,
            "All planned steps are complete. Please review the results, refine if needed, and provide the final polished answer."
        );
        return;
    }

    QString stepText = m_pendingPlanSteps[m_currentStepIndex];
    ConsoleMessageRepository::instance()->addInfo(
        QString("Executing step %1: %2").arg(m_currentStepIndex + 1).arg(stepText)
    );

    // ✅ Now we tell AI explicitly: DO NOT RE-PLAN
    QString message = QString(
        "⚠️ STOP. You are now in EXECUTION MODE.\n\n"
        "❌ DO NOT say PLAN:\n"
        "❌ DO NOT list steps\n"
        "❌ DO NOT output WAITING FOR PLAN CONFIRMATION\n"
        "❌ DO NOT repeat any previous instructions.\n\n"
        "In EXECUTION MODE, for this step you must do ONLY ONE of these:\n"
        "1️⃣ If you need tool info → respond exactly:\n"
        "   Request tool list for category <category>\n\n"
        "2️⃣ If you already know the tools → respond ONLY with this JSON format:\n"
        "{commands:[{tool:...,params:{...}}],shouldContinue:true}\n\n"
        "Nothing else. No explanations. No plans. No mixed text.\n\n"
        "Execute ONLY this step:\n➡️  %1"
    ).arg(stepText);

    sendConversationMessage(m_currentConversationId, message);

    m_pendingContinuationContext = "EXEC_STEP";
    m_currentStepIndex++;
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

void StreamingAIClient::requestJsonRepair(const QString &invalidJson, const QString &errorMessage)
{
    if (m_currentConversationId.isEmpty()) {
        ConsoleMessageRepository::instance()->addError("Cannot request JSON repair: no active conversation");
        return;
    }
    
    // Clear the accumulated response to prepare for the repaired version
    m_accumulatedResponse.clear();
    m_accumulatedCommands.clear();
    
    // Construct a repair request message
    QString repairMessage = QString(
        "Your previous response contained invalid JSON that could not be parsed. "
        "Error: %1\n\n"
        "The invalid response was:\n%2\n\n"
        "Please provide a corrected response with valid JSON format. "
        "Remember to use the exact format: {\"message\": \"...\", \"commands\": \"[...]\", \"shouldContinue\": bool, \"continuationContext\": \"...\"}"
    ).arg(errorMessage).arg(invalidJson);
    
    ConsoleMessageRepository::instance()->addInfo("Requesting AI to fix invalid JSON response...");
    
    // Send the repair request
    sendConversationMessage(m_currentConversationId, repairMessage);
}

void StreamingAIClient::fetchToolRegistry(const QString &category) {
    QString baseUrl = getToolRegistryUrl();
    if (baseUrl.isEmpty()) {
        ConsoleMessageRepository::instance()->addError("Tool registry URL not configured in amplify_outputs.json");
        return;
    }
    
    QString url = QString("%1?category=%2")
                    .arg(baseUrl)
                    .arg(category);

    QNetworkAccessManager *manager = new QNetworkAccessManager(this);
    QNetworkRequest request((QUrl(url)));

    QNetworkReply *reply = manager->get(request);
    connect(reply, &QNetworkReply::finished, this, [this, reply, manager, category, url]() {
        QByteArray data = reply->readAll();
        reply->deleteLater();
        manager->deleteLater();

        if (reply->error() != QNetworkReply::NoError) {
            ConsoleMessageRepository::instance()->addError(
                QString("Tool registry fetch failed: %1").arg(reply->errorString())
            );
            qDebug() << "Tool registry URL was:" << url;
            qDebug() << "Response:" << data;
            return;
        }

        QJsonDocument doc = QJsonDocument::fromJson(data);
        QJsonObject obj = doc.object();
        QJsonArray toolsArray = obj["tools"].toArray();

        // Format a compact tool list for AI
        QStringList toolLines;
        for (auto t : toolsArray) {
            auto tool = t.toObject();
            toolLines << QString("%1: %2 (params: %3)")
                .arg(tool["name"].toString())
                .arg(tool["description"].toString())
                .arg(QString(QJsonDocument(tool["params"].toObject()).toJson(QJsonDocument::Compact)));
        }

        QString toolListForAI = "Available tools in category '" + category + "':\n" + toolLines.join("\n");

        // Send this back into the AI conversation
        sendConversationMessage(m_currentConversationId, toolListForAI);
    });
}
