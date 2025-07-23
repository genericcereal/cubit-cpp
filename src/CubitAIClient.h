#ifndef CUBITAICLIENT_H
#define CUBITAICLIENT_H

#include <QObject>
#include <QNetworkAccessManager>
#include <QString>
#include <QJsonArray>
#include <memory>

class QNetworkReply;
class AuthenticationManager;
class AICommandDispatcher;
class Application;

class CubitAIClient : public QObject
{
    Q_OBJECT
    
public:
    explicit CubitAIClient(AuthenticationManager* authManager, Application* app, QObject *parent = nullptr);
    ~CubitAIClient();
    
    void sendMessage(const QString& description);
    
signals:
    void responseReceived(const QString& response);
    void commandsReceived(const QJsonArray& commands);
    void errorOccurred(const QString& error);
    
private slots:
    void handleNetworkReply(QNetworkReply* reply);
    void onTokensRefreshed();
    
private:
    void makeGraphQLRequest(const QString& query, const QString& variables);
    QString getAuthToken() const;
    bool isTokenExpiredError(const QJsonDocument& doc) const;
    QString getCanvasState() const;
    
    std::unique_ptr<QNetworkAccessManager> m_networkManager;
    AuthenticationManager* m_authManager;
    Application* m_application;
    std::unique_ptr<AICommandDispatcher> m_commandDispatcher;
    QString m_pendingQuery;
    bool m_isRetryingAfterRefresh;
};

#endif // CUBITAICLIENT_H