#pragma once
#include "DesignElement.h"
#include <QColor>

class Frame : public DesignElement {
    Q_OBJECT
    Q_PROPERTY(FillColor fillColor READ fillColor WRITE setFillColor NOTIFY fillColorChanged)
    Q_PROPERTY(QColor fill READ fill NOTIFY fillColorChanged)
    Q_PROPERTY(QColor borderColor READ borderColor WRITE setBorderColor NOTIFY borderColorChanged)
    Q_PROPERTY(int borderWidth READ borderWidth WRITE setBorderWidth NOTIFY borderWidthChanged)
    Q_PROPERTY(int borderRadius READ borderRadius WRITE setBorderRadius NOTIFY borderRadiusChanged)
    Q_PROPERTY(OverflowMode overflow READ overflow WRITE setOverflow NOTIFY overflowChanged)
    Q_PROPERTY(bool acceptsChildren READ acceptsChildren WRITE setAcceptsChildren NOTIFY acceptsChildrenChanged)
    
public:
    enum OverflowMode {
        Hidden,
        Scroll,
        Visible
    };
    Q_ENUM(OverflowMode)
    
    enum FillColor {
        LightBlue,
        DarkBlue,
        Green,
        Red
    };
    Q_ENUM(FillColor)
    explicit Frame(const QString &id, QObject *parent = nullptr);
    
    // Property getters
    FillColor fillColor() const { return m_fillColor; }
    QColor fill() const;
    QColor borderColor() const { return m_borderColor; }
    int borderWidth() const { return m_borderWidth; }
    int borderRadius() const { return m_borderRadius; }
    OverflowMode overflow() const { return m_overflow; }
    bool acceptsChildren() const { return m_acceptsChildren; }
    
    // Property setters
    void setFillColor(FillColor color);
    void setBorderColor(const QColor &color);
    void setBorderWidth(int width);
    void setBorderRadius(int radius);
    void setOverflow(OverflowMode mode);
    void setAcceptsChildren(bool accepts);
    
signals:
    void fillColorChanged();
    void borderColorChanged();
    void borderWidthChanged();
    void borderRadiusChanged();
    void overflowChanged();
    void acceptsChildrenChanged();
    
private:
    FillColor m_fillColor;
    QColor m_borderColor;
    int m_borderWidth;
    int m_borderRadius;
    OverflowMode m_overflow;
    bool m_acceptsChildren;
};