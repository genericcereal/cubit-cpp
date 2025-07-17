#include <QApplication>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include <QtWebEngineQuick>
#include <QFileOpenEvent>
#include <QUrl>
#include <QUrlQuery>
#include <QDesktopServices>
#include <QCryptographicHash>
#include <QUuid>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QFile>
#include <QDebug>
#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>

// AWS SDK includes - commented out for now
// #include <aws/core/Aws.h>
// #include <aws/cognito-idp/CognitoIdentityProviderClient.h>
// #include <aws/cognito-idp/model/InitiateAuthRequest.h>
// #include <aws/cognito-idp/model/InitiateAuthResult.h>

// macOS Keychain includes
#ifdef Q_OS_MACOS
#include <Security/Security.h>
#endif

#include "Element.h"
#include "CanvasElement.h"
#include "DesignElement.h"
#include "ScriptElement.h"
#include "Frame.h"
#include "Text.h"
#include "platforms/web/WebTextInput.h"
#include "Variable.h"
#include "Component.h"
#include "FrameComponentInstance.h"
#include "FrameComponentVariant.h"
#include "TextComponentVariant.h"
#include "TextComponentInstance.h"
#include "Node.h"
#include "Edge.h"
#include "CanvasController.h"
#include "DesignCanvas.h"
#include "ElementModel.h"
#include "SelectionManager.h"
#include "ViewportCache.h"
#include "ConsoleMessageRepository.h"
#include "Application.h"
#include "Project.h"
#include "Panels.h"
#include "Scripts.h"
#include "ScriptCompiler.h"
#include "ElementFilterProxy.h"
#include "PrototypeController.h"
#include "DesignControlsController.h"

// Keychain helper functions
namespace KeychainHelper {
#ifdef Q_OS_MACOS
    bool saveToKeychain(const QString& service, const QString& account, const QString& password) {
        OSStatus status = SecKeychainAddGenericPassword(
            NULL,  // default keychain
            service.length(), service.toUtf8().data(),
            account.length(), account.toUtf8().data(),
            password.length(), password.toUtf8().data(),
            NULL  // don't need the item reference
        );
        
        if (status == errSecDuplicateItem) {
            // Update existing item
            SecKeychainItemRef itemRef = NULL;
            status = SecKeychainFindGenericPassword(
                NULL,
                service.length(), service.toUtf8().data(),
                account.length(), account.toUtf8().data(),
                NULL, NULL,
                &itemRef
            );
            
            if (status == errSecSuccess && itemRef) {
                status = SecKeychainItemModifyAttributesAndData(
                    itemRef,
                    NULL,  // no attribute changes
                    password.length(),
                    password.toUtf8().data()
                );
                CFRelease(itemRef);
            }
        }
        
        return status == errSecSuccess;
    }
    
    QString readFromKeychain(const QString& service, const QString& account) {
        UInt32 passwordLength;
        void* passwordData;
        
        OSStatus status = SecKeychainFindGenericPassword(
            NULL,
            service.length(), service.toUtf8().data(),
            account.length(), account.toUtf8().data(),
            &passwordLength, &passwordData,
            NULL
        );
        
        QString password;
        if (status == errSecSuccess) {
            password = QString::fromUtf8(reinterpret_cast<char*>(passwordData), passwordLength);
            SecKeychainItemFreeContent(NULL, passwordData);
        }
        
        return password;
    }
#else
    // Fallback for non-macOS platforms
    bool saveToKeychain(const QString&, const QString&, const QString&) {
        qWarning() << "Keychain storage not implemented for this platform";
        return false;
    }
    
    QString readFromKeychain(const QString&, const QString&) {
        qWarning() << "Keychain storage not implemented for this platform";
        return QString();
    }
#endif
}

