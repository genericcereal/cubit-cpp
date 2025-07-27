#include "Element.h"
#include "ElementModel.h"
#include "PropertyRegistry.h"
#include "PropertyMetadata.h"
#include "Application.h"
#include "Project.h"
#include "Frame.h"
#include "Text.h"
#include "Shape.h"
#include "platforms/web/WebTextInput.h"
#include "UniqueIdGenerator.h"
#include <QMetaEnum>
#include <QDebug>

Element::Element(ElementType type, const QString &id, QObject *parent)
    : QObject(parent)
    , elementType(type)
    , elementId(id)
    , parentElementId()
    , selected(false)
    , m_properties(std::make_unique<PropertyRegistry>(this))
{
    // Connect property registry changes to element's propertyChanged signal
    connect(m_properties.get(), &PropertyRegistry::propertyChanged,
            this, [this](const QString& name, const QVariant&, const QVariant& newValue) {
                emit propertyChanged(name, newValue);
                emit elementChanged();
            });
    
    // Register base element properties
    registerProperties();
}

Element::~Element() {
    // Destructor defined here so PropertyRegistry is complete
}

QString Element::getTypeName() const
{
    switch (elementType) {
        case FrameType:
            return "Frame";
        case TextType:
            return "Text";
        case VariableType:
            return "Variable";
        case NodeType:
            return "Node";
        case EdgeType:
            return "Edge";
        case ComponentType:
            return "Component";
        case FrameComponentInstanceType:
            return "FrameComponentInstance";
        case FrameComponentVariantType:
            return "FrameComponentVariant";
        case TextVariantType:
            return "TextComponentVariant";
        case WebTextInputType:
            return "WebTextInput";
        case WebTextInputComponentInstanceType:
            return "WebTextInputComponentInstance";
        case WebTextInputComponentVariantType:
            return "WebTextInputComponentVariant";
        case ShapeType:
            return "Shape";
        case ShapeComponentInstanceType:
            return "ShapeComponentInstance";
        case ShapeComponentVariantType:
            return "ShapeComponentVariant";
        default:
            return "Unknown";
    }
}

void Element::setName(const QString &newName)
{
    if (name != newName) {
        name = newName;
        emit nameChanged();
        emit elementChanged();
    }
}

void Element::setParentElementId(const QString &parentId)
{
    if (parentElementId != parentId) {
        QString oldParentId = parentElementId;
        parentElementId = parentId;
        
        qDebug() << "Element::setParentElementId - Element" << elementId << "parent changed from" << oldParentId << "to" << parentId;
        
        // Handle global frame parenting logic
        handleGlobalFrameParenting(oldParentId, parentId);
        
        // If we're being parented (not unparented), reorder in the element list
        if (!parentId.isEmpty()) {
            // Try to get the ElementModel (which should be our parent if we're in one)
            ElementModel* model = qobject_cast<ElementModel*>(parent());
            if (model) {
                // Find the parent element
                Element* parentElement = model->getElementById(parentId);
                if (parentElement) {
                    // Find the parent's index
                    int parentIndex = -1;
                    for (int i = 0; i < model->rowCount(); i++) {
                        if (model->elementAt(i) == parentElement) {
                            parentIndex = i;
                            break;
                        }
                    }
                    
                    if (parentIndex >= 0) {
                        // Find the last child of this parent
                        int lastChildIndex = parentIndex;
                        for (int i = parentIndex + 1; i < model->rowCount(); i++) {
                            Element* elem = model->elementAt(i);
                            if (elem) {
                                // Check if this element is a descendant of the parent
                                Element* current = elem;
                                bool isDescendant = false;
                                while (current && !current->getParentElementId().isEmpty()) {
                                    if (current->getParentElementId() == parentId) {
                                        isDescendant = true;
                                        lastChildIndex = i;
                                        break;
                                    }
                                    // Move up the hierarchy
                                    current = model->getElementById(current->getParentElementId());
                                }
                                if (!isDescendant) {
                                    // We've reached an element that's not a descendant, so stop
                                    break;
                                }
                            }
                        }
                        
                        // Move this element to after the last child
                        model->reorderElement(this, lastChildIndex + 1);
                    }
                }
            }
        }
        
        emit parentIdChanged();
        emit elementChanged();
    }
}

void Element::setSelected(bool sel)
{
    if (selected != sel) {
        selected = sel;
        emit selectedChanged();
    }
}

QVariant Element::getProperty(const QString& name) const
{
    // Check core properties first
    if (name == "name") return getName();
    if (name == "elementId") return getId();
    if (name == "parentId") return getParentElementId();
    if (name == "selected") return isSelected();
    
    // Then check registry
    return m_properties->get(name);
}

