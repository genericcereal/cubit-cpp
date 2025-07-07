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
#include "PropertySyncer.h"
#include "PropertyCopier.h"
#include <QCoreApplication>
#include <QDebug>

// Define static property lists
const QStringList ComponentInstance::s_variantPropertiesToSync = {
    "width",
    "height",
    "fill",
    "borderColor",
    "borderWidth",
    "borderRadius",
    "overflow"
};

const QStringList ComponentInstance::s_childPropertiesToTrack = {
    "x", "y", "width", "height",
    "left", "right", "top", "bottom",
    "leftAnchored", "rightAnchored", "topAnchored", "bottomAnchored",
    "fill", "borderColor", "borderWidth", "borderRadius", "overflow",
    "content", "font", "color",
    "html", "url"
};

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
    m_componentConnections.add(connect(m_component, &Component::variantsChanged,
                            this, &ComponentInstance::onComponentVariantsChanged));
    
    // Connect to the first variant if available
    connectToVariant();
}

void ComponentInstance::disconnectFromComponent()
{
    m_componentConnections.clear();
    m_component = nullptr;
}

void ComponentInstance::connectToVariant()
{
    if (!m_component) {
        return;
    }
    
    QList<Element*> variants = m_component->variants();
    if (variants.isEmpty()) {
        return;
    }
    
    // Connect to the first variant that is a CanvasElement (usually a ComponentVariant)
    for (Element* variant : variants) {
        if (qobject_cast<CanvasElement*>(variant)) {
            m_sourceVariant = variant;
            break;
        }
    }
    
    if (!m_sourceVariant) {
        return;
    }
    
    // Sync initial properties
    syncPropertiesFromVariant();
    
    // Create instances of all children
    createChildInstances();
    
    // Connect to all property change signals from the variant
    PropertySyncer::sync(m_sourceVariant, this, s_variantPropertiesToSync, 
                        "onSourceVariantPropertyChanged()", m_variantConnections);
    
    // Connect to instancesAcceptChildrenChanged signal if it's a ComponentVariant
    if (ComponentVariant* variant = qobject_cast<ComponentVariant*>(m_sourceVariant)) {
        m_variantConnections.add(
            connect(variant, &ComponentVariant::instancesAcceptChildrenChanged,
                    this, &ComponentInstance::onSourceVariantPropertyChanged)
        );
    }
    
    // Connect to ElementModel to track parent changes
    Application* app = Application::instance();
    if (app && app->activeCanvas() && app->activeCanvas()->elementModel()) {
        ElementModel* elementModel = app->activeCanvas()->elementModel();
        
        // Connect to track when new elements are added that might be children
        m_variantConnections.add(
            connect(elementModel, &ElementModel::elementAdded,
                    this, &ComponentInstance::onElementAdded)
        );
        
        // Connect to existing elements to track parent changes
        for (Element* element : elementModel->getAllElements()) {
            if (element && element != m_sourceVariant) {
                m_variantConnections.add(
                    connect(element, &Element::parentIdChanged,
                            this, &ComponentInstance::onElementParentChanged)
                );
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
    
    m_variantConnections.clear();
    m_sourceVariant = nullptr;
    emit sourceVariantChanged();
}

void ComponentInstance::syncPropertiesFromVariant()
{
    if (!m_sourceVariant) {
        return;
    }
    
    // Use PropertyCopier to sync properties
    PropertyCopier::copyProperties(m_sourceVariant, this, s_variantPropertiesToSync);
    
    // Debug overflow sync
    qDebug() << "ComponentInstance::syncPropertiesFromVariant - instance" << getId() 
             << "overflow after sync:" << overflow();
    
    // Sync acceptsChildren based on variant's instancesAcceptChildren property
    if (ComponentVariant* variant = qobject_cast<ComponentVariant*>(m_sourceVariant)) {
        setAcceptsChildren(variant->instancesAcceptChildren());
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
    m_childConnections.clear();
    
    // During shutdown, Qt's parent-child system will handle cleanup
    // We don't need to manually remove elements from the model
    // Just clear our tracking structures
    m_childInstances.clear();
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
        
        // Enable mouse events for child instances so they can be hovered
        instance->setMouseEventsEnabled(true);
        
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
    
    // Connect to property changes
    PropertySyncer::sync(sourceElement, this, s_childPropertiesToTrack,
                        "onInstanceChildPropertyChanged()", m_childConnections[instanceElement]);
}

void ComponentInstance::onVariantChildAdded(Element* child)
{
    if (!child || !m_sourceVariant) {
        return;
    }
    
    // Check if we already have an instance for this child
    if (m_childInstances.contains(child->getId())) {
        return;
    }
    
    CanvasElement* canvasChild = qobject_cast<CanvasElement*>(child);
    if (!canvasChild) {
        return;
    }
    
    qDebug() << "ComponentInstance::onVariantChildAdded - Adding child" << child->getId() 
             << "to instance" << getId();
    
    // Create an instance of the child element
    CanvasElement* childInstance = createInstanceOfElement(canvasChild, this);
    if (!childInstance) {
        qWarning() << "ComponentInstance::onVariantChildAdded - Failed to create instance for" << child->getId();
        return;
    }
    
    // Store the mapping
    m_childInstances[child->getId()] = childInstance;
    
    // Connect to property changes on the source child
    connectToSourceElement(childInstance, canvasChild);
    
    // Add to element model
    Application* app = Application::instance();
    if (app && app->activeCanvas() && app->activeCanvas()->elementModel()) {
        app->activeCanvas()->elementModel()->addElement(childInstance);
    }
    
    // Handle nested children recursively
    if (app && app->activeCanvas() && app->activeCanvas()->elementModel()) {
        ElementModel* elementModel = app->activeCanvas()->elementModel();
        QList<Element*> nestedChildren = elementModel->getChildrenRecursive(child->getId());
        
        for (Element* nestedChild : nestedChildren) {
            if (CanvasElement* nestedCanvas = qobject_cast<CanvasElement*>(nestedChild)) {
                CanvasElement* parentInstance = m_childInstances.value(nestedChild->getParentElementId());
                if (parentInstance) {
                    CanvasElement* nestedInstance = createInstanceOfElement(nestedCanvas, parentInstance);
                    if (nestedInstance) {
                        m_childInstances[nestedChild->getId()] = nestedInstance;
                        connectToSourceElement(nestedInstance, nestedCanvas);
                        elementModel->addElement(nestedInstance);
                    }
                }
            }
        }
    }
}

void ComponentInstance::onVariantChildRemoved(Element* child)
{
    if (!child) {
        return;
    }
    
    // Check if we have an instance for this child
    CanvasElement* childInstance = m_childInstances.value(child->getId());
    if (!childInstance) {
        return;
    }
    
    qDebug() << "ComponentInstance::onVariantChildRemoved - Removing child" << child->getId() 
             << "from instance" << getId();
    
    // Get element model to remove nested children first
    Application* app = Application::instance();
    if (app && app->activeCanvas() && app->activeCanvas()->elementModel()) {
        ElementModel* elementModel = app->activeCanvas()->elementModel();
        
        // Get all nested children instances that need to be removed
        QList<Element*> nestedInstances = elementModel->getChildrenRecursive(childInstance->getId());
        
        // Remove nested instances in reverse order (deepest first)
        for (int i = nestedInstances.size() - 1; i >= 0; --i) {
            Element* nestedInstance = nestedInstances[i];
            
            // Remove from our tracking
            for (auto it = m_childInstances.begin(); it != m_childInstances.end(); ) {
                if (it.value() == nestedInstance) {
                    // Disconnect connections for this child
                    m_childConnections.remove(qobject_cast<CanvasElement*>(nestedInstance));
                    it = m_childInstances.erase(it);
                } else {
                    ++it;
                }
            }
            
            // Remove from element model
            elementModel->removeElement(nestedInstance->getId());
        }
        
        // Now remove the direct child instance
        m_childInstances.remove(child->getId());
        m_childConnections.remove(childInstance);
        elementModel->removeElement(childInstance->getId());
    }
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


void ComponentInstance::onElementAdded(Element* element)
{
    qDebug() << "ComponentInstance::onElementAdded -" << element->getId() 
             << "type:" << element->getTypeName()
             << "parent:" << element->getParentElementId();
    
    if (!element || element == m_sourceVariant || !m_sourceVariant) {
        return;
    }
    
    // Check if it's already a child of our variant
    if (element->getParentElementId() == m_sourceVariant->getId()) {
        qDebug() << "ComponentInstance::onElementAdded - New element" << element->getId() 
                 << "is already child of variant" << m_sourceVariant->getId();
        onVariantChildAdded(element);
    }
    
    // Connect to track future parent changes for this new element
    m_variantConnections.add(
        connect(element, &Element::parentIdChanged,
                this, &ComponentInstance::onElementParentChanged)
    );
}

void ComponentInstance::onElementParentChanged()
{
    Element* element = qobject_cast<Element*>(sender());
    if (!element || !m_sourceVariant) {
        return;
    }
    
    qDebug() << "ComponentInstance::onElementParentChanged for element" << element->getId()
             << "new parent:" << element->getParentElementId()
             << "our variant:" << m_sourceVariant->getId();
    
    if (element->getParentElementId() == m_sourceVariant->getId()) {
        // Element was just parented to our variant
        qDebug() << "Element parented to our variant, adding child";
        onVariantChildAdded(element);
    } else {
        // Check if element was previously a child of our variant
        if (m_childInstances.contains(element->getId())) {
            qDebug() << "Element unparented from our variant, removing child";
            onVariantChildRemoved(element);
        }
    }
}