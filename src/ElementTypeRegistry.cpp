#include "ElementTypeRegistry.h"
#include "Element.h"
#include "DesignElement.h"
#include "Frame.h"
#include "Text.h"
#include "ElementFactory.h"
#include "FrameFactory.h"
#include "TextFactory.h"
#include "WebTextInputFactory.h"
#include "ShapeFactory.h"
#include <QDebug>
#include <QMetaEnum>

ElementTypeRegistry& ElementTypeRegistry::instance()
{
    static ElementTypeRegistry instance;
    return instance;
}

void ElementTypeRegistry::registerType(const ElementTypeInfo& typeInfo)
{
    if (typeInfo.typeName.isEmpty() || !typeInfo.factory) {
        qWarning() << "Invalid element type registration:" << typeInfo.typeName;
        return;
    }
    
    m_types[typeInfo.typeName] = typeInfo;
}

DesignElement* ElementTypeRegistry::createElement(const QString& typeName, const QString& id) const
{
    if (!m_types.contains(typeName)) {
        qWarning() << "Unknown element type:" << typeName;
        return nullptr;
    }
    
    const ElementTypeInfo& typeInfo = m_types[typeName];
    DesignElement* element = typeInfo.factory(id);
    
    if (element) {
        // Apply default property values
        for (const PropertyDefinition& prop : typeInfo.properties) {
            if (prop.isValid()) {
                element->setProperty(prop.name().toUtf8(), prop.defaultValue());
            }
        }
    }
    
    return element;
}

bool ElementTypeRegistry::hasType(const QString& typeName) const
{
    return m_types.contains(typeName);
}

ElementTypeInfo ElementTypeRegistry::getTypeInfo(const QString& typeName) const
{
    return m_types.value(typeName);
}

QList<QString> ElementTypeRegistry::registeredTypes() const
{
    return m_types.keys();
}

QList<QString> ElementTypeRegistry::typesByCategory(const QString& category) const
{
    QList<QString> result;
    for (auto it = m_types.constBegin(); it != m_types.constEnd(); ++it) {
        if (it.value().category == category) {
            result.append(it.key());
        }
    }
    return result;
}

QList<PropertyDefinition> ElementTypeRegistry::getProperties(const QString& typeName) const
{
    if (m_types.contains(typeName)) {
        return m_types[typeName].properties;
    }
    return QList<PropertyDefinition>();
}

void ElementTypeRegistry::registerFactory(std::shared_ptr<ElementFactory> factory)
{
    if (!factory) {
        qWarning() << "Cannot register null factory";
        return;
    }
    
    const QString typeName = factory->typeName();
    if (typeName.isEmpty()) {
        qWarning() << "Cannot register factory with empty type name";
        return;
    }
    
    // Also create and register legacy ElementTypeInfo for backward compatibility
    ElementTypeInfo info;
    info.typeName = typeName;
    info.displayName = factory->displayName();
    info.category = factory->category();
    info.isContainer = factory->isContainer();
    info.acceptsChildren = factory->acceptsChildren();
    info.properties = factory->propertyDefinitions();
    info.factory = [factory = factory.get()](const QString& id) -> DesignElement* {
        return factory->createElement(id);
    };
    
    m_types[typeName] = info;
    m_factories[typeName] = std::move(factory);
    
}

ElementFactory* ElementTypeRegistry::getFactory(const QString& typeName) const
{
    auto it = m_factories.find(typeName);
    return it != m_factories.end() ? it->get() : nullptr;
}

void ElementTypeRegistry::initializeDefaultTypes()
{
    // Register Frame factory
    registerFactory(std::make_shared<FrameFactory>());
    
    // Register Text factory
    registerFactory(std::make_shared<TextFactory>());
    
    // Register WebTextInput factory
    registerFactory(std::make_shared<WebTextInputFactory>());
    
    // Register Shape factory
    registerFactory(std::make_shared<ShapeFactory>());
}