void Element::setProperty(const QString& name, const QVariant& value)
{
    // Handle core properties
    if (name == "name") {
        setName(value.toString());
        return;
    }
    if (name == "parentId") {
        setParentElementId(value.toString());
        return;
    }
    if (name == "selected") {
        setSelected(value.toBool());
        return;
    }
    
    // Otherwise use registry
    m_properties->set(name, value);
    triggerLayoutIfNeeded(name);
}

bool Element::hasProperty(const QString& name) const
{
    // Check core properties
    if (name == "name" || name == "elementId" || name == "parentId" || name == "selected") {
        return true;
    }
    
    return m_properties->hasProperty(name);
}

QStringList Element::propertyNames() const
{
    QStringList names = m_properties->propertyNames();
    
    // Add core properties
    names.prepend("selected");
    names.prepend("parentId");
    names.prepend("name");
    names.prepend("elementId");
    
    return names;
}

QVariantList Element::getPropertyMetadata() const
{
    QVariantList result;
    
    // Add core properties metadata
    QVariantMap nameInfo;
    nameInfo["name"] = "name";
    nameInfo["displayName"] = "Name";
    nameInfo["category"] = "General";
    nameInfo["type"] = "text";
    nameInfo["value"] = getName();
    nameInfo["readOnly"] = false;
    result.append(nameInfo);
    
    QVariantMap typeInfo;
    typeInfo["name"] = "elementType";
    typeInfo["displayName"] = "Type";
    typeInfo["category"] = "General";
    typeInfo["type"] = "label";
    typeInfo["value"] = getTypeName();
    typeInfo["readOnly"] = true;
    result.append(typeInfo);
    
    QVariantMap idInfo;
    idInfo["name"] = "elementId";
    idInfo["displayName"] = "ID";
    idInfo["category"] = "General";
    idInfo["type"] = "label";
    idInfo["value"] = getId();
    idInfo["readOnly"] = true;
    result.append(idInfo);
    
    // Get properties from registry with metadata
    QStringList registeredProps = m_properties->propertyNames();
    for (const QString& propName : registeredProps) {
        PropertyMetadata* meta = m_properties->getMetadata(propName);
        QVariantMap propInfo;
        propInfo["name"] = propName;
        propInfo["value"] = m_properties->get(propName);
        
        if (meta) {
            propInfo["displayName"] = meta->displayName();
            propInfo["category"] = meta->category();
            propInfo["type"] = PropertyMetadata::staticMetaObject.enumerator(0).valueToKey(meta->type());
            propInfo["readOnly"] = meta->readOnly();
            
            if (!meta->minValue().isNull()) propInfo["min"] = meta->minValue();
            if (!meta->maxValue().isNull()) propInfo["max"] = meta->maxValue();
            if (!meta->enumValues().isEmpty()) propInfo["enumValues"] = meta->enumValues();
        } else {
            // Fallback for properties without metadata
            propInfo["displayName"] = propName;
            propInfo["category"] = "Properties";
            propInfo["type"] = "text";
            propInfo["readOnly"] = false;
        }
        
        result.append(propInfo);
    }
    
    return result;
}

void Element::handleGlobalFrameParenting(const QString& oldParentId, const QString& newParentId)
{
    qDebug() << "Element::handleGlobalFrameParenting - Called for element" << elementId 
             << "old parent:" << oldParentId << "new parent:" << newParentId
             << "isGlobalInstance:" << m_isGlobalInstance << "isVisual:" << isVisual();
    
    // Skip if this is already a global instance
    if (m_isGlobalInstance) {
        qDebug() << "  Skipping - element is already a global instance";
        return;
    }
    
    // Only process if this element is visual and we have a model
    if (!isVisual()) {
        qDebug() << "  Skipping - element is not visual";
        return;
    }
    
    ElementModel* model = qobject_cast<ElementModel*>(parent());
    if (!model) {
        qDebug() << "  Skipping - no element model found";
        return;
    }
    
    // Check if we're being removed from a global frame
    if (!oldParentId.isEmpty()) {
        Element* oldParent = model->getElementById(oldParentId);
        qDebug() << "  Checking old parent" << oldParentId << "- found:" << (oldParent != nullptr)
                 << "is global frame:" << (oldParent ? isGlobalFrame(oldParent) : false);
        if (oldParent && isGlobalFrame(oldParent)) {
            qDebug() << "  Removing instances from all frames";
            removeInstancesFromAllFrames();
        }
    }
    
    // Check if we're being added to a global frame
    if (!newParentId.isEmpty()) {
        Element* newParent = model->getElementById(newParentId);
        qDebug() << "  Checking new parent" << newParentId << "- found:" << (newParent != nullptr)
                 << "is global frame:" << (newParent ? isGlobalFrame(newParent) : false);
        if (newParent && isGlobalFrame(newParent)) {
            qDebug() << "  Creating instances in all frames";
            createInstancesInAllFrames();
        }
    }
}