class UrlEventFilter : public QObject {
    Q_OBJECT
signals:
    void urlReceived(const QString &url);
    
protected:
    bool eventFilter(QObject* obj, QEvent* ev) override {
        if (ev->type() == QEvent::FileOpen) {
            auto *fe = static_cast<QFileOpenEvent*>(ev);
            if (!fe->url().isEmpty()) {
                emit urlReceived(fe->url().toString());
            }
        }
        return QObject::eventFilter(obj, ev);
    }
};

// OAuth helper class to manage PKCE flow
class OAuthManager : public QObject {
    Q_OBJECT
    
public:
    OAuthManager(QObject* parent = nullptr) : QObject(parent), m_networkManager(new QNetworkAccessManager(this)) {
        // Try to restore session on startup
        checkExistingTokens();
    }
    
    Q_INVOKABLE void startOAuthFlow() {
        // Generate PKCE code verifier and challenge
        m_codeVerifier = QUuid::createUuid().toByteArray().toBase64(QByteArray::Base64UrlEncoding | QByteArray::OmitTrailingEquals);
        QByteArray challenge = QCryptographicHash::hash(m_codeVerifier, QCryptographicHash::Sha256)
                                  .toBase64(QByteArray::Base64UrlEncoding | QByteArray::OmitTrailingEquals);
        
        // Load configuration from aws-exports.json
        QFile file(":/resources/aws-exports.json");
        if (!file.open(QIODevice::ReadOnly)) {
            qCritical() << "Failed to open aws-exports.json";
            return;
        }
        
        QJsonDocument doc = QJsonDocument::fromJson(file.readAll());
        file.close();
        
        if (doc.isNull() || !doc.isObject()) {
            qCritical() << "Failed to parse aws-exports.json";
            return;
        }
        
        auto cfg = doc.object();
        auto oauth = cfg["oauth"].toObject();
        
        QString domain = oauth["domain"].toString();
        QString clientId = cfg["aws_user_pools_web_client_id"].toString();
        QString redirect = oauth["redirectSignIn"].toString();
        
        // Build authorization URL
        QUrl authUrl = QUrl("https://" + domain + "/oauth2/authorize");
        QUrlQuery q;
        q.addQueryItem("response_type", "code");
        q.addQueryItem("client_id", clientId);
        q.addQueryItem("redirect_uri", redirect);
        
        // Join scope array
        QJsonArray scopeArray = oauth["scope"].toArray();
        QStringList scopes;
        for (const auto& value : scopeArray) {
            scopes.append(value.toString());
        }
        q.addQueryItem("scope", scopes.join(" "));
        
        q.addQueryItem("code_challenge", challenge);
        q.addQueryItem("code_challenge_method", "S256");
        authUrl.setQuery(q);
        
        qDebug() << "Opening authorization URL:" << authUrl.toString();
        
        // Open in system browser
        QDesktopServices::openUrl(authUrl);
    }
    
    void handleCallback(const QString& code) {
        qDebug() << "Handling OAuth callback with code:" << code;
        
        // Exchange authorization code for tokens
        exchangeCodeForTokens(code);
    }
    
    Q_INVOKABLE void refreshTokens() {
        QString refreshToken = KeychainHelper::readFromKeychain("com.cubit.app", "refresh_token");
        if (refreshToken.isEmpty()) {
            qWarning() << "No refresh token found in keychain";
            emit authenticationFailed("No refresh token available");
            return;
        }
        
        performTokenRefresh(refreshToken);
    }
    
