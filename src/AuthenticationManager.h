#ifndef AUTHENTICATIONMANAGER_H
#define AUTHENTICATIONMANAGER_H

#include <QObject>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QString>
#include <QUrl>
#include <QDesktopServices>
#include <QTimer>
#include <memory>

class AuthenticationManager : public QObject
{
    Q_OBJECT
    Q_PROPERTY(bool isAuthenticated READ isAuthenticated NOTIFY isAuthenticatedChanged)
    Q_PROPERTY(QString userName READ userName NOTIFY userNameChanged)
    Q_PROPERTY(QString userEmail READ userEmail NOTIFY userEmailChanged)
    Q_PROPERTY(bool isLoading READ isLoading NOTIFY isLoadingChanged)

public:
    explicit AuthenticationManager(QObject *parent = nullptr);
    ~AuthenticationManager();

    // Authentication state
    bool isAuthenticated() const { return m_isAuthenticated; }
    QString userName() const { return m_userName; }
    QString userEmail() const { return m_userEmail; }
    bool isLoading() const { return m_isLoading; }
    
    // Token access
    QString getIdToken() const { return m_idToken; }
    QString getAccessToken() const { return m_accessToken; }

    // OAuth configuration
    void setClientId(const QString& clientId) { m_clientId = clientId; }
    void setCognitoDomain(const QString& domain) { m_cognitoDomain = domain; }
    void setRedirectUri(const QString& uri) { m_redirectUri = uri; }

    // Set the authorization code from callback
    void handleAuthorizationCode(const QString& code, const QString& state);

public slots:
    // Start the OAuth flow
    void login();
    void logout();
    
    // Handle URL callback
    bool handleCallback(const QUrl& url);
    
    // Check if we should auto-login on startup
    void checkAutoLogin();
    
    // Refresh the access token using refresh token
    void refreshAccessToken();

signals:
    void isAuthenticatedChanged();
    void userNameChanged();
    void userEmailChanged();
    void isLoadingChanged();
    void authenticationError(const QString& error);
    void authenticationSucceeded();
    void tokensRefreshed();

private slots:
    void handleTokenResponse();
    void handleUserInfoResponse();
    void checkForCallback();

private:
    void setIsAuthenticated(bool authenticated);
    void setUserName(const QString& name);
    void setUserEmail(const QString& email);
    void setIsLoading(bool loading);
    
    // OAuth flow methods
    void exchangeCodeForToken(const QString& code);
    void fetchUserInfo();
    void clearAuthData();
    QString generateRandomState();
    
    // Token persistence
    void saveTokens();
    void loadTokens();
    
    // Network manager
    std::unique_ptr<QNetworkAccessManager> m_networkManager;
    
    // OAuth configuration
    QString m_clientId;
    QString m_cognitoDomain;
    QString m_redirectUri;
    QString m_scope = "email openid phone";
    
    // Authentication state
    bool m_isAuthenticated = false;
    bool m_isLoading = true;  // Start with loading state
    QString m_userName;
    QString m_userEmail;
    
    // OAuth tokens
    QString m_accessToken;
    QString m_idToken;
    QString m_refreshToken;
    
    // OAuth flow state
    QString m_currentState;
    QTimer* m_callbackTimer;
};

#endif // AUTHENTICATIONMANAGER_H