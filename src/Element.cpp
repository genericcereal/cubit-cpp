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
            this, [this](const QString& name, const QVariant& oldValue, const QVariant& newValue) {                emit propertyChanged(name, newValue);
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
        case WebTextInputType:
            return "WebTextInput";
        case ShapeType:
            return "Shape";
        case ComponentType:
            return "ComponentElement";
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
        
        // If this element's new parent is a source element with instances,
        // we need to create corresponding child instances
        if (!parentId.isEmpty()) {            // We need to check if the parent is a source element with instances
            // The ElementModel will handle this check when it receives the signal
            emit childAddedToSourceElement(this, parentId);
        }
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
    
    // Check if it's a Q_PROPERTY using Qt's meta-object system
    const QMetaObject* metaObj = metaObject();
    int propertyIndex = metaObj->indexOfProperty(name.toUtf8().constData());
    if (propertyIndex >= 0) {
        QMetaProperty metaProp = metaObj->property(propertyIndex);
        return metaProp.read(this);
    }
    
    // Then check registry
    return m_properties->get(name);
}

void Element::setProperty(const QString& name, const QVariant& value)
{    // Handle core properties
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
    
    // Check if it's a Q_PROPERTY using Qt's meta-object system
    const QMetaObject* metaObj = metaObject();
    int propertyIndex = metaObj->indexOfProperty(name.toUtf8().constData());
    if (propertyIndex >= 0) {
        QMetaProperty metaProp = metaObj->property(propertyIndex);
        if (metaProp.isWritable()) {
            metaProp.write(this, value);
            triggerLayoutIfNeeded(name);
            return;
        }
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
    
    // Check if it's a Q_PROPERTY using Qt's meta-object system
    const QMetaObject* metaObj = metaObject();
    int propertyIndex = metaObj->indexOfProperty(name.toUtf8().constData());
    if (propertyIndex >= 0) {
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
