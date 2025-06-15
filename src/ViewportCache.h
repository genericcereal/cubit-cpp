#pragma once
#include <QObject>
#include <QRectF>
#include <QPointF>
#include <QHash>
#include <QVector>

class Element;

struct ViewportBounds {
    qreal left;
    qreal top;
    qreal right;
    qreal bottom;
    qreal width;
    qreal height;
};

struct ElementVisibility {
    QString elementId;
    bool isVisible;
    QRectF clippedBounds;  // The visible portion in canvas coordinates
};

class ViewportCache : public QObject {
    Q_OBJECT
    Q_PROPERTY(qreal contentX READ contentX WRITE setContentX NOTIFY viewportChanged)
    Q_PROPERTY(qreal contentY READ contentY WRITE setContentY NOTIFY viewportChanged)
    Q_PROPERTY(qreal viewportWidth READ viewportWidth WRITE setViewportWidth NOTIFY viewportChanged)
    Q_PROPERTY(qreal viewportHeight READ viewportHeight WRITE setViewportHeight NOTIFY viewportChanged)
    Q_PROPERTY(qreal zoomLevel READ zoomLevel WRITE setZoomLevel NOTIFY viewportChanged)
    
public:
    explicit ViewportCache(QObject *parent = nullptr);
    
    // Canvas constants
    static constexpr qreal CANVAS_MIN_X = -10000;
    static constexpr qreal CANVAS_MIN_Y = -10000;
    static constexpr qreal CANVAS_MAX_X = 10000;
    static constexpr qreal CANVAS_MAX_Y = 10000;
    static constexpr qreal CANVAS_WIDTH = CANVAS_MAX_X - CANVAS_MIN_X;
    static constexpr qreal CANVAS_HEIGHT = CANVAS_MAX_Y - CANVAS_MIN_Y;
    
    // Property accessors
    qreal contentX() const { return m_contentX; }
    qreal contentY() const { return m_contentY; }
    qreal viewportWidth() const { return m_viewportWidth; }
    qreal viewportHeight() const { return m_viewportHeight; }
    qreal zoomLevel() const { return m_zoomLevel; }
    
    void setContentX(qreal x);
    void setContentY(qreal y);
    void setViewportWidth(qreal width);
    void setViewportHeight(qreal height);
    void setZoomLevel(qreal zoom);
    
    // Cached viewport bounds
    Q_INVOKABLE QRectF viewportBounds() const { return m_viewportBounds; }
    
    // Coordinate transformations (these are still computed but optimized)
    Q_INVOKABLE QPointF viewportToCanvas(qreal viewportX, qreal viewportY) const;
    Q_INVOKABLE QPointF canvasToViewport(qreal canvasX, qreal canvasY) const;
    Q_INVOKABLE qreal canvasXToRelative(qreal canvasX) const;
    Q_INVOKABLE qreal canvasYToRelative(qreal canvasY) const;
    
    // Element visibility checking
    Q_INVOKABLE bool isElementVisible(const QString &elementId, qreal x, qreal y, qreal width, qreal height) const;
    Q_INVOKABLE QRectF getElementClippedBounds(const QString &elementId, qreal x, qreal y, qreal width, qreal height) const;
    
    // Batch visibility update for all elements
    void updateElementVisibility(const QVector<Element*> &elements);
    Q_INVOKABLE bool getElementVisibility(const QString &elementId) const;
    
    // Content position calculation for centering
    Q_INVOKABLE QPointF calculateContentPositionForCenter(qreal canvasX, qreal canvasY) const;
    
signals:
    void viewportChanged();
    void visibilityChanged();
    
private:
    void updateViewportBounds();
    
    qreal m_contentX = 0;
    qreal m_contentY = 0;
    qreal m_viewportWidth = 0;
    qreal m_viewportHeight = 0;
    qreal m_zoomLevel = 1.0;
    
    QRectF m_viewportBounds;
    QHash<QString, ElementVisibility> m_elementVisibility;
    
    static constexpr qreal VISIBILITY_MARGIN = 100.0;
};