#include "AuthenticationManager.h"
#include "Config.h"
#include <QJsonDocument>
#include <QJsonObject>
#include <QUrlQuery>
#include <QRandomGenerator>
#include <QDebug>
#include <QApplication>
#include <QSettings>

#ifdef Q_OS_WIN
#include <windows.h>
#include <shellapi.h>
#endif

AuthenticationManager::AuthenticationManager(QObject *parent)
    : QObject(parent)
    , m_networkManager(std::make_unique<QNetworkAccessManager>())
    , m_callbackTimer(new QTimer(this))
{
    // Configure default values from Config
    m_cognitoDomain = Config::COGNITO_DOMAIN;
    m_clientId = Config::COGNITO_CLIENT_ID;
    m_redirectUri = Config::COGNITO_REDIRECT_URI;
    m_scope = Config::COGNITO_SCOPE;
    
    // Setup callback timer for checking if authentication completed
    m_callbackTimer->setInterval(1000); // Check every second
    connect(m_callbackTimer, &QTimer::timeout, this, &AuthenticationManager::checkForCallback);
    
    // Load saved tokens if available
    loadTokens();
}

AuthenticationManager::~AuthenticationManager() = default;

void AuthenticationManager::checkAutoLogin()
{
    // If we have stored tokens, verify they're still valid
    if (m_isAuthenticated && !m_accessToken.isEmpty()) {
        qDebug() << "User already authenticated, skipping login";
        // Set loading to false since we're done checking
        setIsLoading(false);
        return;
    }
    
    // Auto-login on startup if not already authenticated
    if (!m_isAuthenticated) {
        login();
    }
}

void AuthenticationManager::login()
{
    if (m_isLoading) {
        return;
    }
    
    setIsLoading(true);
    
    // Generate random state for CSRF protection
    m_currentState = generateRandomState();
    
    // Build the authorization URL
    QUrl authUrl(m_cognitoDomain + "/login");
    QUrlQuery query;
    query.addQueryItem("client_id", m_clientId);
    query.addQueryItem("response_type", "code");
    query.addQueryItem("scope", m_scope);
    query.addQueryItem("redirect_uri", m_redirectUri);
    query.addQueryItem("state", m_currentState);
    authUrl.setQuery(query);
    
    qDebug() << "Opening authentication URL:" << authUrl.toString();
    
    // Open the URL in the default browser
    bool opened = QDesktopServices::openUrl(authUrl);
    
    if (!opened) {
        qWarning() << "Failed to open URL with QDesktopServices::openUrl()";
        
#ifdef Q_OS_WIN
        // Windows-specific fallback using ShellExecute
        QString urlString = authUrl.toString();
        HINSTANCE result = ShellExecuteW(nullptr, L"open", 
            reinterpret_cast<const wchar_t*>(urlString.utf16()), 
            nullptr, nullptr, SW_SHOWNORMAL);
        
        if (reinterpret_cast<INT_PTR>(result) <= 32) {
            qCritical() << "ShellExecute failed with error code:" << reinterpret_cast<INT_PTR>(result);
            emit authenticationError("Failed to open login URL in browser. Please use the 'Open Login URL in Browser' button.");
        } else {
            qDebug() << "Successfully opened URL using ShellExecute";
        }
#else
        emit authenticationError("Failed to open login URL in browser. Please use the 'Open Login URL in Browser' button.");
#endif
    } else {
        qDebug() << "Successfully opened URL using QDesktopServices";
    }
    
    // Start checking for callback
    m_callbackTimer->start();
}

void AuthenticationManager::logout()
{
    clearAuthData();
    
    // Optionally open logout URL
    QUrl logoutUrl(m_cognitoDomain + "/logout");
    QUrlQuery query;
    query.addQueryItem("client_id", m_clientId);
    query.addQueryItem("logout_uri", m_redirectUri);
    logoutUrl.setQuery(query);
    
    QDesktopServices::openUrl(logoutUrl);
}

bool AuthenticationManager::handleCallback(const QUrl& url)
{
    qDebug() << "Handling callback URL:" << url.toString();
    
    // Check if this is our callback URL
    if (url.scheme() != "cubitapp" || url.host() != "callback") {
        return false;
    }
    
    // Stop the callback timer
    m_callbackTimer->stop();
    
    // Parse the query parameters
    QUrlQuery query(url);
    QString code = query.queryItemValue("code");
    QString state = query.queryItemValue("state");
    QString error = query.queryItemValue("error");
    
    if (!error.isEmpty()) {
        setIsLoading(false);
        emit authenticationError(error);
        return true;
    }
    
    // Verify state to prevent CSRF attacks
    if (state != m_currentState) {
        setIsLoading(false);
        emit authenticationError("Invalid state parameter");
        return true;
    }
    
    if (!code.isEmpty()) {
        handleAuthorizationCode(code, state);
        return true;
    }
    
    setIsLoading(false);
    emit authenticationError("No authorization code received");
    return true;
}

