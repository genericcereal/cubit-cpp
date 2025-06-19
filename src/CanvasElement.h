#pragma once
#include "Element.h"
#include <QPointF>
#include <QSizeF>
#include <QRectF>

class CanvasElement : public Element {
    Q_OBJECT
    Q_PROPERTY(qreal x READ x WRITE setX NOTIFY xChanged)
    Q_PROPERTY(qreal y READ y WRITE setY NOTIFY yChanged)
    Q_PROPERTY(qreal width READ width WRITE setWidth NOTIFY widthChanged)
    Q_PROPERTY(qreal height READ height WRITE setHeight NOTIFY heightChanged)
    Q_PROPERTY(bool isDesignElement READ isDesignElement CONSTANT)
    Q_PROPERTY(bool isScriptElement READ isScriptElement CONSTANT)
    
public:
    explicit CanvasElement(ElementType type, const QString &id, QObject *parent = nullptr);
    virtual ~CanvasElement() = default;
    
    // Property getters
    qreal x() const { return canvasPosition.x(); }
    qreal y() const { return canvasPosition.y(); }
    qreal width() const { return canvasSize.width(); }
    qreal height() const { return canvasSize.height(); }
    
    QRectF rect() const { 
        if (!m_boundsValid) {
            updateCachedBounds();
        }
        return m_cachedBounds; 
    }
    const QRectF& cachedBounds() const { 
        if (!m_boundsValid) {
            updateCachedBounds();
        }
        return m_cachedBounds; 
    }
    
    // Hit testing - can be overridden for custom shapes
    virtual bool containsPoint(const QPointF &point) const;
    
    // Property setters
    void setX(qreal x);
    void setY(qreal y);
    void setWidth(qreal w);
    void setHeight(qreal h);
    void setRect(const QRectF &rect);
    
    // Override from Element
    bool isVisual() const override { return true; }
    
    // Virtual methods to identify element category
    virtual bool isDesignElement() const { return false; }
    virtual bool isScriptElement() const { return false; }
    
signals:
    void xChanged();
    void yChanged();
    void widthChanged();
    void heightChanged();
    void geometryChanged();  // Emitted when x, y, width, or height change
    
protected:
    QPointF canvasPosition;
    QSizeF canvasSize;
    
private:
    // Cached bounding box to avoid repeated construction
    mutable QRectF m_cachedBounds;
    mutable bool m_boundsValid = false;
    
    // Update the cached bounds
    void updateCachedBounds() const;
};