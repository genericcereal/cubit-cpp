#include "ShapeComponentVariant.h"
#include "ShapeComponentInstance.h"
#include "DesignElement.h"
#include "Scripts.h"

ShapeComponentVariant::ShapeComponentVariant(const QString &id, QObject *parent)
    : Shape(id, parent)
    , ComponentVariant(id)
{
    elementType = Element::ShapeComponentVariantType;
    
    // Prevent scripts initialization by clearing the pointer
    // Scripts were initialized in DesignElement constructor, but variants shouldn't have scripts
    m_scripts.reset();
    
    // Set a more appropriate name
    setName(QString("Variant %1").arg(id.right(4)));
    setVariantName(QString("Variant %1").arg(id.right(4)));
}

ShapeComponentVariant::~ShapeComponentVariant() = default;

void ShapeComponentVariant::setInstancesAcceptChildren(bool accept)
{
    if (ComponentVariant::instancesAcceptChildren() != accept) {
        ComponentVariant::setInstancesAcceptChildren(accept);
        emit instancesAcceptChildrenChanged();
    }
}

void ShapeComponentVariant::setEditableProperties(const QStringList& properties)
{
    if (ComponentVariant::editableProperties() != properties) {
        ComponentVariant::setEditableProperties(properties);
        emit editablePropertiesChanged();
    }
}

void ShapeComponentVariant::setVariantName(const QString& name)
{
    if (ComponentVariant::variantName() != name) {
        ComponentVariant::setVariantName(name);
        emit variantNameChanged();
    }
}

void ShapeComponentVariant::applyToInstance(ComponentInstance* instance)
{
    ShapeComponentInstance* shapeInstance = dynamic_cast<ShapeComponentInstance*>(instance);
    if (!shapeInstance) {
        return;
    }
    
    // Apply properties with special handling for editableProperties
    DesignElement::copyElementProperties(shapeInstance, this, true);
    
    // Apply shape-specific properties
    shapeInstance->setShapeType(shapeType());
    shapeInstance->setEdgeWidth(edgeWidth());
    shapeInstance->setEdgeColor(edgeColor());
    shapeInstance->setFillColor(fillColor());
    shapeInstance->setHasFill(hasFill());
    
    // The joints will be automatically updated based on the shape type and size
}

ComponentVariant* ShapeComponentVariant::clone(const QString& newId) const
{
    ShapeComponentVariant* cloned = new ShapeComponentVariant(newId, parent());
    
    // Copy base properties
    DesignElement::copyElementProperties(cloned, const_cast<ShapeComponentVariant*>(this), true);
    
    // Copy shape-specific properties
    cloned->setShapeType(shapeType());
    cloned->setEdgeWidth(edgeWidth());
    cloned->setEdgeColor(edgeColor());
    cloned->setFillColor(fillColor());
    cloned->setHasFill(hasFill());
    
    // Copy variant properties
    cloned->setEditableProperties(editableProperties());
    cloned->setInstancesAcceptChildren(instancesAcceptChildren());
    cloned->setVariantName(variantName());
    
    return cloned;
}