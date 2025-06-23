#include "ComponentInstance.h"
#include "Component.h"
#include "ComponentVariant.h"
#include "Frame.h"
#include "Text.h"
#include "Html.h"
#include "DesignElement.h"
#include "Application.h"
#include "ElementModel.h"
#include "UniqueIdGenerator.h"
#include <QCoreApplication>
#include <QDebug>

ComponentInstance::ComponentInstance(const QString &id, QObject *parent)
    : Frame(id, parent)
{
    // Set the element type directly (it's protected, not private)
    elementType = ComponentInstanceType;
    
    // Set the name to "Instance" + last 4 digits of ID
    QString last4 = id.right(4);
    setName("Instance" + last4);
    
    // Set overflow to Hidden by default to test clipping
    setOverflow(Hidden);
    qDebug() << "ComponentInstance constructor - set overflow to Hidden for" << id;
}

ComponentInstance::~ComponentInstance()
{
    // Set flag to indicate we're destructing
    m_isDestructing = true;
    
    // During destruction, we don't need to clean up child instances
    // Qt's parent-child relationship will handle the deletion
    
    // Just clear our tracking structures
    m_childInstances.clear();
    
    // Clear connections
    for (auto& connections : m_childConnections.values()) {
        for (const auto& connection : connections) {
            disconnect(connection);
        }
    }
    m_childConnections.clear();
    
    disconnectFromVariant();
    disconnectFromComponent();
}

void ComponentInstance::setInstanceOf(const QString &componentId)
{
    if (m_instanceOf != componentId) {
        m_instanceOf = componentId;
        
        // Disconnect from previous component/variant
        disconnectFromVariant();
        disconnectFromComponent();
        
        // Defer connection to avoid issues during initialization
        // Connect to new component on the next event loop iteration
        QMetaObject::invokeMethod(this, &ComponentInstance::connectToComponent, Qt::QueuedConnection);
        
        emit instanceOfChanged();
    }
}

void ComponentInstance::connectToComponent()
{
    if (m_instanceOf.isEmpty()) {
        return;
    }
    
    // Find the component in the element model
    Application* app = Application::instance();
    if (!app || !app->activeCanvas() || !app->activeCanvas()->elementModel()) {
        return;
    }
    
    ElementModel* elementModel = app->activeCanvas()->elementModel();
    Element* element = elementModel->getElementById(m_instanceOf);
    
    m_component = qobject_cast<Component*>(element);
    if (!m_component) {
        qWarning() << "ComponentInstance: Could not find Component with ID" << m_instanceOf;
        return;
    }
    
    // Connect to component's variants changed signal
    auto connection = connect(m_component, &Component::variantsChanged,
                            this, &ComponentInstance::onComponentVariantsChanged);
    m_componentConnections.append(connection);
    
    // Connect to the first variant if available
    connectToVariant();
}

void ComponentInstance::disconnectFromComponent()
{
    for (const auto& connection : m_componentConnections) {
        disconnect(connection);
    }
    m_componentConnections.clear();
    m_component = nullptr;
}

void ComponentInstance::connectToVariant()
{
    if (!m_component) {
        return;
    }
    
    QList<ComponentVariant*> variants = m_component->variants();
    if (variants.isEmpty()) {
        return;
    }
    
    // Connect to the first variant
    m_sourceVariant = variants.first();
    
    // Sync initial properties
    syncPropertiesFromVariant();
    
    // Create instances of all children
    createChildInstances();
    
    // Connect to all property change signals from the variant
    const QMetaObject* metaObj = m_sourceVariant->metaObject();
    
    // List of properties to sync (excluding position)
    static const QStringList propertiesToSync = {
        "fillColor",
        "borderColor", 
        "borderWidth",
        "borderRadius",
        "overflow",
        "width",
        "height"
    };
    
    for (const QString& propertyName : propertiesToSync) {
        int propertyIndex = metaObj->indexOfProperty(propertyName.toUtf8().constData());
        if (propertyIndex >= 0) {
            QMetaProperty property = metaObj->property(propertyIndex);
            if (property.hasNotifySignal()) {
                QMetaMethod notifySignal = property.notifySignal();
                QMetaMethod slot = metaObject()->method(
                    metaObject()->indexOfSlot("onSourceVariantPropertyChanged()"));
                
                auto connection = connect(m_sourceVariant, notifySignal,
                                        this, slot, Qt::UniqueConnection);
                if (connection) {
                    m_variantConnections.append(connection);
                }
            }
        }
    }
    
    emit sourceVariantChanged();
}

