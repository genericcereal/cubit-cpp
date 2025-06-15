#pragma once
#include "DesignElement.h"
#include <QColor>

class Frame : public DesignElement {
    Q_OBJECT
    Q_PROPERTY(QColor backgroundColor READ backgroundColor WRITE setBackgroundColor NOTIFY backgroundColorChanged)
    Q_PROPERTY(QColor borderColor READ borderColor WRITE setBorderColor NOTIFY borderColorChanged)
    Q_PROPERTY(int borderWidth READ borderWidth WRITE setBorderWidth NOTIFY borderWidthChanged)
    Q_PROPERTY(int borderRadius READ borderRadius WRITE setBorderRadius NOTIFY borderRadiusChanged)
    Q_PROPERTY(bool clipContent READ clipContent WRITE setClipContent NOTIFY clipContentChanged)
    
public:
    explicit Frame(const QString &id, QObject *parent = nullptr);
    
    // Property getters
    QColor backgroundColor() const { return m_backgroundColor; }
    QColor borderColor() const { return m_borderColor; }
    int borderWidth() const { return m_borderWidth; }
    int borderRadius() const { return m_borderRadius; }
    bool clipContent() const { return m_clipContent; }
    
    // Property setters
    void setBackgroundColor(const QColor &color);
    void setBorderColor(const QColor &color);
    void setBorderWidth(int width);
    void setBorderRadius(int radius);
    void setClipContent(bool clip);
    
signals:
    void backgroundColorChanged();
    void borderColorChanged();
    void borderWidthChanged();
    void borderRadiusChanged();
    void clipContentChanged();
    
private:
    QColor m_backgroundColor;
    QColor m_borderColor;
    int m_borderWidth;
    int m_borderRadius;
    bool m_clipContent;
};