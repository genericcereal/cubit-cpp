#include "UrlSchemeHandler.h"
#include "AuthenticationManager.h"
#include <QDebug>
#include <QCoreApplication>

#include <QDesktopServices>

#ifdef Q_OS_MAC
#endif

#ifdef Q_OS_WIN
#include <windows.h>
#include <QSettings>
#endif

UrlSchemeHandler* UrlSchemeHandler::s_instance = nullptr;

UrlSchemeHandler::UrlSchemeHandler(AuthenticationManager* authManager, QObject *parent)
    : QObject(parent)
    , m_authManager(authManager)
{
    s_instance = this;
    
    // Connect URL handling
    connect(this, &UrlSchemeHandler::urlReceived, this, [this](const QUrl& url) {
        handleUrl(url);
    });
}

UrlSchemeHandler::~UrlSchemeHandler()
{
    if (s_instance == this) {
        s_instance = nullptr;
    }
}

void UrlSchemeHandler::registerUrlScheme()
{
#ifdef Q_OS_MAC
    registerMacUrlScheme();
#endif
    
#ifdef Q_OS_WIN
    registerWindowsUrlScheme();
#endif
    
#ifdef Q_OS_LINUX
    registerLinuxUrlScheme();
#endif
}

bool UrlSchemeHandler::handleUrl(const QUrl& url)
{
    qDebug() << "UrlSchemeHandler: Received URL:" << url.toString();
    
    if (url.scheme() == "cubitapp" && m_authManager) {
        return m_authManager->handleCallback(url);
    }
    
    return false;
}

#ifdef Q_OS_MAC
void UrlSchemeHandler::registerMacUrlScheme()
{
    // On macOS, we need to handle URLs through QDesktopServices
    QDesktopServices::setUrlHandler("cubitapp", this, "handleUrlSlot");
    
    qDebug() << "Registered cubitapp:// URL scheme handler for macOS";
}
#endif

void UrlSchemeHandler::handleUrlSlot(const QUrl& url)
{
    handleUrl(url);
}

#ifdef Q_OS_WIN
void UrlSchemeHandler::registerWindowsUrlScheme()
{
    // Register the URL scheme in Windows Registry
    QString appPath = QCoreApplication::applicationFilePath();
    appPath = appPath.replace("/", "\\");
    
    QSettings protocolSettings("HKEY_CURRENT_USER\\Software\\Classes\\cubitapp", QSettings::NativeFormat);
    protocolSettings.setValue(".", "URL:CubitApp Protocol");
    protocolSettings.setValue("URL Protocol", "");
    
    QSettings iconSettings("HKEY_CURRENT_USER\\Software\\Classes\\cubitapp\\DefaultIcon", QSettings::NativeFormat);
    iconSettings.setValue(".", appPath + ",0");
    
    QSettings commandSettings("HKEY_CURRENT_USER\\Software\\Classes\\cubitapp\\shell\\open\\command", QSettings::NativeFormat);
    commandSettings.setValue(".", "\"" + appPath + "\" \"%1\"");
    
    qDebug() << "Registered cubitapp:// URL scheme handler for Windows";
}
#endif

#ifdef Q_OS_LINUX
void UrlSchemeHandler::registerLinuxUrlScheme()
{
    // On Linux, URL schemes are typically handled through .desktop files
    // This would need to be installed in the appropriate location
    // For development, we can rely on command line arguments
    qDebug() << "Linux URL scheme registration requires .desktop file installation";
}
#endif