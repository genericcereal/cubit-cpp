#include "CanvasElement.h"
#include "ElementModel.h"
#include <QMetaProperty>
#include <QDebug>

CanvasElement::CanvasElement(ElementType type, const QString &id, QObject *parent)
    : Element(type, id, parent), canvasPosition(0, 0), canvasSize(200, 150) // Default size
{
    updateCachedBounds();
}

void CanvasElement::setX(qreal x)
{
    if (!qFuzzyCompare(canvasPosition.x(), x))
    {
        qreal oldX = canvasPosition.x();
        canvasPosition.setX(x);
        m_boundsValid = false;
        updateCachedBounds();
        qDebug() << "CanvasElement::setX -" << this->getTypeName() << getId() << "moved from x:" << oldX << "to x:" << x;
        emit xChanged();
        emit geometryChanged();
        emit elementChanged();
    }
}

void CanvasElement::setY(qreal y)
{
    if (!qFuzzyCompare(canvasPosition.y(), y))
    {
        qreal oldY = canvasPosition.y();
        canvasPosition.setY(y);
        m_boundsValid = false;
        updateCachedBounds();
        qDebug() << "CanvasElement::setY -" << this->getTypeName() << getId() << "moved from y:" << oldY << "to y:" << y;
        emit yChanged();
        emit geometryChanged();
        emit elementChanged();
    }
}

void CanvasElement::setWidth(qreal w)
{
    if (!qFuzzyCompare(canvasSize.width(), w))
    {
        qreal oldWidth = canvasSize.width();
        canvasSize.setWidth(w);
        m_boundsValid = false;
        updateCachedBounds();
        qDebug() << "CanvasElement::setWidth -" << this->getTypeName() << getId() << "resized from width:" << oldWidth << "to width:" << w;
        emit widthChanged();
        emit geometryChanged();
        emit elementChanged();
    }
}

void CanvasElement::setHeight(qreal h)
{
    if (!qFuzzyCompare(canvasSize.height(), h))
    {
        qreal oldHeight = canvasSize.height();
        canvasSize.setHeight(h);
        m_boundsValid = false;
        updateCachedBounds();
        qDebug() << "CanvasElement::setHeight -" << this->getTypeName() << getId() << "resized from height:" << oldHeight << "to height:" << h;
        emit heightChanged();
        emit geometryChanged();
        emit elementChanged();
    }
}

void CanvasElement::setRect(const QRectF &rect)
{
    bool posChanged = canvasPosition != rect.topLeft();
    bool sizeChanged = canvasSize != rect.size();

    if (posChanged || sizeChanged)
    {
        canvasPosition = rect.topLeft();
        canvasSize = rect.size();
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
    
    qDebug() << "CanvasElement::setParentElement -" << this->getTypeName() << getId() 
             << "setting parent to" << (parent ? parent->getTypeName() + " " + parent->getId() : "null");
    
    // Disconnect all existing parent connections
    for (const auto &connection : m_parentConnections)
    {
        disconnect(connection);
    }
    m_parentConnections.clear();

    m_parentElement = parent;
    emit parentElementChanged();

    // Only subscribe to parent properties for non-design elements
    // (DesignElement has its own parent tracking system)
    if (m_parentElement && !isDesignElement())
    {
        // Store initial parent position for tracking
        m_lastParentPosition = QPointF(m_parentElement->x(), m_parentElement->y());
        m_trackingParentPosition = true;
        
        qDebug() << "  -> Element is NOT a design element. Tracking parent position enabled.";
        qDebug() << "  -> Initial parent position:" << m_lastParentPosition;
        
        // Define the properties we want to track from parent
        static const QStringList parentPropertiesToTrack = {
            "overflow", // For clipping
            "x",        // For relative positioning
            "y",        // For relative positioning
            "width",    // For bounds checking
            "height"    // For bounds checking
        };

        // Subscribe to each property
        for (const QString &propertyName : parentPropertiesToTrack)
        {
            subscribeToParentProperty(propertyName);
        }
        
        qDebug() << "  -> Subscribed to parent properties:" << parentPropertiesToTrack;
    }
    else
    {
        m_trackingParentPosition = false;
        if (m_parentElement && isDesignElement()) {
            qDebug() << "  -> Element is a design element. Using DesignElement's parent tracking.";
        }
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

    // Find the property's notify signal
    const QMetaObject *metaObj = m_parentElement->metaObject();
    int propertyIndex = metaObj->indexOfProperty(propertyName.toUtf8().constData());

    if (propertyIndex >= 0)
    {
        QMetaProperty property = metaObj->property(propertyIndex);
        if (property.hasNotifySignal())
        {
            QMetaMethod notifySignal = property.notifySignal();
            QMetaMethod targetSlot = metaObject()->method(
                metaObject()->indexOfSlot("onParentPropertyChanged()"));

            // Connect the parent's property change signal to our slot
            QMetaObject::Connection connection = connect(
                m_parentElement, notifySignal,
                this, targetSlot,
                Qt::UniqueConnection);

            if (connection)
            {
                m_parentConnections.append(connection);
                qDebug() << "    -> Successfully connected to parent's" << propertyName << "property change signal";
            }
            else
            {
                qDebug() << "    -> FAILED to connect to parent's" << propertyName << "property change signal";
            }
        }
    }
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
                
                qDebug() << "CanvasElement::onParentPropertyChanged -" << this->getTypeName() << getId() 
                         << "detected parent" << m_parentElement->getTypeName() << m_parentElement->getId()
                         << "moved. Delta x:" << delta.x() << "y:" << delta.y();
                
                // Move this element by the same delta
                if (!qFuzzyIsNull(delta.x()) || !qFuzzyIsNull(delta.y()))
                {
                    qDebug() << "  -> Moving child from (" << x() << "," << y() << ") to (" 
                             << (x() + delta.x()) << "," << (y() + delta.y()) << ")";
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
    }
}

void CanvasElement::setMouseEventsEnabled(bool enabled)
{
    if (m_mouseEventsEnabled != enabled) {
        m_mouseEventsEnabled = enabled;
        emit mouseEventsEnabledChanged();
    }
}