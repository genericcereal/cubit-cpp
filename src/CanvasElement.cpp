#include "CanvasElement.h"
#include "ElementModel.h"
#include "PropertySyncer.h"
#include "PropertyRegistry.h"
#include <QMetaProperty>
#include <QDebug>

CanvasElement::CanvasElement(ElementType type, const QString &id, QObject *parent)
    : Element(type, id, parent), canvasPosition(0, 0), canvasSize(200, 150) // Default size
{
    updateCachedBounds();
}

void CanvasElement::setX(qreal x)
{
    qreal roundedX = qRound(x);
    if (!qFuzzyCompare(canvasPosition.x(), roundedX))
    {
        canvasPosition.setX(roundedX);
        m_boundsValid = false;
        updateCachedBounds();
        emit xChanged();
        emit geometryChanged();
        emit elementChanged();
    }
}

void CanvasElement::setY(qreal y)
{
    qreal roundedY = qRound(y);
    if (!qFuzzyCompare(canvasPosition.y(), roundedY))
    {
        canvasPosition.setY(roundedY);
        m_boundsValid = false;
        updateCachedBounds();
        emit yChanged();
        emit geometryChanged();
        emit elementChanged();
    }
}

void CanvasElement::setWidth(qreal w)
{
    qreal roundedW = qRound(w);
    if (!qFuzzyCompare(canvasSize.width(), roundedW))
    {
        canvasSize.setWidth(roundedW);
        m_boundsValid = false;
        updateCachedBounds();
        emit widthChanged();
        emit geometryChanged();
        emit elementChanged();
    }
}

void CanvasElement::setHeight(qreal h)
{
    qreal roundedH = qRound(h);
    if (!qFuzzyCompare(canvasSize.height(), roundedH))
    {
        canvasSize.setHeight(roundedH);
        m_boundsValid = false;
        updateCachedBounds();
        emit heightChanged();
        emit geometryChanged();
        emit elementChanged();
    }
}