    Q_INVOKABLE QString getAccessToken() const { return m_accessToken; }
    Q_INVOKABLE QString getIdToken() const { return m_idToken; }
    Q_INVOKABLE bool isAuthenticated() const { return !m_accessToken.isEmpty(); }
    
signals:
    void authenticationSucceeded();
    void authenticationFailed(const QString& error);
    
private:
    void exchangeCodeForTokens(const QString& authCode) {
        // Load configuration
        QFile file(":/resources/aws-exports.json");
        if (!file.open(QIODevice::ReadOnly)) {
            emit authenticationFailed("Failed to open aws-exports.json");
            return;
        }
        
        QJsonDocument doc = QJsonDocument::fromJson(file.readAll());
        file.close();
        
        auto cfg = doc.object();
        auto oauth = cfg["oauth"].toObject();
        
        QString domain = oauth["domain"].toString();
        QString clientId = cfg["aws_user_pools_web_client_id"].toString();
        QString redirect = oauth["redirectSignIn"].toString();
        
        // Construct token endpoint URL
        QUrl tokenUrl("https://" + domain + "/oauth2/token");
        
        // Prepare POST data
        QUrlQuery postData;
        postData.addQueryItem("grant_type", "authorization_code");
        postData.addQueryItem("client_id", clientId);
        postData.addQueryItem("code", authCode);
        postData.addQueryItem("redirect_uri", redirect);
        postData.addQueryItem("code_verifier", QString::fromUtf8(m_codeVerifier));
        
        // Create network request
        QNetworkRequest request(tokenUrl);
        request.setHeader(QNetworkRequest::ContentTypeHeader, "application/x-www-form-urlencoded");
        
        // Send POST request
        QNetworkReply* reply = m_networkManager->post(request, postData.toString(QUrl::FullyEncoded).toUtf8());
        
        // Handle response
        connect(reply, &QNetworkReply::finished, this, [this, reply]() {
            if (reply->error() == QNetworkReply::NoError) {
                QByteArray response = reply->readAll();
                QJsonDocument jsonDoc = QJsonDocument::fromJson(response);
                
                if (!jsonDoc.isNull() && jsonDoc.isObject()) {
                    QJsonObject tokens = jsonDoc.object();
                    
                    m_idToken = tokens["id_token"].toString();
                    m_accessToken = tokens["access_token"].toString();
                    QString refreshToken = tokens["refresh_token"].toString();
                    
                    qDebug() << "Token exchange successful";
                    
                    // Save refresh token to keychain
                    if (!refreshToken.isEmpty()) {
                        if (KeychainHelper::saveToKeychain("com.cubit.app", "refresh_token", refreshToken)) {
                            qDebug() << "Refresh token saved to keychain";
                        } else {
                            qWarning() << "Failed to save refresh token to keychain";
                        }
                    }
                    
                    emit authenticationSucceeded();
                } else {
                    emit authenticationFailed("Invalid token response format");
                }
            } else {
                QString errorMsg = QString("Token exchange failed: %1").arg(reply->errorString());
                QByteArray errorResponse = reply->readAll();
                if (!errorResponse.isEmpty()) {
                    errorMsg += " - " + QString::fromUtf8(errorResponse);
                }
                qCritical() << errorMsg;
                emit authenticationFailed(errorMsg);
            }
            
            reply->deleteLater();
        });
    }
    
    void checkExistingTokens() {
        QString refreshToken = KeychainHelper::readFromKeychain("com.cubit.app", "refresh_token");
        if (!refreshToken.isEmpty()) {
            qDebug() << "Found existing refresh token, attempting to refresh";
            performTokenRefresh(refreshToken);
        }
    }
    