void AuthenticationManager::handleAuthorizationCode(const QString& code, const QString& state)
{
    if (state != m_currentState) {
        setIsLoading(false);
        emit authenticationError("Invalid state parameter");
        return;
    }
    
    exchangeCodeForToken(code);
}

void AuthenticationManager::exchangeCodeForToken(const QString& code)
{
    QUrl tokenUrl(m_cognitoDomain + "/oauth2/token");
    
    QUrlQuery postData;
    postData.addQueryItem("grant_type", "authorization_code");
    postData.addQueryItem("client_id", m_clientId);
    postData.addQueryItem("code", code);
    postData.addQueryItem("redirect_uri", m_redirectUri);
    
    QNetworkRequest request(tokenUrl);
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/x-www-form-urlencoded");
    
    QNetworkReply* reply = m_networkManager->post(request, postData.toString(QUrl::FullyEncoded).toUtf8());
    connect(reply, &QNetworkReply::finished, this, &AuthenticationManager::handleTokenResponse);
}

void AuthenticationManager::handleTokenResponse()
{
    QNetworkReply* reply = qobject_cast<QNetworkReply*>(sender());
    if (!reply) {
        return;
    }
    
    reply->deleteLater();
    
    if (reply->error() != QNetworkReply::NoError) {
        setIsLoading(false);
        emit authenticationError(reply->errorString());
        return;
    }
    
    QByteArray response = reply->readAll();
    QJsonDocument doc = QJsonDocument::fromJson(response);
    QJsonObject obj = doc.object();
    
    if (obj.contains("error")) {
        setIsLoading(false);
        emit authenticationError(obj["error_description"].toString());
        return;
    }
    
    // Store tokens
    m_accessToken = obj["access_token"].toString();
    m_idToken = obj["id_token"].toString();
    m_refreshToken = obj["refresh_token"].toString();
    
    if (m_accessToken.isEmpty()) {
        setIsLoading(false);
        emit authenticationError("No access token received");
        return;
    }
    
    // Fetch user info
    fetchUserInfo();
}

void AuthenticationManager::fetchUserInfo()
{
    QUrl userInfoUrl(m_cognitoDomain + "/oauth2/userInfo");
    
    QNetworkRequest request(userInfoUrl);
    request.setRawHeader("Authorization", ("Bearer " + m_accessToken).toUtf8());
    
    QNetworkReply* reply = m_networkManager->get(request);
    connect(reply, &QNetworkReply::finished, this, &AuthenticationManager::handleUserInfoResponse);
}

void AuthenticationManager::handleUserInfoResponse()
{
    QNetworkReply* reply = qobject_cast<QNetworkReply*>(sender());
    if (!reply) {
        return;
    }
    
    reply->deleteLater();
    
    if (reply->error() != QNetworkReply::NoError) {
        setIsLoading(false);
        emit authenticationError(reply->errorString());
        return;
    }
    
    QByteArray response = reply->readAll();
    QJsonDocument doc = QJsonDocument::fromJson(response);
    QJsonObject obj = doc.object();
    
    // Extract user information
    QString email = obj["email"].toString();
    QString username = obj["username"].toString();
    
    if (!username.isEmpty()) {
        setUserName(username);
    }
    if (!email.isEmpty()) {
        setUserEmail(email);
    }
    
    setIsAuthenticated(true);
    setIsLoading(false);
    
    // Save tokens for persistent authentication
    saveTokens();
    
    emit authenticationSucceeded();
}

void AuthenticationManager::checkForCallback()
{
    // This is a placeholder for platform-specific callback checking
    // In a real implementation, this would check if the app received a URL callback
    // For now, we'll rely on the platform-specific URL handler to call handleCallback
}

void AuthenticationManager::clearAuthData()
{
    m_accessToken.clear();
    m_idToken.clear();
    m_refreshToken.clear();
    m_currentState.clear();
    m_isRefreshing = false;
    
    setUserName("");
    setUserEmail("");
    setIsAuthenticated(false);
    
    // Clear saved tokens
    QSettings settings;
    settings.remove("auth/idToken");
    settings.remove("auth/accessToken");
    settings.remove("auth/refreshToken");
    settings.remove("auth/userName");
    settings.remove("auth/userEmail");
}

QString AuthenticationManager::generateRandomState()
{
    const QString possibleCharacters("ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789");
    const int randomStringLength = 32;
    QString randomString;
    
    for(int i = 0; i < randomStringLength; ++i) {
        int index = QRandomGenerator::global()->bounded(possibleCharacters.length());
        randomString.append(possibleCharacters.at(index));
    }
    
    return randomString;
}