void CanvasElement::setRect(const QRectF &rect)
{
    QPointF roundedPos(qRound(rect.x()), qRound(rect.y()));
    bool posChanged = canvasPosition != roundedPos;
    bool sizeChanged = canvasSize != rect.size();

    if (posChanged || sizeChanged)
    {
        canvasPosition = roundedPos;
        canvasSize = QSizeF(qRound(rect.width()), qRound(rect.height()));
        m_boundsValid = false;
        updateCachedBounds();

        if (posChanged)
        {
            emit xChanged();
            emit yChanged();
        }
        if (sizeChanged)
        {
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
    if (!m_boundsValid)
    {
        m_cachedBounds = QRectF(canvasPosition, canvasSize);
        m_boundsValid = true;
    }
}

void CanvasElement::setParentElement(CanvasElement *parent)
{
    if (m_parentElement == parent)
    {
        return;
    }
    
    
    // Disconnect all existing parent connections
    m_parentConnections.clear();

    m_parentElement = parent;
    
    // Also update the parent ID so ElementModel can maintain proper hierarchy
    if (parent) {
        Element::setParentElementId(parent->getId());
    } else {
        Element::setParentElementId(QString());
    }
    
    emit parentElementChanged();

    // Only subscribe to parent properties for non-design elements
    // (DesignElement has its own parent tracking system)
    if (m_parentElement && !isDesignElement())
    {
        // Store initial parent position for tracking
        m_lastParentPosition = QPointF(m_parentElement->x(), m_parentElement->y());
        m_trackingParentPosition = true;
        
        
        // Define the properties we want to track from parent
        static const QStringList parentPropertiesToTrack = {
            "overflow", // For clipping
            "x",        // For relative positioning
            "y",        // For relative positioning
            "width",    // For bounds checking
            "height"    // For bounds checking
        };

        // Use PropertySyncer to connect all parent property signals
        PropertySyncer::sync(m_parentElement, this, parentPropertiesToTrack,
                           "onParentPropertyChanged()", m_parentConnections);
        
        // Track which properties we're subscribed to
        m_subscribedProperties = parentPropertiesToTrack;
        
    }
    else
    {
        m_trackingParentPosition = false;
    }
}

void CanvasElement::subscribeToParentProperty(const QString &propertyName)
{
    if (!m_parentElement)
    {
        return;
    }

    // Add to subscribed list if not already there
    if (!m_subscribedProperties.contains(propertyName))
    {
        m_subscribedProperties.append(propertyName);
    }

    // Use PropertySyncer for single property subscription
    QStringList singleProperty = { propertyName };
    PropertySyncer::sync(m_parentElement, this, singleProperty,
                        "onParentPropertyChanged()", m_parentConnections);
    
}

void CanvasElement::unsubscribeFromParentProperty(const QString &propertyName)
{
    m_subscribedProperties.removeAll(propertyName);

    // Note: We don't disconnect individual properties here since it's complex
    // to track which connection corresponds to which property.
    // Connections will be cleaned up when parent changes or object is destroyed.
}

void CanvasElement::onParentPropertyChanged()
{
    // Determine which property changed by inspecting the sender's signal
    if (!m_parentElement || !sender())
    {
        return;
    }

    // Get the signal that triggered this slot
    int signalIndex = senderSignalIndex();
    if (signalIndex < 0)
    {
        return;
    }

    // Find which property this signal belongs to
    const QMetaObject *metaObj = m_parentElement->metaObject();
    for (int i = 0; i < metaObj->propertyCount(); ++i)
    {
        QMetaProperty property = metaObj->property(i);
        if (property.hasNotifySignal() &&
            property.notifySignalIndex() == signalIndex)
        {
            // Found the property that changed
            QString propertyName = QString::fromUtf8(property.name());
            
            // Handle parent position changes
            if (m_trackingParentPosition && (propertyName == "x" || propertyName == "y"))
            {
                QPointF currentParentPos(m_parentElement->x(), m_parentElement->y());
                QPointF delta = currentParentPos - m_lastParentPosition;
                
                
                // Move this element by the same delta
                if (!qFuzzyIsNull(delta.x()) || !qFuzzyIsNull(delta.y()))
                {
                    setX(x() + delta.x());
                    setY(y() + delta.y());
                    m_lastParentPosition = currentParentPos;
                }
            }
            
            emit parentPropertyChanged(propertyName);
            break;
        }
    }
}

void CanvasElement::setParentElementId(const QString &parentId)
{
    // Call the base class implementation first
    Element::setParentElementId(parentId);

    // Now resolve the parent element pointer
    if (parentId.isEmpty())
    {
        // No parent, clear the parent element
        setParentElement(nullptr);
    }
    else
    {
        // Find the parent element in the model
        ElementModel *model = nullptr;

        // Try to find the ElementModel through the parent chain
        QObject *p = parent();
        while (p && !model)
        {
            model = qobject_cast<ElementModel *>(p);
            if (!model)
            {
                p = p->parent();
            }
        }

        if (model)
        {
            // Look for the parent element
            Element *parentElement = model->getElementById(parentId);
            if (parentElement && parentElement->isVisual())
            {
                CanvasElement *canvasParent = qobject_cast<CanvasElement *>(parentElement);
                if (canvasParent)
                {
                    setParentElement(canvasParent);
                }
            }
        }
        else
        {
            // ElementModel not found yet - this can happen during loading
        }
    }
}

void CanvasElement::setMouseEventsEnabled(bool enabled)
{
    if (m_mouseEventsEnabled != enabled) {
        m_mouseEventsEnabled = enabled;
        emit mouseEventsEnabledChanged();
    }
}

void CanvasElement::registerProperties() {
    // Call parent implementation first
    Element::registerProperties();
    
    // Register CanvasElement-specific properties
    m_properties->registerProperty("x", 0.0);
    m_properties->registerProperty("y", 0.0);
    m_properties->registerProperty("width", 100.0);
    m_properties->registerProperty("height", 100.0);
    m_properties->registerProperty("mouseEventsEnabled", true);
}