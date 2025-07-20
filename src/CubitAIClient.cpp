#include "CubitAIClient.h"
#include "ConsoleMessageRepository.h"
#include "AuthenticationManager.h"
#include "AICommandDispatcher.h"
#include "Application.h"
#include "ElementModel.h"
#include "SelectionManager.h"
#include "Element.h"
#include "CanvasElement.h"
#include "Project.h"
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QFile>
#include <QDebug>

CubitAIClient::CubitAIClient(AuthenticationManager* authManager, Application* app, QObject *parent)
    : QObject(parent)
    , m_networkManager(std::make_unique<QNetworkAccessManager>(this))
    , m_authManager(authManager)
    , m_application(app)
    , m_commandDispatcher(std::make_unique<AICommandDispatcher>(app, this))
    , m_isRetryingAfterRefresh(false)
{
    // Connect to token refresh signal
    if (m_authManager) {
        connect(m_authManager, &AuthenticationManager::tokensRefreshed,
                this, &CubitAIClient::onTokensRefreshed,
                Qt::UniqueConnection);
    }
    
    // Connect to command dispatcher signals
    connect(m_commandDispatcher.get(), &AICommandDispatcher::commandExecuted,
            this, [](const QString& description, bool success) {
        if (success) {
            ConsoleMessageRepository::instance()->addInfo(description);
        }
    });
    
    connect(m_commandDispatcher.get(), &AICommandDispatcher::commandError,
            this, [](const QString& error) {
        ConsoleMessageRepository::instance()->addError(QString("Command error: %1").arg(error));
    });
}

CubitAIClient::~CubitAIClient() = default;

void CubitAIClient::sendMessage(const QString& description) {
    QJsonObject queryObj;
    queryObj["query"] = "query CubitAi($description:String, $canvasState:String){ cubitAi(description:$description, canvasState:$canvasState) { message commands } }";
    
    QJsonObject variables;
    variables["description"] = description;
    
    // Get current canvas state as JSON string
    QString canvasStateJson = getCanvasState();
    variables["canvasState"] = canvasStateJson;
    
    queryObj["variables"] = variables;
    
    QJsonDocument doc(queryObj);
    m_pendingQuery = doc.toJson(QJsonDocument::Compact);
    makeGraphQLRequest(m_pendingQuery, "");
}

void CubitAIClient::makeGraphQLRequest(const QString& queryJson, const QString& /*variables*/) {
    QString endpoint = getApiEndpoint();
    if (endpoint.isEmpty()) {
        emit errorOccurred("Failed to read API endpoint from amplify_outputs.json");
        return;
    }
    
    // Get authentication token
    QString authToken = getAuthToken();
    
    QNetworkRequest request;
    request.setUrl(QUrl(endpoint));
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
    
    // Add authorization header if we have a token
    if (!authToken.isEmpty()) {
        request.setRawHeader("Authorization", authToken.toUtf8());
    } else {
        // If no auth token, log a warning
        ConsoleMessageRepository::instance()->addWarning("No authentication token available. API calls may fail.");
    }
    
    // Send the request
    QNetworkReply* reply = m_networkManager->post(request, queryJson.toUtf8());
    
    // Connect to handle the response
    connect(reply, &QNetworkReply::finished, this, [this, reply]() {
        handleNetworkReply(reply);
    });
}

void CubitAIClient::handleNetworkReply(QNetworkReply* reply) {
    reply->deleteLater();
    
    // Always read the response body
    QByteArray responseData = reply->readAll();
    
    if (reply->error() != QNetworkReply::NoError) {
        QString errorMsg = QString("Network error: %1").arg(reply->errorString());
        
        // Log additional error details
        QVariant statusCode = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute);
        if (statusCode.isValid()) {
            errorMsg += QString(" (HTTP %1)").arg(statusCode.toInt());
        }
        
        // Parse the response to check for token expiration
        QJsonDocument errorDoc = QJsonDocument::fromJson(responseData);
        if (!errorDoc.isNull() && isTokenExpiredError(errorDoc) && !m_isRetryingAfterRefresh) {
            // Token expired, try to refresh
            ConsoleMessageRepository::instance()->addInfo("Token expired. Attempting to refresh...");
            m_isRetryingAfterRefresh = true;
            
            // Store the pending query to retry after refresh
            // Note: This is a simplified approach - in production you'd want to store the full request details
            
            // Request token refresh
            if (m_authManager) {
                m_authManager->refreshAccessToken();
                return; // Wait for refresh to complete
            }
        }
        
        // Log response headers for debugging
        ConsoleMessageRepository::instance()->addError("Response headers:");
        foreach (const QByteArray& headerName, reply->rawHeaderList()) {
            ConsoleMessageRepository::instance()->addError(QString("  %1: %2")
                .arg(QString::fromUtf8(headerName))
                .arg(QString::fromUtf8(reply->rawHeader(headerName))));
        }
        
        // Log response body for debugging
        if (!responseData.isEmpty()) {
            ConsoleMessageRepository::instance()->addError(QString("Response body: %1").arg(QString::fromUtf8(responseData)));
        }
        
        ConsoleMessageRepository::instance()->addError(errorMsg);
        emit errorOccurred(errorMsg);
        m_isRetryingAfterRefresh = false;
        return;
    }
    
    QJsonDocument doc = QJsonDocument::fromJson(responseData);
    
    if (doc.isNull()) {
        QString errorMsg = "Failed to parse response JSON";
        ConsoleMessageRepository::instance()->addError(errorMsg);
        emit errorOccurred(errorMsg);
        return;
    }
    
    QJsonObject root = doc.object();
    
    // Check for GraphQL errors
    if (root.contains("errors")) {
        QJsonArray errors = root["errors"].toArray();
        if (!errors.isEmpty()) {
            QString errorMsg = errors[0].toObject()["message"].toString();
            ConsoleMessageRepository::instance()->addError(QString("CubitAI error: %1").arg(errorMsg));
            emit errorOccurred(errorMsg);
            return;
        }
    }
    
    // Extract the response from the data field
    if (root.contains("data")) {
        QJsonObject data = root["data"].toObject();
        if (data.contains("cubitAi")) {
            QJsonObject aiResponse = data["cubitAi"].toObject();
            
            // Extract the message and commands
            QString message = aiResponse["message"].toString();
            
            // Parse commands from JSON string
            QString commandsJson = aiResponse["commands"].toString();
            
            QJsonDocument commandsDoc = QJsonDocument::fromJson(commandsJson.toUtf8());
            QJsonArray commands;
            
            if (commandsDoc.isArray()) {
                commands = commandsDoc.array();
            } else if (!commandsJson.isEmpty() && commandsJson != "[]") {
                ConsoleMessageRepository::instance()->addWarning("Commands response is not a valid JSON array");
            }
            
            // Display the message to the user
            if (!message.isEmpty()) {
                ConsoleMessageRepository::instance()->addOutput(QString("CubitAI: %1").arg(message));
                emit responseReceived(message);
            }
            
            // Execute commands if any
            if (!commands.isEmpty()) {
                ConsoleMessageRepository::instance()->addInfo(QString("Executing %1 commands...").arg(commands.size()));
                
                // Execute the commands
                m_commandDispatcher->executeCommands(commands);
                emit commandsReceived(commands);
            }
        }
    }
}

