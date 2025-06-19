#include "CanvasElement.h"

CanvasElement::CanvasElement(ElementType type, const QString &id, QObject *parent)
    : Element(type, id, parent)
    , canvasPosition(0, 0)
    , canvasSize(200, 150)  // Default size
{
    updateCachedBounds();
}

void CanvasElement::setX(qreal x)
{
    if (!qFuzzyCompare(canvasPosition.x(), x)) {
        canvasPosition.setX(x);
        m_boundsValid = false;
        updateCachedBounds();
        emit xChanged();
        emit geometryChanged();
        emit elementChanged();
    }
}

void CanvasElement::setY(qreal y)
{
    if (!qFuzzyCompare(canvasPosition.y(), y)) {
        canvasPosition.setY(y);
        m_boundsValid = false;
        updateCachedBounds();
        emit yChanged();
        emit geometryChanged();
        emit elementChanged();
    }
}

void CanvasElement::setWidth(qreal w)
{
    if (!qFuzzyCompare(canvasSize.width(), w)) {
        canvasSize.setWidth(w);
        m_boundsValid = false;
        updateCachedBounds();
        emit widthChanged();
        emit geometryChanged();
        emit elementChanged();
    }
}

void CanvasElement::setHeight(qreal h)
{
    if (!qFuzzyCompare(canvasSize.height(), h)) {
        canvasSize.setHeight(h);
        m_boundsValid = false;
        updateCachedBounds();
        emit heightChanged();
        emit geometryChanged();
        emit elementChanged();
    }
}

void CanvasElement::setRect(const QRectF &rect)
{
    bool posChanged = canvasPosition != rect.topLeft();
    bool sizeChanged = canvasSize != rect.size();
    
    if (posChanged || sizeChanged) {
        canvasPosition = rect.topLeft();
        canvasSize = rect.size();
        m_boundsValid = false;
        updateCachedBounds();
        
        if (posChanged) {
            emit xChanged();
            emit yChanged();
        }
        if (sizeChanged) {
            emit widthChanged();
            emit heightChanged();
        }
        emit geometryChanged();
        emit elementChanged();
    }
}

bool CanvasElement::containsPoint(const QPointF &point) const
{
    // Default implementation uses bounding box
    return rect().contains(point);
}

void CanvasElement::updateCachedBounds() const
{
    if (!m_boundsValid) {
        m_cachedBounds = QRectF(canvasPosition, canvasSize);
        m_boundsValid = true;
    }
}