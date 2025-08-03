#pragma once

#include <QObject>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QFont>
#include <QFontDatabase>
#include <QHash>
#include <QJsonArray>
#include <QJsonObject>

class GoogleFonts : public QObject {
    Q_OBJECT
    Q_PROPERTY(QStringList availableFonts READ availableFonts NOTIFY availableFontsChanged)
    Q_PROPERTY(bool isLoading READ isLoading NOTIFY isLoadingChanged)
    
public:
    static GoogleFonts* instance();
    
    // Get list of available Google Fonts
    QStringList availableFonts() const;
    bool isLoading() const { return m_isLoading; }
    
    // Load a specific font family with weight
    Q_INVOKABLE void loadFont(const QString& fontFamily, const QString& weight = "regular");
    
    // Load all common weights for a font family
    Q_INVOKABLE void loadFontWithAllWeights(const QString& fontFamily);
    
    // Check if a font with specific weight is already loaded
    Q_INVOKABLE bool isFontLoaded(const QString& fontFamily, const QString& weight = "regular") const;
    
    // Get available weights for a font family
    Q_INVOKABLE QStringList getAvailableWeights(const QString& fontFamily) const;
    
    // Get the font URL for a specific family and weight
    QString getFontUrl(const QString& fontFamily, const QString& weight = "regular") const;
    
signals:
    void availableFontsChanged();
    void fontLoaded(const QString& fontFamily);
    void fontLoadError(const QString& fontFamily, const QString& error);
    void isLoadingChanged();
    
private:
    explicit GoogleFonts(QObject *parent = nullptr);
    ~GoogleFonts();
    
    void fetchFontList();
    void handleFontListReply();
    void handleFontFileReply(const QString& fontFamily, const QString& weight);
    QString normalizeWeight(const QString& weight) const;
    
    static GoogleFonts* s_instance;
    QNetworkAccessManager* m_networkManager;
    QJsonArray m_fontList;
    QStringList m_availableFonts;
    QHash<QString, bool> m_loadedFonts;  // Key: "family:weight"
    bool m_isLoading;
    bool m_fontListLoaded;
    
    // Active downloads - store both family and weight
    struct DownloadInfo {
        QString family;
        QString weight;
    };
    QHash<QNetworkReply*, DownloadInfo> m_activeDownloads;
};