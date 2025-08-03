#include "GoogleFonts.h"
#include "Secrets.h"
#include <QNetworkRequest>
#include <QJsonDocument>
#include <QJsonArray>
#include <QJsonObject>
#include <QFontDatabase>
#include <QUrl>
#include <QUrlQuery>
#include <QDebug>

GoogleFonts* GoogleFonts::s_instance = nullptr;

GoogleFonts* GoogleFonts::instance() {
    if (!s_instance) {
        s_instance = new GoogleFonts();
    }
    return s_instance;
}

GoogleFonts::GoogleFonts(QObject *parent) 
    : QObject(parent)
    , m_networkManager(new QNetworkAccessManager(this))
    , m_isLoading(false)
    , m_fontListLoaded(false) {
    
    // Fetch the font list on initialization
    fetchFontList();
}

GoogleFonts::~GoogleFonts() {
    if (s_instance == this) {
        s_instance = nullptr;
    }
}

QStringList GoogleFonts::availableFonts() const {
    return m_availableFonts;
}

void GoogleFonts::fetchFontList() {
    if (m_fontListLoaded || m_isLoading) {
        return;
    }
    
    m_isLoading = true;
    emit isLoadingChanged();
    
    QUrl url("https://www.googleapis.com/webfonts/v1/webfonts");
    QUrlQuery query;
    query.addQueryItem("key", Secrets::GOOGLE_FONTS_API_KEY);
    query.addQueryItem("sort", "popularity");
    url.setQuery(query);
    
    QNetworkRequest request(url);
    QNetworkReply* reply = m_networkManager->get(request);
    
    connect(reply, &QNetworkReply::finished, this, [this, reply]() {
        handleFontListReply();
        reply->deleteLater();
    });
}

void GoogleFonts::handleFontListReply() {
    auto reply = qobject_cast<QNetworkReply*>(sender());
    if (!reply) return;
    
    m_isLoading = false;
    emit isLoadingChanged();
    
    if (reply->error() != QNetworkReply::NoError) {
        qWarning() << "Failed to fetch Google Fonts list:" << reply->errorString();
        return;
    }
    
    QByteArray data = reply->readAll();
    QJsonDocument doc = QJsonDocument::fromJson(data);
    
    if (!doc.isObject()) {
        qWarning() << "Invalid JSON response from Google Fonts API";
        return;
    }
    
    QJsonObject root = doc.object();
    m_fontList = root["items"].toArray();
    
    m_availableFonts.clear();
    for (const QJsonValue& value : m_fontList) {
        QJsonObject font = value.toObject();
        QString family = font["family"].toString();
        if (!family.isEmpty()) {
            m_availableFonts.append(family);
        }
    }
    
    m_fontListLoaded = true;
    emit availableFontsChanged();
}

void GoogleFonts::loadFont(const QString& fontFamily, const QString& weight) {
    QString normalizedWeight = normalizeWeight(weight);
    QString key = fontFamily + ":" + normalizedWeight;
    
    if (m_loadedFonts.contains(key)) {
        emit fontLoaded(fontFamily);
        return;
    }
    
    QString url = getFontUrl(fontFamily, normalizedWeight);
    if (url.isEmpty()) {
        emit fontLoadError(fontFamily, "Font not found in Google Fonts");
        return;
    }
    
    QNetworkRequest request((QUrl(url)));
    QNetworkReply* reply = m_networkManager->get(request);
    
    DownloadInfo info;
    info.family = fontFamily;
    info.weight = normalizedWeight;
    m_activeDownloads[reply] = info;
    
    connect(reply, &QNetworkReply::finished, this, [this, fontFamily, normalizedWeight, reply]() {
        handleFontFileReply(fontFamily, normalizedWeight);
        m_activeDownloads.remove(reply);
        reply->deleteLater();
    });
}

void GoogleFonts::loadFontWithAllWeights(const QString& fontFamily) {
    QStringList weights = getAvailableWeights(fontFamily);
    for (const QString& weight : weights) {
        loadFont(fontFamily, weight);
    }
}

void GoogleFonts::handleFontFileReply(const QString& fontFamily, const QString& weight) {
    auto reply = qobject_cast<QNetworkReply*>(sender());
    if (!reply) return;
    
    if (reply->error() != QNetworkReply::NoError) {
        emit fontLoadError(fontFamily, reply->errorString());
        return;
    }
    
    QByteArray fontData = reply->readAll();
    
    // Add font to Qt's font database
    int fontId = QFontDatabase::addApplicationFontFromData(fontData);
    if (fontId == -1) {
        emit fontLoadError(fontFamily, "Failed to load font data");
        return;
    }
    
    QString key = fontFamily + ":" + weight;
    m_loadedFonts[key] = true;
    emit fontLoaded(fontFamily);
}

bool GoogleFonts::isFontLoaded(const QString& fontFamily, const QString& weight) const {
    QString normalizedWeight = normalizeWeight(weight);
    QString key = fontFamily + ":" + normalizedWeight;
    return m_loadedFonts.contains(key);
}

QStringList GoogleFonts::getAvailableWeights(const QString& fontFamily) const {
    QStringList weights;
    
    // Find the font in our list
    for (const QJsonValue& value : m_fontList) {
        QJsonObject font = value.toObject();
        if (font["family"].toString() == fontFamily) {
            QJsonObject files = font["files"].toObject();
            weights = files.keys();
            break;
        }
    }
    
    return weights;
}

QString GoogleFonts::normalizeWeight(const QString& weight) const {
    // Convert numeric weights to named weights
    if (weight == "400" || weight.isEmpty()) return "regular";
    if (weight == "700") return "700";
    if (weight == "300") return "300";
    if (weight == "500") return "500";
    if (weight == "600") return "600";
    if (weight == "800") return "800";
    if (weight == "900") return "900";
    if (weight == "100") return "100";
    if (weight == "200") return "200";
    
    // Handle named weights
    QString lowerWeight = weight.toLower();
    if (lowerWeight == "normal") return "regular";
    if (lowerWeight == "bold") return "700";
    if (lowerWeight == "light") return "300";
    if (lowerWeight == "medium") return "500";
    if (lowerWeight == "semibold") return "600";
    if (lowerWeight == "extrabold") return "800";
    if (lowerWeight == "black") return "900";
    if (lowerWeight == "thin") return "100";
    if (lowerWeight == "extralight") return "200";
    
    return weight;
}

QString GoogleFonts::getFontUrl(const QString& fontFamily, const QString& weight) const {
    QString normalizedWeight = normalizeWeight(weight);
    
    // Find the font in our list
    for (const QJsonValue& value : m_fontList) {
        QJsonObject font = value.toObject();
        if (font["family"].toString() == fontFamily) {
            QJsonObject files = font["files"].toObject();
            
            // Try to find the requested weight
            if (files.contains(normalizedWeight)) {
                return files[normalizedWeight].toString();
            }
            
            // Try numeric version if named version not found
            if (normalizedWeight == "regular" && files.contains("400")) {
                return files["400"].toString();
            }
            if (normalizedWeight == "700" && files.contains("bold")) {
                return files["bold"].toString();
            }
            
            // Fallback to regular weight
            if (files.contains("regular")) {
                return files["regular"].toString();
            }
            if (files.contains("400")) {
                return files["400"].toString();
            }
            
            // Return first available weight
            if (!files.isEmpty()) {
                return files.begin().value().toString();
            }
        }
    }
    
    return QString();
}