QString CubitAIClient::getApiEndpoint() const {
    // Read the amplify_outputs.json file
    QFile file("amplify_outputs.json");
    if (!file.open(QIODevice::ReadOnly)) {
        qWarning() << "Failed to open amplify_outputs.json";
        return QString();
    }
    
    QByteArray data = file.readAll();
    QJsonDocument doc = QJsonDocument::fromJson(data);
    
    if (doc.isNull()) {
        qWarning() << "Failed to parse amplify_outputs.json";
        return QString();
    }
    
    QJsonObject root = doc.object();
    if (root.contains("data") && root["data"].isObject()) {
        QJsonObject dataObj = root["data"].toObject();
        if (dataObj.contains("url")) {
            return dataObj["url"].toString();
        }
    }
    
    return QString();
}

QString CubitAIClient::getAuthToken() const {
    if (m_authManager && m_authManager->isAuthenticated()) {
        // Try using access token instead of ID token for AppSync
        QString accessToken = m_authManager->getAccessToken();
        if (!accessToken.isEmpty()) {
            return accessToken;
        }
        
        // Fall back to ID token if access token is not available
        QString idToken = m_authManager->getIdToken();
        if (!idToken.isEmpty()) {
            return idToken;
        }
    }
    return QString();
}

bool CubitAIClient::isTokenExpiredError(const QJsonDocument& doc) const {
    if (!doc.isObject()) {
        return false;
    }
    
    QJsonObject root = doc.object();
    if (root.contains("errors")) {
        QJsonArray errors = root["errors"].toArray();
        for (const QJsonValue& errorValue : errors) {
            QJsonObject errorObj = errorValue.toObject();
            QString message = errorObj["message"].toString().toLower();
            if (message.contains("token has expired") || 
                message.contains("token expired") ||
                message.contains("unauthorized")) {
                return true;
            }
        }
    }
    
    return false;
}

void CubitAIClient::onTokensRefreshed() {
    ConsoleMessageRepository::instance()->addInfo("Tokens refreshed");
    
    // If we were retrying after a refresh, retry the pending query
    if (m_isRetryingAfterRefresh && !m_pendingQuery.isEmpty()) {
        m_isRetryingAfterRefresh = false;
        makeGraphQLRequest(m_pendingQuery, "");
    }
}

QString CubitAIClient::getCanvasState() const {
    QJsonObject state;
    
    if (!m_application) {
        return QJsonDocument(state).toJson(QJsonDocument::Compact);
    }
    
    // Get current canvas mode if available
    if (m_application->activeCanvas()) {
        state["canvasMode"] = m_application->activeCanvas()->viewMode();
    }
    
    // Get selected elements
    auto selectionManager = m_application->activeCanvas() ? m_application->activeCanvas()->selectionManager() : nullptr;
    if (selectionManager) {
        QJsonArray selectedElements;
        for (Element* elem : selectionManager->selectedElements()) {
            QJsonObject elemObj;
            elemObj["id"] = elem->getId();
            elemObj["name"] = elem->getName();
            elemObj["type"] = elem->metaObject()->className();
            selectedElements.append(elemObj);
        }
        state["selectedElements"] = selectedElements;
    }
    
    // Get all elements
    auto elementModel = m_application->activeCanvas() ? m_application->activeCanvas()->elementModel() : nullptr;
    if (elementModel) {
        QJsonArray allElements;
        for (int i = 0; i < elementModel->rowCount(); ++i) {
            Element* elem = elementModel->elementAt(i);
            if (elem) {
                QJsonObject elemObj;
                elemObj["id"] = elem->getId();
                elemObj["name"] = elem->getName();
                elemObj["type"] = elem->metaObject()->className();
                
                // Add position and size for visual elements
                if (auto canvasElem = qobject_cast<CanvasElement*>(elem)) {
                    elemObj["x"] = canvasElem->x();
                    elemObj["y"] = canvasElem->y();
                    elemObj["width"] = canvasElem->width();
                    elemObj["height"] = canvasElem->height();
                }
                
                allElements.append(elemObj);
            }
        }
        state["elements"] = allElements;
    }
    
    return QJsonDocument(state).toJson(QJsonDocument::Compact);
}