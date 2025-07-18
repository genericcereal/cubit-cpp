#include "CubitAIClient.h"
#include "ConsoleMessageRepository.h"
#include "AuthenticationManager.h"
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QFile>
#include <QDebug>

CubitAIClient::CubitAIClient(AuthenticationManager* authManager, QObject *parent)
    : QObject(parent)
    , m_networkManager(std::make_unique<QNetworkAccessManager>(this))
    , m_authManager(authManager)
    , m_isRetryingAfterRefresh(false)
{
    // Connect to token refresh signal
    if (m_authManager) {
        connect(m_authManager, &AuthenticationManager::tokensRefreshed,
                this, &CubitAIClient::onTokensRefreshed,
                Qt::UniqueConnection);
    }
}

CubitAIClient::~CubitAIClient() = default;

void CubitAIClient::sendMessage(const QString& description) {
    QJsonObject queryObj;
    queryObj["query"] = "query CubitAi($description:String){ cubitAi(description:$description) }";
    
    QJsonObject variables;
    variables["description"] = description;
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
            QString rawResponse = data["cubitAi"].toString();
            
            // Try to parse the response as JSON
            QJsonDocument responseDoc = QJsonDocument::fromJson(rawResponse.toUtf8());
            QString finalResponse;
            
            if (!responseDoc.isNull() && responseDoc.isObject()) {
                QJsonObject responseObj = responseDoc.object();
                // Check if the response contains a description field
                if (responseObj.contains("description")) {
                    finalResponse = responseObj["description"].toString();
                } else if (responseObj.contains("response")) {
                    finalResponse = responseObj["response"].toString();
                } else if (responseObj.contains("message")) {
                    finalResponse = responseObj["message"].toString();
                } else {
                    // If no known field, use the raw response
                    finalResponse = rawResponse;
                }
            } else {
                // If not JSON, use the raw response
                finalResponse = rawResponse;
            }
            
            ConsoleMessageRepository::instance()->addOutput(QString("CubitAI: %1").arg(finalResponse));
            emit responseReceived(finalResponse);
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
    ConsoleMessageRepository::instance()->addInfo("Tokens refreshed. Retrying previous request...");
    
    // If we were retrying after a refresh, retry the pending query
    if (m_isRetryingAfterRefresh && !m_pendingQuery.isEmpty()) {
        m_isRetryingAfterRefresh = false;
        makeGraphQLRequest(m_pendingQuery, "");
    }
}