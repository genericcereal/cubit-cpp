#ifndef ELEMENTFACTORY_H
#define ELEMENTFACTORY_H

#include <QString>
#include <QList>
#include <memory>
#include "PropertyDefinition.h"

class DesignElement;
class ComponentVariant;
class ComponentInstance;

/**
 * Abstract base class for element factories.
 * Each design element type should have its own factory implementation.
 */
class ElementFactory
{
public:
    virtual ~ElementFactory() = default;
    
    // Element type information
    virtual QString typeName() const = 0;
    virtual QString displayName() const = 0;
    virtual QString category() const = 0;
    virtual bool isContainer() const = 0;
    virtual bool acceptsChildren() const = 0;
    
    // Property definitions
    virtual QList<PropertyDefinition> propertyDefinitions() const = 0;
    
    // Factory methods
    virtual DesignElement* createElement(const QString& id) const = 0;
    virtual ComponentVariant* createComponentVariant(const QString& id) const = 0;
    virtual ComponentInstance* createComponentInstance(const QString& id) const = 0;
    
    // Helper to apply default properties
    void applyDefaultProperties(DesignElement* element) const;
};

/**
 * Template base class for concrete element factories.
 * Provides common implementation for standard element types.
 */
template<typename TElement, typename TVariant, typename TInstance>
class ElementFactoryBase : public ElementFactory
{
public:
    DesignElement* createElement(const QString& id) const override
    {
        auto element = new TElement(id);
        applyDefaultProperties(element);
        return element;
    }
    
    ComponentVariant* createComponentVariant(const QString& id) const override
    {
        auto variant = new TVariant(id);
        applyDefaultProperties(variant);
        return variant;
    }
    
    ComponentInstance* createComponentInstance(const QString& id) const override
    {
        auto instance = new TInstance(id);
        applyDefaultProperties(instance);
        return instance;
    }
};

#endif // ELEMENTFACTORY_H