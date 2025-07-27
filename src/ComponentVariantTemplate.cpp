#include "ComponentVariantTemplate.h"
#include "ComponentInstance.h"
#include "ComponentInstanceTemplate.h"
#include <QMetaProperty>
#include <QDebug>

// FrameComponentVariantTemplate implementation
FrameComponentVariantTemplate::FrameComponentVariantTemplate(const QString &id, QObject *parent)
    : Frame(id, parent), ComponentVariant(id)
{
}

void FrameComponentVariantTemplate::setInstancesAcceptChildren(bool accept)
{
    if (ComponentVariant::instancesAcceptChildren() != accept) {
        ComponentVariant::setInstancesAcceptChildren(accept);
        emit instancesAcceptChildrenChanged();
    }
}

void FrameComponentVariantTemplate::setEditableProperties(const QStringList& properties)
{
    if (ComponentVariant::editableProperties() != properties) {
        ComponentVariant::setEditableProperties(properties);
        emit editablePropertiesChanged();
    }
}

void FrameComponentVariantTemplate::setVariantName(const QString& name)
{
    if (ComponentVariant::variantName() != name) {
        ComponentVariant::setVariantName(name);
        emit variantNameChanged();
    }
}

void FrameComponentVariantTemplate::applyToInstance(ComponentInstance* instance)
{
    if (!instance) return;
    
    // Cast to FrameComponentInstanceTemplate to access Frame properties
    FrameComponentInstanceTemplate* frameInstance = dynamic_cast<FrameComponentInstanceTemplate*>(instance);
    if (!frameInstance) return;
    
    // Apply all Frame properties to the instance
    const QMetaObject* metaObj = &Frame::staticMetaObject;
    for (int i = 0; i < metaObj->propertyCount(); ++i) {
        QMetaProperty prop = metaObj->property(i);
        if (prop.isWritable() && prop.isReadable()) {
            QString propName = prop.name();
            // Skip internal Qt properties
            if (propName != "objectName" && propName != "parent" && propName != "elementId") {
                QVariant value = Frame::property(prop.name());
                frameInstance->setProperty(prop.name(), value);
            }
        }
    }
}

ComponentVariant* FrameComponentVariantTemplate::clone(const QString& newId) const
{
    auto* newVariant = new FrameComponentVariantTemplate(newId);
    
    // Copy ComponentVariant properties
    newVariant->setVariantName(ComponentVariant::variantName());
    newVariant->setEditableProperties(ComponentVariant::editableProperties());
    newVariant->setInstancesAcceptChildren(ComponentVariant::instancesAcceptChildren());
    
    // Copy all Frame properties
    const QMetaObject* metaObj = &Frame::staticMetaObject;
    for (int i = 0; i < metaObj->propertyCount(); ++i) {
        QMetaProperty prop = metaObj->property(i);
        if (prop.isWritable() && prop.isReadable()) {
            QString propName = prop.name();
            if (propName != "objectName" && propName != "parent" && propName != "elementId") {
                QVariant value = Frame::property(prop.name());
                newVariant->setProperty(prop.name(), value);
            }
        }
    }
    
    return newVariant;
}

// TextComponentVariantTemplate implementation
TextComponentVariantTemplate::TextComponentVariantTemplate(const QString &id, QObject *parent)
    : Text(id, parent), ComponentVariant(id)
{
}

void TextComponentVariantTemplate::setInstancesAcceptChildren(bool accept)
{
    if (ComponentVariant::instancesAcceptChildren() != accept) {
        ComponentVariant::setInstancesAcceptChildren(accept);
        emit instancesAcceptChildrenChanged();
    }
}

void TextComponentVariantTemplate::setEditableProperties(const QStringList& properties)
{
    if (ComponentVariant::editableProperties() != properties) {
        ComponentVariant::setEditableProperties(properties);
        emit editablePropertiesChanged();
    }
}