    void performTokenRefresh(const QString& refreshToken) {
        // Load configuration
        QFile file(":/resources/aws-exports.json");
        if (!file.open(QIODevice::ReadOnly)) {
            emit authenticationFailed("Failed to open aws-exports.json");
            return;
        }
        
        QJsonDocument doc = QJsonDocument::fromJson(file.readAll());
        file.close();
        
        auto cfg = doc.object();
        auto oauth = cfg["oauth"].toObject();
        
        QString domain = oauth["domain"].toString();
        QString clientId = cfg["aws_user_pools_web_client_id"].toString();
        
        // Construct token endpoint URL
        QUrl tokenUrl("https://" + domain + "/oauth2/token");
        
        // Prepare POST data for refresh
        QUrlQuery postData;
        postData.addQueryItem("grant_type", "refresh_token");
        postData.addQueryItem("client_id", clientId);
        postData.addQueryItem("refresh_token", refreshToken);
        
        // Create network request
        QNetworkRequest request(tokenUrl);
        request.setHeader(QNetworkRequest::ContentTypeHeader, "application/x-www-form-urlencoded");
        
        // Send POST request
        QNetworkReply* reply = m_networkManager->post(request, postData.toString(QUrl::FullyEncoded).toUtf8());
        
        // Handle response
        connect(reply, &QNetworkReply::finished, this, [this, reply]() {
            if (reply->error() == QNetworkReply::NoError) {
                QByteArray response = reply->readAll();
                QJsonDocument jsonDoc = QJsonDocument::fromJson(response);
                
                if (!jsonDoc.isNull() && jsonDoc.isObject()) {
                    QJsonObject tokens = jsonDoc.object();
                    
                    m_idToken = tokens["id_token"].toString();
                    m_accessToken = tokens["access_token"].toString();
                    
                    qDebug() << "Token refresh successful";
                    emit authenticationSucceeded();
                } else {
                    emit authenticationFailed("Invalid refresh token response format");
                }
            } else {
                QString errorMsg = QString("Token refresh failed: %1").arg(reply->errorString());
                QByteArray errorResponse = reply->readAll();
                if (!errorResponse.isEmpty()) {
                    errorMsg += " - " + QString::fromUtf8(errorResponse);
                }
                qCritical() << errorMsg;
                emit authenticationFailed(errorMsg);
            }
            
            reply->deleteLater();
        });
    }
    
    QByteArray m_codeVerifier;
    QString m_accessToken;
    QString m_idToken;
    QNetworkAccessManager* m_networkManager;
};

#include "main.moc"

