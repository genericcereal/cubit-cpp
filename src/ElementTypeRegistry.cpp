#include "ElementTypeRegistry.h"
#include "Element.h"
#include "DesignElement.h"
#include "Frame.h"
#include "Text.h"
#include "Shape.h"
#include "ElementTemplates.h"
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

template<typename TElement>
void ElementTypeRegistry::registerElementType()
{
    ElementTypeInfo info;
    info.typeName = ElementCreator<TElement>::typeName();
    info.displayName = info.typeName; // Could be enhanced with a display name trait
    info.category = "Basic"; // Could be enhanced with a category trait
    info.isContainer = std::is_same<TElement, Frame>::value; // Simple check for now
    info.acceptsChildren = info.isContainer;
    info.properties = ElementCreator<TElement>::propertyDefinitions();
    info.factory = [](const QString& id) -> DesignElement* {
        return ElementCreator<TElement>::createAsDesignElement(id);
    };
    
    m_types[info.typeName] = info;
}

// Explicit instantiations for our element types
template void ElementTypeRegistry::registerElementType<Frame>();
template void ElementTypeRegistry::registerElementType<Text>();
template void ElementTypeRegistry::registerElementType<Shape>();

void ElementTypeRegistry::initializeDefaultTypes()
{
    // Register element types using templates
    registerElementType<Frame>();
    registerElementType<Text>();
    registerElementType<Shape>();
}