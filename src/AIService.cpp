#include "AIService.h"
#include <QNetworkRequest>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QDebug>

const QString AIService::GRAPHQL_ENDPOINT = "https://enrxjqdgdvc7pkragdib6abhyy.appsync-api.us-west-2.amazonaws.com/graphql";

AIService::AIService(QObject *parent)
    : QObject(parent)
    , m_networkManager(new QNetworkAccessManager(this))
{
}

AIService::~AIService() = default;

void AIService::callCubitAI(const QString &prompt, const QString &accessToken)
{
    if (prompt.isEmpty()) {
        emit errorOccurred("No prompt provided");
        return;
    }
    
    if (accessToken.isEmpty()) {
        emit errorOccurred("Authentication required");
        return;
    }
    
    // Prepare GraphQL query
    QString query = "query CallCubitAI($prompt: String!) { CubitAi(prompt: $prompt) }";
    
    QJsonObject variables;
    variables["prompt"] = prompt;
    
    QJsonObject requestBody;
    requestBody["query"] = query;
    requestBody["variables"] = variables;
    
    QJsonDocument doc(requestBody);
    QByteArray jsonData = doc.toJson();
    
    // Create network request
    QNetworkRequest request((QUrl(GRAPHQL_ENDPOINT)));
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
    request.setRawHeader("Authorization", accessToken.toUtf8());
    
    // Send the request
    QNetworkReply *reply = m_networkManager->post(request, jsonData);
    connect(reply, &QNetworkReply::finished, this, &AIService::handleNetworkReply);
    
}

void AIService::handleNetworkReply()
{
    QNetworkReply *reply = qobject_cast<QNetworkReply*>(sender());
    if (!reply) {
        return;
    }
    
    reply->deleteLater();
    
    if (reply->error() != QNetworkReply::NoError) {
        QString errorStr = QString("HTTP Error: %1").arg(reply->errorString());
        qWarning() << "AIService network error:" << errorStr;
        emit errorOccurred(errorStr);
        return;
    }
    
    QByteArray responseData = reply->readAll();
    QJsonDocument doc = QJsonDocument::fromJson(responseData);
    
    if (doc.isNull()) {
        emit errorOccurred("Failed to parse response");
        return;
    }
    
    QJsonObject response = doc.object();
    
    // Check for GraphQL errors
    if (response.contains("errors")) {
        QJsonArray errors = response["errors"].toArray();
        QString errorMessage = "GraphQL Error: ";
        for (const auto &error : errors) {
            errorMessage += error.toObject()["message"].toString() + " ";
        }
        qWarning() << "AIService GraphQL error:" << errorMessage;
        emit errorOccurred(errorMessage);
        return;
    }
    
    // Extract the AI response
    if (response.contains("data") && response["data"].toObject().contains("CubitAi")) {
        QString aiResponse = response["data"].toObject()["CubitAi"].toString();
        emit responseReceived(aiResponse);
    } else {
        emit errorOccurred("Unknown error - no data in response");
    }
}