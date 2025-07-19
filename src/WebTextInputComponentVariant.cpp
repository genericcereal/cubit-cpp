#include "WebTextInputComponentVariant.h"
#include "WebTextInputComponentInstance.h"
#include <QDebug>

WebTextInputComponentVariant::WebTextInputComponentVariant(const QString &id, QObject *parent)
    : WebTextInput(id, parent)
    , ComponentVariant(id)
{
    // Set the element type
    elementType = Element::WebTextInputComponentVariantType;
}

WebTextInputComponentVariant::~WebTextInputComponentVariant()
{
}

void WebTextInputComponentVariant::setInstancesAcceptChildren(bool accept)
{
    if (ComponentVariant::instancesAcceptChildren() != accept) {
        ComponentVariant::setInstancesAcceptChildren(accept);
        emit instancesAcceptChildrenChanged();
    }
}

void WebTextInputComponentVariant::setEditableProperties(const QStringList& properties)
{
    if (ComponentVariant::editableProperties() != properties) {
        ComponentVariant::setEditableProperties(properties);
        emit editablePropertiesChanged();
    }
}

void WebTextInputComponentVariant::setVariantName(const QString& name)
{
    if (ComponentVariant::variantName() != name) {
        ComponentVariant::setVariantName(name);
        emit variantNameChanged();
    }
}

void WebTextInputComponentVariant::applyToInstance(ComponentInstance* instance)
{
    WebTextInputComponentInstance* webTextInputInstance = dynamic_cast<WebTextInputComponentInstance*>(instance);
    if (!webTextInputInstance) {
        qWarning() << "WebTextInputComponentVariant::applyToInstance - Invalid instance type";
        return;
    }
    
    // Apply WebTextInput-specific properties
    // WebTextInput doesn't have setContent, it has setValue
    webTextInputInstance->setValue(value());
    // WebTextInput doesn't have font/color properties
    webTextInputInstance->setPlaceholder(placeholder());
    webTextInputInstance->setPosition(position());
    webTextInputInstance->setBorderColor(borderColor());
    webTextInputInstance->setBorderWidth(borderWidth());
    webTextInputInstance->setBorderRadius(borderRadius());
    
    // Apply geometry
    webTextInputInstance->setRect(rect());
    
    // Apply common properties
    webTextInputInstance->setName(getName());
    webTextInputInstance->setSelected(false);
    
    // Apply anchor properties
    webTextInputInstance->setLeft(left());
    webTextInputInstance->setRight(right());
    webTextInputInstance->setTop(top());
    webTextInputInstance->setBottom(bottom());
    webTextInputInstance->setLeftAnchored(leftAnchored());
    webTextInputInstance->setRightAnchored(rightAnchored());
    webTextInputInstance->setTopAnchored(topAnchored());
    webTextInputInstance->setBottomAnchored(bottomAnchored());
}

ComponentVariant* WebTextInputComponentVariant::clone(const QString& newId) const
{
    WebTextInputComponentVariant* newVariant = new WebTextInputComponentVariant(newId, parent());
    
    // Copy geometry
    newVariant->setRect(rect());
    
    // Copy all WebTextInput properties
    newVariant->setPlaceholder(placeholder());
    newVariant->setValue(value());
    newVariant->setBorderColor(borderColor());
    newVariant->setBorderWidth(borderWidth());
    newVariant->setBorderRadius(borderRadius());
    newVariant->setPosition(position());
    
    // Copy anchor properties
    newVariant->setLeft(left());
    newVariant->setRight(right());
    newVariant->setTop(top());
    newVariant->setBottom(bottom());
    newVariant->setLeftAnchored(leftAnchored());
    newVariant->setRightAnchored(rightAnchored());
    newVariant->setTopAnchored(topAnchored());
    newVariant->setBottomAnchored(bottomAnchored());
    
    // Copy variant-specific properties
    newVariant->setInstancesAcceptChildren(instancesAcceptChildren());
    newVariant->setEditableProperties(editableProperties());
    
    return newVariant;
}