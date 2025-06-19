#include "CanvasElement.h"
#include <QMetaProperty>
#include <QDebug>

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

void CanvasElement::setParentElement(CanvasElement* parent)
{
    if (m_parentElement == parent) {
        return;
    }
    
    // Disconnect all existing parent connections
    for (const auto& connection : m_parentConnections) {
        disconnect(connection);
    }
    m_parentConnections.clear();
    
    m_parentElement = parent;
    
    // Only subscribe to parent properties for design elements
    if (m_parentElement && isDesignElement()) {
        // Define the properties we want to track from parent
        static const QStringList parentPropertiesToTrack = {
            "overflow",  // For clipping
            "x",         // For relative positioning
            "y",         // For relative positioning
            "width",     // For bounds checking
            "height"     // For bounds checking
        };
        
        // Subscribe to each property
        for (const QString& propertyName : parentPropertiesToTrack) {
            subscribeToParentProperty(propertyName);
        }
    }
}

void CanvasElement::subscribeToParentProperty(const QString& propertyName)
{
    if (!m_parentElement) {
        return;
    }
    
    // Add to subscribed list if not already there
    if (!m_subscribedProperties.contains(propertyName)) {
        m_subscribedProperties.append(propertyName);
    }
    
    // Find the property's notify signal
    const QMetaObject* metaObj = m_parentElement->metaObject();
    int propertyIndex = metaObj->indexOfProperty(propertyName.toUtf8().constData());
    
    if (propertyIndex >= 0) {
        QMetaProperty property = metaObj->property(propertyIndex);
        if (property.hasNotifySignal()) {
            QMetaMethod notifySignal = property.notifySignal();
            QMetaMethod targetSlot = metaObject()->method(
                metaObject()->indexOfSlot("onParentPropertyChanged()")
            );
            
            // Connect the parent's property change signal to our slot
            QMetaObject::Connection connection = connect(
                m_parentElement, notifySignal,
                this, targetSlot,
                Qt::UniqueConnection
            );
            
            if (connection) {
                m_parentConnections.append(connection);
            }
        }
    }
}

void CanvasElement::unsubscribeFromParentProperty(const QString& propertyName)
{
    m_subscribedProperties.removeAll(propertyName);
    
    // Note: We don't disconnect individual properties here since it's complex
    // to track which connection corresponds to which property.
    // Connections will be cleaned up when parent changes or object is destroyed.
}

void CanvasElement::onParentPropertyChanged()
{
    // Determine which property changed by inspecting the sender's signal
    if (!m_parentElement || !sender()) {
        return;
    }
    
    // Get the signal that triggered this slot
    int signalIndex = senderSignalIndex();
    if (signalIndex < 0) {
        return;
    }
    
    // Find which property this signal belongs to
    const QMetaObject* metaObj = m_parentElement->metaObject();
    for (int i = 0; i < metaObj->propertyCount(); ++i) {
        QMetaProperty property = metaObj->property(i);
        if (property.hasNotifySignal() && 
            property.notifySignalIndex() == signalIndex) {
            // Found the property that changed
            emit parentPropertyChanged(QString::fromUtf8(property.name()));
            break;
        }
    }
}

QRectF CanvasElement::clipBounds() const
{
    // If no parent, return infinite bounds (no clipping)
    if (!m_parentElement) {
        return QRectF(-1e9, -1e9, 2e9, 2e9);
    }
    
    // Check if parent has overflow property (only Frame elements have this)
    QVariant overflowVar = m_parentElement->property("overflow");
    if (!overflowVar.isValid()) {
        // Parent doesn't have overflow property, no clipping
        return QRectF(-1e9, -1e9, 2e9, 2e9);
    }
    
    // Check overflow mode
    int overflowMode = overflowVar.toInt();
    // Frame::OverflowMode::Hidden = 0, Scroll = 1, Visible = 2
    if (overflowMode == 2) { // Visible
        // No clipping for overflow: visible
        return QRectF(-1e9, -1e9, 2e9, 2e9);
    }
    
    // For Hidden or Scroll modes, clip to parent bounds
    // Return parent's bounds in parent's coordinate space
    // Child elements will need to transform this to their own coordinate space
    return m_parentElement->rect();
}