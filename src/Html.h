#pragma once
#include "CanvasElement.h"

class Html : public CanvasElement {
    Q_OBJECT
    Q_PROPERTY(QString html READ html WRITE setHtml NOTIFY htmlChanged)
    Q_PROPERTY(QString url READ url WRITE setUrl NOTIFY urlChanged)
    
public:
    explicit Html(const QString &id, QObject *parent = nullptr);
    
    QString html() const { return m_html; }
    QString url() const { return m_url; }
    
    void setHtml(const QString &html);
    void setUrl(const QString &url);
    
signals:
    void htmlChanged();
    void urlChanged();
    
private:
    QString m_html;
    QString m_url;
};