void ComponentInstance::disconnectFromVariant()
{
    // Clear child instances only if we're not in the destructor
    // During destruction, Qt's parent-child system handles cleanup automatically
    if (!m_isDestructing) {
        clearChildInstances();
    }
    
    for (const auto& connection : m_variantConnections) {
        disconnect(connection);
    }
    m_variantConnections.clear();
    m_sourceVariant = nullptr;
    emit sourceVariantChanged();
}

void ComponentInstance::syncPropertiesFromVariant()
{
    if (!m_sourceVariant) {
        return;
    }
    
    // Sync size properties
    setWidth(m_sourceVariant->width());
    setHeight(m_sourceVariant->height());
    
    // Sync visual properties from the ComponentVariant (which is a Frame)
    if (ComponentVariant* variant = qobject_cast<ComponentVariant*>(m_sourceVariant)) {
        setFillColor(variant->fillColor());
        setBorderColor(variant->borderColor());
        setBorderWidth(variant->borderWidth());
        setBorderRadius(variant->borderRadius());
        
        // Debug overflow sync
        OverflowMode variantOverflow = variant->overflow();
        qDebug() << "ComponentInstance::syncPropertiesFromVariant - variant overflow:" << variantOverflow;
        setOverflow(variantOverflow);
        qDebug() << "ComponentInstance::syncPropertiesFromVariant - instance" << getId() << "overflow after set:" << overflow();
    }
    
    // Note: We don't sync position (x, y) as instances maintain their own position
}

void ComponentInstance::onSourceVariantPropertyChanged()
{
    // Re-sync properties when the source variant changes
    syncPropertiesFromVariant();
}

void ComponentInstance::onComponentVariantsChanged()
{
    // Re-connect to the first variant when the component's variants change
    disconnectFromVariant();
    connectToVariant();
}

void ComponentInstance::createChildInstances()
{
    if (!m_sourceVariant) {
        return;
    }
    
    Application* app = Application::instance();
    if (!app || !app->activeCanvas() || !app->activeCanvas()->elementModel()) {
        return;
    }
    
    ElementModel* elementModel = app->activeCanvas()->elementModel();
    
    // Get all children of the source variant (recursively)
    QList<Element*> variantChildren = elementModel->getChildrenRecursive(m_sourceVariant->getId());
    
    // Create instances for each child
    for (Element* child : variantChildren) {
        CanvasElement* canvasChild = qobject_cast<CanvasElement*>(child);
        if (canvasChild && canvasChild->isVisual()) {
            // Find the parent for this instance
            CanvasElement* instanceParent = this;
            
            // If the child has a parent in the variant hierarchy, find its corresponding instance
            if (!canvasChild->getParentElementId().isEmpty() && canvasChild->getParentElementId() != m_sourceVariant->getId()) {
                if (m_childInstances.contains(canvasChild->getParentElementId())) {
                    instanceParent = m_childInstances[canvasChild->getParentElementId()];
                }
            }
            
            // Create instance of this child
            CanvasElement* childInstance = createInstanceOfElement(canvasChild, instanceParent);
            if (childInstance) {
                m_childInstances[canvasChild->getId()] = childInstance;
                
                // Debug parent-child relationship
                qDebug() << "ComponentInstance::createChildInstances - Created child" << childInstance->getId()
                         << "type:" << childInstance->getTypeName()
                         << "parent:" << (instanceParent ? instanceParent->getId() : "null")
                         << "parentId:" << childInstance->getParentElementId();
                
                // Add to element model
                elementModel->addElement(childInstance);
                
                // Connect to track changes
                connectToSourceElement(childInstance, canvasChild);
            }
        }
    }
}