bool Element::isGlobalFrame(Element* element) const
{
    if (!element) return false;
    
    Frame* frame = qobject_cast<Frame*>(element);
    if (!frame) return false;
    
    return frame->role() == Frame::appContainer;
}

void Element::createInstancesInAllFrames()
{
    qDebug() << "Element::createInstancesInAllFrames - Called for element" << elementId;
    
    // We need to trigger the DesignCanvas to create instances
    // Find all canvases and trigger instance creation
    Application* app = Application::instance();
    if (!app) {
        qDebug() << "  No application instance found";
        return;
    }
    
    ElementModel* sourceModel = qobject_cast<ElementModel*>(parent());
    if (!sourceModel) {
        qDebug() << "  No source model found";
        return;
    }
    
    // For each project/canvas in the application
    for (const auto& project : app->canvases()) {
        ElementModel* canvasModel = project->elementModel();
        if (!canvasModel || canvasModel == sourceModel) {
            qDebug() << "  Skipping canvas (same as source or null)";
            continue;
        }
        
        qDebug() << "  Processing canvas with" << canvasModel->rowCount() << "elements";
        
        // Find all frames in this canvas (except global frames)
        QList<Element*> elements = canvasModel->getAllElements();
        for (Element* elem : elements) {
            if (elem && elem->getType() == Element::FrameType) {
                Frame* frame = qobject_cast<Frame*>(elem);
                if (frame && frame->role() != Frame::appContainer) {
                    qDebug() << "    Found target frame" << frame->getId() << "in canvas";
                    // Create instance of this element in the frame
                    createInstanceInFrame(frame, canvasModel);
                }
            }
        }
    }
}

void Element::removeInstancesFromAllFrames()
{
    Application* app = Application::instance();
    if (!app) return;
    
    ElementModel* sourceModel = qobject_cast<ElementModel*>(parent());
    if (!sourceModel) return;
    
    // For each canvas/project in the application
    for (const auto& project : app->canvases()) {
        ElementModel* canvasModel = project->elementModel();
        if (!canvasModel || canvasModel == sourceModel) continue; // Skip the source canvas
        
        // Find and remove instances of this element
        QList<Element*> elements = canvasModel->getAllElements();
        QList<Element*> toRemove;
        
        for (Element* elem : elements) {
            // Check if this element is an instance by matching properties
            if (elem->getType() == elementType && 
                !elem->showInElementList() &&
                elem->getName() == name) {
                toRemove.append(elem);
            }
        }
        
        // Remove the instances
        for (Element* elem : toRemove) {
            canvasModel->removeElement(elem);
        }
    }
}

void Element::createInstanceInFrame(Frame* targetFrame, ElementModel* targetModel)
{
    if (!targetFrame || !targetModel) return;
    
    qDebug() << "Element::createInstanceInFrame - Creating instance of" << elementId 
             << "in frame" << targetFrame->getId();
    
    // Create the appropriate type of element
    Element* instance = nullptr;
    QString instanceId = UniqueIdGenerator::generate16DigitId();
    
    switch (elementType) {
        case FrameType:
            instance = new Frame(instanceId, targetModel);
            break;
        case TextType:
            instance = new Text(instanceId, targetModel);
            break;
        case ShapeType:
            instance = new Shape(instanceId, targetModel);
            break;
        case WebTextInputType:
            instance = new WebTextInput(instanceId, targetModel);
            break;
        default:
            qDebug() << "  Unsupported element type for instance creation";
            return;
    }
    
    if (instance) {
        // Copy properties from source element
        QStringList propNames = propertyNames();
        for (const QString& propName : propNames) {
            if (propName != "elementId" && propName != "parentId" && propName != "selected") {
                if (hasProperty(propName) && instance->hasProperty(propName)) {
                    instance->setProperty(propName, getProperty(propName));
                }
            }
        }
        
        // Set the parent to the target frame
        instance->setParentElementId(targetFrame->getId());
        
        // Mark as an instance (not shown in element list)
        instance->setShowInElementList(false);
        
        // Mark as a global instance to prevent recursive parenting
        instance->setIsGlobalInstance(true);
        
        // Add to the model
        targetModel->addElement(instance);
        
        qDebug() << "  Instance created with ID" << instanceId;
    }
}