void AuthenticationManager::setIsAuthenticated(bool authenticated)
{
    if (m_isAuthenticated != authenticated) {
        m_isAuthenticated = authenticated;
        emit isAuthenticatedChanged();
    }
}

void AuthenticationManager::setUserName(const QString& name)
{
    if (m_userName != name) {
        m_userName = name;
        emit userNameChanged();
    }
}

void AuthenticationManager::setUserEmail(const QString& email)
{
    if (m_userEmail != email) {
        m_userEmail = email;
        emit userEmailChanged();
    }
}

void AuthenticationManager::setIsLoading(bool loading)
{
    if (m_isLoading != loading) {
        m_isLoading = loading;
        emit isLoadingChanged();
    }
}

void AuthenticationManager::saveTokens()
{
    QSettings settings;
    settings.setValue("auth/idToken", m_idToken);
    settings.setValue("auth/accessToken", m_accessToken);
    settings.setValue("auth/refreshToken", m_refreshToken);
    settings.setValue("auth/userName", m_userName);
    settings.setValue("auth/userEmail", m_userEmail);
}

void AuthenticationManager::loadTokens()
{
    QSettings settings;
    m_idToken = settings.value("auth/idToken").toString();
    m_accessToken = settings.value("auth/accessToken").toString();
    m_refreshToken = settings.value("auth/refreshToken").toString();
    QString savedUserName = settings.value("auth/userName").toString();
    QString savedUserEmail = settings.value("auth/userEmail").toString();
    
    if (!m_idToken.isEmpty() && !m_refreshToken.isEmpty()) {
        setUserName(savedUserName);
        setUserEmail(savedUserEmail);
        setIsAuthenticated(true);
    }
}

void AuthenticationManager::refreshAccessToken() {
    if (m_refreshToken.isEmpty()) {
        emit authenticationError("No refresh token available");
        return;
    }
    
    // Prevent concurrent refresh attempts
    if (m_isRefreshing) {
        qDebug() << "Token refresh already in progress, skipping duplicate request";
        return;
    }
    
    m_isRefreshing = true;
    setIsLoading(true);
    
    // Construct the token refresh URL
    QString tokenUrl = m_cognitoDomain + "/oauth2/token";
    
    // Prepare the refresh request
    QNetworkRequest request;
    request.setUrl(QUrl(tokenUrl));
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/x-www-form-urlencoded");
    
    // Prepare the refresh token grant parameters
    QUrlQuery params;
    params.addQueryItem("grant_type", "refresh_token");
    params.addQueryItem("client_id", m_clientId);
    params.addQueryItem("refresh_token", m_refreshToken);
    
    QNetworkReply* reply = m_networkManager->post(request, params.toString(QUrl::FullyEncoded).toUtf8());
    
    connect(reply, &QNetworkReply::finished, this, [this, reply]() {
        reply->deleteLater();
        
        if (reply->error() != QNetworkReply::NoError) {
            QString errorString = reply->errorString();
            qDebug() << "Token refresh error:" << errorString;
            
            // If refresh fails, clear authentication and require re-login
            clearAuthData();
            emit authenticationError("Token refresh failed: " + errorString);
            setIsLoading(false);
            m_isRefreshing = false;
            return;
        }
        
        QByteArray response = reply->readAll();
        QJsonDocument doc = QJsonDocument::fromJson(response);
        QJsonObject obj = doc.object();
        
        if (obj.contains("access_token")) {
            // Update tokens
            m_accessToken = obj["access_token"].toString();
            m_idToken = obj["id_token"].toString();
            
            // Save the new tokens
            saveTokens();
            
            qDebug() << "Successfully refreshed access token";
            m_isRefreshing = false;
            emit tokensRefreshed();
            
            // Fetch updated user info with the new token
            fetchUserInfo();
        } else {
            QString error = obj["error"].toString();
            qDebug() << "Token refresh failed:" << error;
            clearAuthData();
            emit authenticationError("Token refresh failed: " + error);
            m_isRefreshing = false;
        }
        
        setIsLoading(false);
    });
}

QString AuthenticationManager::getLoginUrl() const
{
    // Generate random state for CSRF protection
    QString state = const_cast<AuthenticationManager*>(this)->generateRandomState();
    const_cast<AuthenticationManager*>(this)->m_currentState = state;
    
    // Build the authorization URL
    QUrl authUrl(m_cognitoDomain + "/login");
    QUrlQuery query;
    query.addQueryItem("client_id", m_clientId);
    query.addQueryItem("response_type", "code");
    query.addQueryItem("scope", m_scope);
    query.addQueryItem("redirect_uri", m_redirectUri);
    query.addQueryItem("state", state);
    authUrl.setQuery(query);
    
    return authUrl.toString();
}