void ComponentInstance::clearChildInstances()
{
    // Clear connections first
    for (auto& connections : m_childConnections.values()) {
        for (const auto& connection : connections) {
            disconnect(connection);
        }
    }
    m_childConnections.clear();
    
    // During shutdown, Qt's parent-child system will handle cleanup
    // We don't need to manually remove elements from the model
    // Just clear our tracking structures
    m_childInstances.clear();
}

void ComponentInstance::updateChildInstancesForResize()
{
    if (!m_sourceVariant) {
        return;
    }
    
    // Calculate scale factors based on the instance size vs variant size
    qreal scaleX = width() / m_sourceVariant->width();
    qreal scaleY = height() / m_sourceVariant->height();
    
    // Get the element model to find source elements
    Application* app = Application::instance();
    if (!app || !app->activeCanvas() || !app->activeCanvas()->elementModel()) {
        return;
    }
    
    ElementModel* elementModel = app->activeCanvas()->elementModel();
    
    // Only update direct children of the ComponentInstance (first level children of the variant)
    for (auto it = m_childInstances.begin(); it != m_childInstances.end(); ++it) {
        const QString& sourceId = it.key();
        CanvasElement* instanceElement = it.value();
        
        // Find the source element
        Element* sourceElement = elementModel->getElementById(sourceId);
        CanvasElement* canvasSource = qobject_cast<CanvasElement*>(sourceElement);
        
        if (instanceElement && canvasSource) {
            // Only update if this is a direct child of the variant
            if (canvasSource->getParentElementId() == m_sourceVariant->getId()) {
                // Calculate the source element's position relative to the variant
                qreal sourceRelX = canvasSource->x() - m_sourceVariant->x();
                qreal sourceRelY = canvasSource->y() - m_sourceVariant->y();
                
                // Apply scaling to position and size
                instanceElement->setX(x() + sourceRelX * scaleX);
                instanceElement->setY(y() + sourceRelY * scaleY);
                instanceElement->setWidth(canvasSource->width() * scaleX);
                instanceElement->setHeight(canvasSource->height() * scaleY);
                
                qDebug() << "ComponentInstance::updateChildInstancesForResize - Updated direct child" 
                         << instanceElement->getId() << "to" << instanceElement->width() << "x" << instanceElement->height();
            }
        }
    }
}

CanvasElement* ComponentInstance::createInstanceOfElement(CanvasElement* sourceElement, CanvasElement* parent)
{
    if (!sourceElement) {
        return nullptr;
    }
    
    QString instanceId = UniqueIdGenerator::generate16DigitId();
    CanvasElement* instance = nullptr;
    
    // Create appropriate type of instance
    if (Frame* frame = qobject_cast<Frame*>(sourceElement)) {
        Frame* frameInstance = new Frame(instanceId, parent);
        frameInstance->setName("Instance of " + frame->getName());
        instance = frameInstance;
    } else if (Text* text = qobject_cast<Text*>(sourceElement)) {
        Text* textInstance = new Text(instanceId, parent);
        textInstance->setName("Instance of " + text->getName());
        instance = textInstance;
    } else if (Html* html = qobject_cast<Html*>(sourceElement)) {
        Html* htmlInstance = new Html(instanceId, parent);
        htmlInstance->setName("Instance of " + html->getName());
        instance = htmlInstance;
    }
    
    if (instance) {
        // Set parent relationship
        instance->setParentElement(parent);
        instance->setParentElementId(parent->getId());
        
        // Disable mouse events for child instances
        instance->setMouseEventsEnabled(false);
        
        // Hide from element list
        instance->setShowInElementList(false);
        
        // Sync initial properties
        syncElementProperties(instance, sourceElement);
    }
    
    return instance;
}

