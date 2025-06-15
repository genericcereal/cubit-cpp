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
    
public:
    explicit CanvasElement(ElementType type, const QString &id, QObject *parent = nullptr);
    virtual ~CanvasElement() = default;
    
    // Property getters
    qreal x() const { return canvasPosition.x(); }
    qreal y() const { return canvasPosition.y(); }
    qreal width() const { return canvasSize.width(); }
    qreal height() const { return canvasSize.height(); }
    
    QRectF rect() const { return QRectF(canvasPosition, canvasSize); }
    
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
};