void TextComponentVariantTemplate::setVariantName(const QString& name)
{
    if (ComponentVariant::variantName() != name) {
        ComponentVariant::setVariantName(name);
        emit variantNameChanged();
    }
}

void TextComponentVariantTemplate::applyToInstance(ComponentInstance* instance)
{
    if (!instance) return;
    
    // Cast to TextComponentInstanceTemplate to access Text properties
    TextComponentInstanceTemplate* textInstance = dynamic_cast<TextComponentInstanceTemplate*>(instance);
    if (!textInstance) return;
    
    // Apply all Text properties to the instance
    const QMetaObject* metaObj = &Text::staticMetaObject;
    for (int i = 0; i < metaObj->propertyCount(); ++i) {
        QMetaProperty prop = metaObj->property(i);
        if (prop.isWritable() && prop.isReadable()) {
            QString propName = prop.name();
            // Skip internal Qt properties
            if (propName != "objectName" && propName != "parent" && propName != "elementId") {
                QVariant value = Text::property(prop.name());
                textInstance->setProperty(prop.name(), value);
            }
        }
    }
}

ComponentVariant* TextComponentVariantTemplate::clone(const QString& newId) const
{
    auto* newVariant = new TextComponentVariantTemplate(newId);
    
    // Copy ComponentVariant properties
    newVariant->setVariantName(ComponentVariant::variantName());
    newVariant->setEditableProperties(ComponentVariant::editableProperties());
    newVariant->setInstancesAcceptChildren(ComponentVariant::instancesAcceptChildren());
    
    // Copy all Text properties
    const QMetaObject* metaObj = &Text::staticMetaObject;
    for (int i = 0; i < metaObj->propertyCount(); ++i) {
        QMetaProperty prop = metaObj->property(i);
        if (prop.isWritable() && prop.isReadable()) {
            QString propName = prop.name();
            if (propName != "objectName" && propName != "parent" && propName != "elementId") {
                QVariant value = Text::property(prop.name());
                newVariant->setProperty(prop.name(), value);
            }
        }
    }
    
    return newVariant;
}

// ShapeComponentVariantTemplate implementation
ShapeComponentVariantTemplate::ShapeComponentVariantTemplate(const QString &id, QObject *parent)
    : Shape(id, parent), ComponentVariant(id)
{
}

void ShapeComponentVariantTemplate::setInstancesAcceptChildren(bool accept)
{
    if (ComponentVariant::instancesAcceptChildren() != accept) {
        ComponentVariant::setInstancesAcceptChildren(accept);
        emit instancesAcceptChildrenChanged();
    }
}

void ShapeComponentVariantTemplate::setEditableProperties(const QStringList& properties)
{
    if (ComponentVariant::editableProperties() != properties) {
        ComponentVariant::setEditableProperties(properties);
        emit editablePropertiesChanged();
    }
}

void ShapeComponentVariantTemplate::setVariantName(const QString& name)
{
    if (ComponentVariant::variantName() != name) {
        ComponentVariant::setVariantName(name);
        emit variantNameChanged();
    }
}

void ShapeComponentVariantTemplate::applyToInstance(ComponentInstance* instance)
{
    if (!instance) return;
    
    // Cast to ShapeComponentInstanceTemplate to access Shape properties
    ShapeComponentInstanceTemplate* shapeInstance = dynamic_cast<ShapeComponentInstanceTemplate*>(instance);
    if (!shapeInstance) return;
    
    // Apply all Shape properties to the instance
    const QMetaObject* metaObj = &Shape::staticMetaObject;
    for (int i = 0; i < metaObj->propertyCount(); ++i) {
        QMetaProperty prop = metaObj->property(i);
        if (prop.isWritable() && prop.isReadable()) {
            QString propName = prop.name();
            // Skip internal Qt properties
            if (propName != "objectName" && propName != "parent" && propName != "elementId") {
                QVariant value = Shape::property(prop.name());
                shapeInstance->setProperty(prop.name(), value);
            }
        }
    }
}