void ComponentInstance::syncElementProperties(CanvasElement* target, CanvasElement* source)
{
    if (!target || !source) {
        return;
    }
    
    // Calculate relative position to the variant
    qreal relX = source->x();
    qreal relY = source->y();
    
    // If source has a parent, calculate relative position
    if (source->parentElement()) {
        relX = source->x() - source->parentElement()->x();
        relY = source->y() - source->parentElement()->y();
    }
    
    // Apply relative position to instance
    if (target->parentElement()) {
        target->setX(target->parentElement()->x() + relX);
        target->setY(target->parentElement()->y() + relY);
    } else {
        target->setX(x() + relX);
        target->setY(y() + relY);
    }
    
    // Use utility function to copy properties (including size)
    DesignElement::copyElementProperties(target, source, true);
}

void ComponentInstance::connectToSourceElement(CanvasElement* instanceElement, CanvasElement* sourceElement)
{
    if (!instanceElement || !sourceElement) {
        return;
    }
    
    QList<QMetaObject::Connection> connections;
    
    // Connect to property changes
    const QMetaObject* metaObj = sourceElement->metaObject();
    
    // List of properties to track
    static const QStringList propertiesToTrack = {
        "x", "y", "width", "height",
        "left", "right", "top", "bottom",
        "leftAnchored", "rightAnchored", "topAnchored", "bottomAnchored",
        "fillColor", "borderColor", "borderWidth", "borderRadius", "overflow",
        "text", "font", "color",
        "html", "url"
    };
    
    for (const QString& propertyName : propertiesToTrack) {
        int propertyIndex = metaObj->indexOfProperty(propertyName.toUtf8().constData());
        if (propertyIndex >= 0) {
            QMetaProperty property = metaObj->property(propertyIndex);
            if (property.hasNotifySignal()) {
                QMetaMethod notifySignal = property.notifySignal();
                QMetaMethod slot = metaObject()->method(
                    metaObject()->indexOfSlot("onInstanceChildPropertyChanged()"));
                
                auto connection = connect(sourceElement, notifySignal,
                                        this, slot, Qt::UniqueConnection);
                if (connection) {
                    connections.append(connection);
                }
            }
        }
    }
    
    m_childConnections[instanceElement] = connections;
}

void ComponentInstance::onVariantChildAdded()
{
    // TODO: Handle dynamic addition of children to the variant
}

void ComponentInstance::onVariantChildRemoved()
{
    // TODO: Handle dynamic removal of children from the variant
}

void ComponentInstance::onInstanceChildPropertyChanged()
{
    // Find which source element triggered this
    CanvasElement* sourceElement = qobject_cast<CanvasElement*>(sender());
    if (!sourceElement) {
        return;
    }
    
    // Find the corresponding instance
    CanvasElement* instanceElement = m_childInstances.value(sourceElement->getId());
    if (instanceElement) {
        // Re-sync properties
        syncElementProperties(instanceElement, sourceElement);
    }
}

void ComponentInstance::setWidth(qreal width)
{
    qreal oldWidth = this->width();
    
    // Call base class implementation
    CanvasElement::setWidth(width);
    
    // Update child instances to scale proportionally
    if (!qFuzzyCompare(oldWidth, width) && m_sourceVariant && oldWidth > 0) {
        updateChildInstancesForResize();
    }
}

void ComponentInstance::setHeight(qreal height)
{
    qreal oldHeight = this->height();
    
    // Call base class implementation
    CanvasElement::setHeight(height);
    
    // Update child instances to scale proportionally
    if (!qFuzzyCompare(oldHeight, height) && m_sourceVariant && oldHeight > 0) {
        updateChildInstancesForResize();
    }
}

void ComponentInstance::setRect(const QRectF &rect)
{
    QRectF oldRect(x(), y(), width(), height());
    
    // Call base class implementation
    CanvasElement::setRect(rect);
    
    // Update child instances if size changed
    if (!qFuzzyCompare(oldRect.width(), rect.width()) || 
        !qFuzzyCompare(oldRect.height(), rect.height())) {
        updateChildInstancesForResize();
    }
}