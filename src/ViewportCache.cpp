#include "ViewportCache.h"
#include "Element.h"
#include <QDebug>

ViewportCache::ViewportCache(QObject *parent)
    : QObject(parent)
{
    updateViewportBounds();
}

void ViewportCache::setContentX(qreal x) {
    if (qFuzzyCompare(m_contentX, x)) return;
    m_contentX = x;
    updateViewportBounds();
    emit viewportChanged();
}

void ViewportCache::setContentY(qreal y) {
    if (qFuzzyCompare(m_contentY, y)) return;
    m_contentY = y;
    updateViewportBounds();
    emit viewportChanged();
}

void ViewportCache::setViewportWidth(qreal width) {
    if (qFuzzyCompare(m_viewportWidth, width)) return;
    m_viewportWidth = width;
    updateViewportBounds();
    emit viewportChanged();
}

void ViewportCache::setViewportHeight(qreal height) {
    if (qFuzzyCompare(m_viewportHeight, height)) return;
    m_viewportHeight = height;
    updateViewportBounds();
    emit viewportChanged();
}

void ViewportCache::setZoomLevel(qreal zoom) {
    if (qFuzzyCompare(m_zoomLevel, zoom)) return;
    m_zoomLevel = zoom;
    updateViewportBounds();
    emit viewportChanged();
}

void ViewportCache::updateViewportBounds() {
    m_viewportBounds = QRectF(
        m_contentX / m_zoomLevel + CANVAS_MIN_X,
        m_contentY / m_zoomLevel + CANVAS_MIN_Y,
        m_viewportWidth / m_zoomLevel,
        m_viewportHeight / m_zoomLevel
    );
}

QPointF ViewportCache::viewportToCanvas(qreal viewportX, qreal viewportY) const {
    qreal canvasX = (m_contentX + viewportX) / m_zoomLevel + CANVAS_MIN_X;
    qreal canvasY = (m_contentY + viewportY) / m_zoomLevel + CANVAS_MIN_Y;
    return QPointF(canvasX, canvasY);
}

QPointF ViewportCache::canvasToViewport(qreal canvasX, qreal canvasY) const {
    qreal viewportX = (canvasX - CANVAS_MIN_X) * m_zoomLevel - m_contentX;
    qreal viewportY = (canvasY - CANVAS_MIN_Y) * m_zoomLevel - m_contentY;
    return QPointF(viewportX, viewportY);
}

qreal ViewportCache::canvasXToRelative(qreal canvasX) const {
    return canvasX - CANVAS_MIN_X;
}

qreal ViewportCache::canvasYToRelative(qreal canvasY) const {
    return canvasY - CANVAS_MIN_Y;
}

bool ViewportCache::isElementVisible(const QString &elementId, qreal x, qreal y, qreal width, qreal height) const {
    Q_UNUSED(elementId)
    
    QRectF elementBounds(x, y, width, height);
    QRectF expandedViewport = m_viewportBounds.adjusted(-VISIBILITY_MARGIN, -VISIBILITY_MARGIN, 
                                                        VISIBILITY_MARGIN, VISIBILITY_MARGIN);
    return elementBounds.intersects(expandedViewport);
}

QRectF ViewportCache::getElementClippedBounds(const QString &elementId, qreal x, qreal y, qreal width, qreal height) const {
    Q_UNUSED(elementId)
    
    QRectF elementBounds(x, y, width, height);
    return elementBounds.intersected(m_viewportBounds);
}

void ViewportCache::updateElementVisibility(const QVector<Element*> &elements) {
    m_elementVisibility.clear();
    
    QRectF expandedViewport = m_viewportBounds.adjusted(-VISIBILITY_MARGIN, -VISIBILITY_MARGIN, 
                                                        VISIBILITY_MARGIN, VISIBILITY_MARGIN);
    
    for (Element *element : elements) {
        if (!element) continue;
        
        QRectF elementBounds(element->x(), element->y(), element->width(), element->height());
        bool isVisible = elementBounds.intersects(expandedViewport);
        
        ElementVisibility vis;
        vis.elementId = element->getId();
        vis.isVisible = isVisible;
        vis.clippedBounds = isVisible ? elementBounds.intersected(m_viewportBounds) : QRectF();
        
        m_elementVisibility[element->getId()] = vis;
    }
    
    emit visibilityChanged();
}

bool ViewportCache::getElementVisibility(const QString &elementId) const {
    auto it = m_elementVisibility.find(elementId);
    return it != m_elementVisibility.end() ? it->isVisible : false;
}

QPointF ViewportCache::calculateContentPositionForCenter(qreal canvasX, qreal canvasY) const {
    qreal contentX = (canvasX - CANVAS_MIN_X) * m_zoomLevel - m_viewportWidth / 2;
    qreal contentY = (canvasY - CANVAS_MIN_Y) * m_zoomLevel - m_viewportHeight / 2;
    return QPointF(contentX, contentY);
}