ComponentVariant* ShapeComponentVariantTemplate::clone(const QString& newId) const
{
    auto* newVariant = new ShapeComponentVariantTemplate(newId);
    
    // Copy ComponentVariant properties
    newVariant->setVariantName(ComponentVariant::variantName());
    newVariant->setEditableProperties(ComponentVariant::editableProperties());
    newVariant->setInstancesAcceptChildren(ComponentVariant::instancesAcceptChildren());
    
    // Copy all Shape properties
    const QMetaObject* metaObj = &Shape::staticMetaObject;
    for (int i = 0; i < metaObj->propertyCount(); ++i) {
        QMetaProperty prop = metaObj->property(i);
        if (prop.isWritable() && prop.isReadable()) {
            QString propName = prop.name();
            if (propName != "objectName" && propName != "parent" && propName != "elementId") {
                QVariant value = Shape::property(prop.name());
                newVariant->setProperty(prop.name(), value);
            }
        }
    }
    
    return newVariant;
}

// WebTextInputComponentVariantTemplate implementation
WebTextInputComponentVariantTemplate::WebTextInputComponentVariantTemplate(const QString &id, QObject *parent)
    : WebTextInput(id, parent), ComponentVariant(id)
{
}

void WebTextInputComponentVariantTemplate::setInstancesAcceptChildren(bool accept)
{
    if (ComponentVariant::instancesAcceptChildren() != accept) {
        ComponentVariant::setInstancesAcceptChildren(accept);
        emit instancesAcceptChildrenChanged();
    }
}

void WebTextInputComponentVariantTemplate::setEditableProperties(const QStringList& properties)
{
    if (ComponentVariant::editableProperties() != properties) {
        ComponentVariant::setEditableProperties(properties);
        emit editablePropertiesChanged();
    }
}

void WebTextInputComponentVariantTemplate::setVariantName(const QString& name)
{
    if (ComponentVariant::variantName() != name) {
        ComponentVariant::setVariantName(name);
        emit variantNameChanged();
    }
}

void WebTextInputComponentVariantTemplate::applyToInstance(ComponentInstance* instance)
{
    if (!instance) return;
    
    // Cast to WebTextInputComponentInstanceTemplate to access WebTextInput properties
    WebTextInputComponentInstanceTemplate* webTextInputInstance = dynamic_cast<WebTextInputComponentInstanceTemplate*>(instance);
    if (!webTextInputInstance) return;
    
    // Apply all WebTextInput properties to the instance
    const QMetaObject* metaObj = &WebTextInput::staticMetaObject;
    for (int i = 0; i < metaObj->propertyCount(); ++i) {
        QMetaProperty prop = metaObj->property(i);
        if (prop.isWritable() && prop.isReadable()) {
            QString propName = prop.name();
            // Skip internal Qt properties
            if (propName != "objectName" && propName != "parent" && propName != "elementId") {
                QVariant value = WebTextInput::property(prop.name());
                webTextInputInstance->setProperty(prop.name(), value);
            }
        }
    }
}

ComponentVariant* WebTextInputComponentVariantTemplate::clone(const QString& newId) const
{
    auto* newVariant = new WebTextInputComponentVariantTemplate(newId);
    
    // Copy ComponentVariant properties
    newVariant->setVariantName(ComponentVariant::variantName());
    newVariant->setEditableProperties(ComponentVariant::editableProperties());
    newVariant->setInstancesAcceptChildren(ComponentVariant::instancesAcceptChildren());
    
    // Copy all WebTextInput properties
    const QMetaObject* metaObj = &WebTextInput::staticMetaObject;
    for (int i = 0; i < metaObj->propertyCount(); ++i) {
        QMetaProperty prop = metaObj->property(i);
        if (prop.isWritable() && prop.isReadable()) {
            QString propName = prop.name();
            if (propName != "objectName" && propName != "parent" && propName != "elementId") {
                QVariant value = WebTextInput::property(prop.name());
                newVariant->setProperty(prop.name(), value);
            }
        }
    }
    
    return newVariant;
}