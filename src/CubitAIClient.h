#ifndef CUBITAICLIENT_H
#define CUBITAICLIENT_H

#include <QObject>
#include <QNetworkAccessManager>
#include <QString>
#include <memory>

class QNetworkReply;
class AuthenticationManager;

class CubitAIClient : public QObject
{
    Q_OBJECT
    
public:
    explicit CubitAIClient(AuthenticationManager* authManager, QObject *parent = nullptr);
    ~CubitAIClient();
    
    void sendMessage(const QString& description);
    
signals:
    void responseReceived(const QString& response);
    void errorOccurred(const QString& error);
    
private slots:
    void handleNetworkReply(QNetworkReply* reply);
    void onTokensRefreshed();
    
private:
    void makeGraphQLRequest(const QString& query, const QString& variables);
    QString getApiEndpoint() const;
    QString getAuthToken() const;
    bool isTokenExpiredError(const QJsonDocument& doc) const;
    
    std::unique_ptr<QNetworkAccessManager> m_networkManager;
    AuthenticationManager* m_authManager;
    QString m_pendingQuery;
    bool m_isRetryingAfterRefresh;
};

#endif // CUBITAICLIENT_H