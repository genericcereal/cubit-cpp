#ifndef ELEMENTTYPEREGISTRY_H
#define ELEMENTTYPEREGISTRY_H

#include <QString>
#include <QHash>
#include <QList>
#include <functional>
#include <memory>
#include "PropertyDefinition.h"

class Element;
class DesignElement;
class ElementFactory;

struct ElementTypeInfo {
    QString typeName;
    QString displayName;
    QString category; // "Basic", "Layout", "Media", etc.
    QList<PropertyDefinition> properties;
    std::function<DesignElement*(const QString& id)> factory;
    bool isContainer = false;
    bool acceptsChildren = false;
};

class ElementTypeRegistry
{
public:
    static ElementTypeRegistry& instance();
    
    // Register a new element type with legacy info struct
    void registerType(const ElementTypeInfo& typeInfo);
    
    // Register a new element type with factory
    void registerFactory(std::shared_ptr<ElementFactory> factory);
    
    // Create an element of the given type
    DesignElement* createElement(const QString& typeName, const QString& id) const;
    
    // Get type information
    bool hasType(const QString& typeName) const;
    ElementTypeInfo getTypeInfo(const QString& typeName) const;
    QList<QString> registeredTypes() const;
    QList<QString> typesByCategory(const QString& category) const;
    
    // Get properties for a type
    QList<PropertyDefinition> getProperties(const QString& typeName) const;
    
    // Get factory for a type
    ElementFactory* getFactory(const QString& typeName) const;
    
    // Initialize default types (Frame, Text, etc.)
    void initializeDefaultTypes();
    
private:
    ElementTypeRegistry() = default;
    ~ElementTypeRegistry() = default;
    ElementTypeRegistry(const ElementTypeRegistry&) = delete;
    ElementTypeRegistry& operator=(const ElementTypeRegistry&) = delete;
    
    QHash<QString, ElementTypeInfo> m_types;
    QHash<QString, std::shared_ptr<ElementFactory>> m_factories;
};

#endif // ELEMENTTYPEREGISTRY_H