int main(int argc, char *argv[])
{
    QtWebEngineQuick::initialize();

    // Enable high DPI scaling for better rendering quality
    QApplication::setHighDpiScaleFactorRoundingPolicy(Qt::HighDpiScaleFactorRoundingPolicy::PassThrough);

    // Enable antialiasing for smoother rendering
    QSurfaceFormat format = QSurfaceFormat::defaultFormat();
    format.setSamples(8); // Increase to 8x MSAA
    QSurfaceFormat::setDefaultFormat(format);

    QApplication app(argc, argv);
    app.setApplicationName("Cubit");
    app.setOrganizationName("Cubit");

    // Install URL event filter to handle callback URLs
    UrlEventFilter urlFilter;
    app.installEventFilter(&urlFilter);
    
    // Create OAuth manager
    OAuthManager* oauthManager = new OAuthManager(&app);

    QQmlApplicationEngine engine;

    // Register C++ types with QML
    qmlRegisterUncreatableType<Element>("Cubit", 1, 0, "Element", "Element is an abstract base class");
    qmlRegisterUncreatableType<CanvasElement>("Cubit", 1, 0, "CanvasElement", "CanvasElement is an abstract base class");
    qmlRegisterUncreatableType<DesignElement>("Cubit", 1, 0, "DesignElement", "DesignElement is an abstract base class");
    qmlRegisterUncreatableType<ScriptElement>("Cubit", 1, 0, "ScriptElement", "ScriptElement is an abstract base class");
    qmlRegisterType<Frame>("Cubit", 1, 0, "Frame");
    qmlRegisterType<Text>("Cubit", 1, 0, "TextElement");  // Rename to avoid conflict with QML Text
    qmlRegisterType<WebTextInput>("Cubit", 1, 0, "WebTextInput");
    qmlRegisterType<Variable>("Cubit", 1, 0, "Variable");
    qmlRegisterType<Component>("Cubit", 1, 0, "ComponentElement");
    qmlRegisterType<FrameComponentInstance>("Cubit", 1, 0, "FrameComponentInstance");
    qmlRegisterType<FrameComponentVariant>("Cubit", 1, 0, "FrameComponentVariant");
    qmlRegisterType<TextComponentVariant>("Cubit", 1, 0, "TextComponentVariant");
    qmlRegisterType<TextComponentInstance>("Cubit", 1, 0, "TextComponentInstance");
    qmlRegisterType<Node>("Cubit", 1, 0, "Node");
    qmlRegisterType<Edge>("Cubit", 1, 0, "Edge");
    
    // Register singleton controllers
    qmlRegisterType<CanvasController>("Cubit", 1, 0, "CanvasController");
    qmlRegisterType<DesignCanvas>("Cubit", 1, 0, "DesignCanvas");
    qmlRegisterType<ElementModel>("Cubit", 1, 0, "ElementModel");
    qmlRegisterType<SelectionManager>("Cubit", 1, 0, "SelectionManager");
    qmlRegisterType<ViewportCache>("Cubit", 1, 0, "ViewportCache");
    qmlRegisterType<Project>("Cubit", 1, 0, "CanvasData");
    qmlRegisterType<Panels>("Cubit", 1, 0, "Panels");
    qmlRegisterType<Scripts>("Cubit", 1, 0, "Scripts");
    qmlRegisterType<ScriptCompiler>("Cubit", 1, 0, "ScriptCompiler");
    qmlRegisterType<ElementFilterProxy>("Cubit", 1, 0, "ElementFilterProxy");
    qmlRegisterType<PrototypeController>("Cubit", 1, 0, "PrototypeController");
    
    // Register singleton ConsoleMessageRepository
    qmlRegisterSingletonType<ConsoleMessageRepository>("Cubit", 1, 0, "ConsoleMessageRepository",
        [](QQmlEngine *engine, QJSEngine *scriptEngine) -> QObject * {
            Q_UNUSED(engine)
            Q_UNUSED(scriptEngine)
            return ConsoleMessageRepository::instance();
        });

    // Register Application singleton
    Application* appInstance = new Application(&app);
    qmlRegisterSingletonType<Application>("Cubit", 1, 0, "Application",
        [appInstance](QQmlEngine *engine, QJSEngine *scriptEngine) -> QObject * {
            Q_UNUSED(engine)
            Q_UNUSED(scriptEngine)
            return appInstance;
        });
    
    // Register OAuthManager singleton
    qmlRegisterSingletonType<OAuthManager>("Cubit", 1, 0, "OAuthManager",
        [oauthManager](QQmlEngine *engine, QJSEngine *scriptEngine) -> QObject * {
            Q_UNUSED(engine)
            Q_UNUSED(scriptEngine)
            return oauthManager;
        });

    // Connect URL event filter to OAuth manager
    QObject::connect(&urlFilter, &UrlEventFilter::urlReceived,
                     [oauthManager](const QString &url) {
        // Parse the URL and extract the code parameter
        QUrl callbackUrl(url);
        if (callbackUrl.scheme() == "cubitapp") {
            QUrlQuery query(callbackUrl);
            QString code = query.queryItemValue("code");
            if (!code.isEmpty()) {
                oauthManager->handleCallback(code);
            }
        }
    });

    // Create and register DesignControlsController as context property
    DesignControlsController* designControlsController = new DesignControlsController(appInstance, &app);
    engine.rootContext()->setContextProperty("designControls", designControlsController);

    // Register QML singleton
    qmlRegisterSingletonType(QUrl("qrc:/qml/Config.qml"), "Cubit.UI", 1, 0, "Config");

    const QUrl url(QStringLiteral("qrc:/qml/main.qml"));
    QObject::connect(&engine, &QQmlApplicationEngine::objectCreated,
                     &app, [url](QObject *obj, const QUrl &objUrl) {
        if (!obj && url == objUrl)
            QCoreApplication::exit(-1);
    }, Qt::QueuedConnection);
    engine.load(url);

    return app.exec();
}