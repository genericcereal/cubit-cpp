#pragma once
#include "Element.h"
#include <QPointF>
#include <QSizeF>
#include <QRectF>
#include <QMetaObject>
#include <QStringList>
#include "ConnectionManager.h"

class CanvasElement : public Element {
    Q_OBJECT
    Q_PROPERTY(qreal x READ x WRITE setX NOTIFY xChanged)
    Q_PROPERTY(qreal y READ y WRITE setY NOTIFY yChanged)
    Q_PROPERTY(qreal width READ width WRITE setWidth NOTIFY widthChanged)
    Q_PROPERTY(qreal height READ height WRITE setHeight NOTIFY heightChanged)
    Q_PROPERTY(bool isDesignElement READ isDesignElement CONSTANT)
    Q_PROPERTY(bool isScriptElement READ isScriptElement CONSTANT)
    Q_PROPERTY(CanvasElement* parentElement READ parentElement WRITE setParentElement NOTIFY parentElementChanged)
    Q_PROPERTY(bool mouseEventsEnabled READ mouseEventsEnabled WRITE setMouseEventsEnabled NOTIFY mouseEventsEnabledChanged)
    
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
    virtual void setX(qreal x);
    virtual void setY(qreal y);
    virtual void setWidth(qreal w);
    virtual void setHeight(qreal h);
    virtual void setRect(const QRectF &rect);
    
    // Override from Element
    void setParentElementId(const QString &parentId) override;
    
    // Virtual methods to identify element category
    virtual bool isDesignElement() const { return false; }
    virtual bool isScriptElement() const { return false; }
    
    // Override from Element - CanvasElements have visual properties
    bool isCanvasElement() const override { return true; }
    
    // Parent tracking
    virtual void setParentElement(CanvasElement* parent);
    CanvasElement* parentElement() const { return m_parentElement; }
    
    // Property subscription system
    void subscribeToParentProperty(const QString& propertyName);
    void unsubscribeFromParentProperty(const QString& propertyName);
    
    // Mouse events control
    bool mouseEventsEnabled() const { return m_mouseEventsEnabled; }
    void setMouseEventsEnabled(bool enabled);
    
    // Register CanvasElement properties
    void registerProperties() override;
    
signals:
    void xChanged();
    void yChanged();
    void widthChanged();
    void heightChanged();
    void geometryChanged();  // Emitted when x, y, width, or height change
    void parentElementChanged();
    void parentPropertyChanged(const QString& propertyName);  // Generic signal for any parent property change
    void mouseEventsEnabledChanged();
    
protected:
    QPointF canvasPosition;
    QSizeF canvasSize;
    
private slots:
    void onParentPropertyChanged();
    
private:
    // Cached bounding box to avoid repeated construction
    mutable QRectF m_cachedBounds;
    mutable bool m_boundsValid = false;
    
    // Update the cached bounds
    void updateCachedBounds() const;
    
    // Parent element tracking
    CanvasElement* m_parentElement = nullptr;
    QStringList m_subscribedProperties;
    ConnectionManager m_parentConnections;
    
    // Parent position tracking for relative positioning
    QPointF m_lastParentPosition;
    bool m_trackingParentPosition = false;
    
    // Mouse events control
    bool m_mouseEventsEnabled = true;
};