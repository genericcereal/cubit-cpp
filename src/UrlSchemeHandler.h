#ifndef URLSCHEMEHANDLER_H
#define URLSCHEMEHANDLER_H

#include <QObject>
#include <QUrl>
#include <QGuiApplication>


class AuthenticationManager;

class UrlSchemeHandler : public QObject
{
    Q_OBJECT

public:
    explicit UrlSchemeHandler(AuthenticationManager* authManager, QObject *parent = nullptr);
    ~UrlSchemeHandler();
    
    void registerUrlScheme();
    
    // Platform-specific URL handling
    bool handleUrl(const QUrl& url);
    
    // Static instance access for platform callbacks
    static UrlSchemeHandler* instance() { return s_instance; }

signals:
    void urlReceived(const QUrl& url);

private slots:
    void handleUrlSlot(const QUrl& url);

private:
    AuthenticationManager* m_authManager;
    static UrlSchemeHandler* s_instance;
    
#ifdef Q_OS_MAC
    void registerMacUrlScheme();
#endif
    
#ifdef Q_OS_WIN
    void registerWindowsUrlScheme();
#endif
    
#ifdef Q_OS_LINUX
    void registerLinuxUrlScheme();
#endif
};


#endif // URLSCHEMEHANDLER_H