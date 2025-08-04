#ifndef COMPONENTTEMPLATES_H
#define COMPONENTTEMPLATES_H

#include <QObject>
#include <QString>
#include <QStringList>
#include "ComponentVariant.h"
#include "ComponentInstance.h"

/**
 * Template-based ComponentVariant that can be used for any element type.
 * This replaces individual FrameComponentVariant, TextComponentVariant, etc.
 */
template<typename TElement>
class ComponentVariantTemplate : public TElement, public ComponentVariant
{
public:
    explicit ComponentVariantTemplate(const QString &id, QObject *parent = nullptr)
        : TElement(id, parent), ComponentVariant(id)
    {
        // Set element type to indicate this is a component variant
        TElement::elementType = TElement::elementType; // Keep the base type
    }

    virtual ~ComponentVariantTemplate() = default;

    // ComponentVariant interface implementation
    void applyToInstance(ComponentInstance* instance) override
    {
        if (!instance) return;
        
        // Apply all properties from this variant to the instance
        // This is a generic implementation that works for all element types
        const QMetaObject* metaObj = TElement::staticMetaObject;
        for (int i = 0; i < metaObj->propertyCount(); ++i) {
            QMetaProperty prop = metaObj->property(i);
            if (prop.isWritable() && prop.isReadable()) {
                QString propName = prop.name();
                // Skip internal Qt properties
                if (propName != "objectName" && propName != "parent") {
                    QVariant value = TElement::property(prop.name());
                    instance->setProperty(prop.name(), value);
                }
            }
        }
    }

    ComponentVariant* clone(const QString& newId) const override
    {
        auto* newVariant = new ComponentVariantTemplate<TElement>(newId);
        
        // Copy ComponentVariant properties
        newVariant->setVariantName(ComponentVariant::variantName());
        newVariant->setEditableProperties(ComponentVariant::editableProperties());
        newVariant->setInstancesAcceptChildren(ComponentVariant::instancesAcceptChildren());
        
        // Copy all element properties
        const QMetaObject* metaObj = TElement::staticMetaObject;
        for (int i = 0; i < metaObj->propertyCount(); ++i) {
            QMetaProperty prop = metaObj->property(i);
            if (prop.isWritable() && prop.isReadable()) {
                QString propName = prop.name();
                if (propName != "objectName" && propName != "parent" && propName != "elementId") {
                    QVariant value = TElement::property(prop.name());
                    newVariant->setProperty(prop.name(), value);
                }
            }
        }
        
        return newVariant;
    }

    // Override scripts getter to return nullptr for variants
    Scripts* scripts() const override { return nullptr; }
    
    // Override to identify as component variant
    bool isComponentVariant() const override { return true; }
    
    // Override to prevent script execution for variants
    void executeScriptEvent(const QString& eventName, const QVariantMap& eventData = QVariantMap()) override { 
        Q_UNUSED(eventName); 
        Q_UNUSED(eventData); 
    }

    // Property accessors for QML binding
    bool instancesAcceptChildren() const { return ComponentVariant::instancesAcceptChildren(); }
    QStringList editableProperties() const { return ComponentVariant::editableProperties(); }
    QString variantName() const { return ComponentVariant::variantName(); }
    
    void setInstancesAcceptChildren(bool accept) { 
        ComponentVariant::setInstancesAcceptChildren(accept);
    }
    void setEditableProperties(const QStringList& properties) { 
        ComponentVariant::setEditableProperties(properties);
    }
    void setVariantName(const QString& name) { 
        ComponentVariant::setVariantName(name);
    }
};

/**
 * Template-based ComponentInstance that can be used for any element type.
 * This replaces individual FrameComponentInstance, TextComponentInstance, etc.
 */
template<typename TElement>
class ComponentInstanceTemplate : public TElement, public ComponentInstance
{
public:
    explicit ComponentInstanceTemplate(const QString &id, QObject *parent = nullptr)
        : TElement(id, parent), ComponentInstance(id)
    {
        // Set element type to indicate this is a component instance
        TElement::elementType = TElement::elementType; // Keep the base type
    }

    virtual ~ComponentInstanceTemplate() = default;

    // Override to identify this as a visual element
    bool isVisual() const override { return true; }
    
    // Override to identify this as an instance
    bool isInstance() const override { return true; }

    // Property accessors
    QString instanceOf() const { return m_instanceOf; }
    Element* sourceVariant() const { return m_sourceVariant; }
    
    void setInstanceOf(const QString &componentId) { 
        if (m_instanceOf != componentId) {
            m_instanceOf = componentId;
            // emit signal if this becomes a QObject-derived template
        }
    }
    
    void setSourceVariant(Element* variant) { 
        if (m_sourceVariant != variant) {
            m_sourceVariant = variant;
            // emit signal if this becomes a QObject-derived template
        }
    }

    // Get editable properties from source variant
    QStringList getEditableProperties() const
    {
        if (auto* variant = dynamic_cast<ComponentVariant*>(m_sourceVariant)) {
            return variant->editableProperties();
        }
        return QStringList();
    }

protected:
    QString m_instanceOf;
    Element* m_sourceVariant = nullptr;
};

/**
 * Helper functions for creating component variants and instances using templates
 */
namespace ComponentTemplateHelpers {
    template<typename TElement>
    ComponentVariantTemplate<TElement>* createVariant(const QString& id) {
        return new ComponentVariantTemplate<TElement>(id);
    }
    
    template<typename TElement>
    ComponentInstanceTemplate<TElement>* createInstance(const QString& id) {
        return new ComponentInstanceTemplate<TElement>(id);
    }
}

#endif // COMPONENTTEMPLATES_H