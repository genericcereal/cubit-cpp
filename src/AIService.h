#ifndef AISERVICE_H
#define AISERVICE_H

#include <QObject>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QJsonObject>

class AIService : public QObject
{
    Q_OBJECT
    
public:
    explicit AIService(QObject *parent = nullptr);
    ~AIService();
    
    // Q_INVOKABLE method that can be called from JavaScript
    Q_INVOKABLE void callCubitAI(const QString &prompt, const QString &accessToken);
    
signals:
    // Emitted when AI response is received
    void responseReceived(const QString &response);
    
    // Emitted when an error occurs
    void errorOccurred(const QString &error);
    
private slots:
    void handleNetworkReply();
    
private:
    QNetworkAccessManager *m_networkManager;
    
    // GraphQL endpoint
    static const QString GRAPHQL_ENDPOINT;
};

#endif // AISERVICE_H