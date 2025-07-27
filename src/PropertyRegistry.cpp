#include "PropertyRegistry.h"
#include <QDebug>

PropertyRegistry::PropertyRegistry(QObject *parent)
    : QObject(parent)
{
}

PropertyRegistry::~PropertyRegistry()
{
}

void PropertyRegistry::registerProperty(const QString& name, const QVariant& defaultValue)
{
    if (m_properties.find(name) != m_properties.end()) {
        qWarning() << "PropertyRegistry: Property" << name << "already registered";
        return;
    }
    
    PropertyInfo info;
    info.value = defaultValue;
    info.defaultValue = defaultValue;
    info.metadata = nullptr;
    m_properties.emplace(name, std::move(info));
}

void PropertyRegistry::registerProperty(const QString& name, PropertyMetadata* metadata)
{
    if (m_properties.find(name) != m_properties.end()) {
        qWarning() << "PropertyRegistry: Property" << name << "already registered";
        return;
    }
    
    if (!metadata) {
        qWarning() << "PropertyRegistry: Null metadata provided for property" << name;
        return;
    }
    
    PropertyInfo info;
    info.value = metadata->defaultValue();
    info.defaultValue = metadata->defaultValue();
    info.metadata.reset(metadata);
    m_properties.emplace(name, std::move(info));
}

QVariant PropertyRegistry::get(const QString& name) const
{
    auto it = m_properties.find(name);
    if (it == m_properties.end()) {
        qWarning() << "PropertyRegistry: Property" << name << "not found";
        return QVariant();
    }
    return it->second.value;
}

void PropertyRegistry::set(const QString& name, const QVariant& value)
{
    auto it = m_properties.find(name);
    if (it == m_properties.end()) {
        qWarning() << "PropertyRegistry: Property" << name << "not found";
        return;
    }
    
    if (it->second.value != value) {
        QVariant oldValue = it->second.value;
        it->second.value = value;
        emit propertyChanged(name, oldValue, value);
    }
}

bool PropertyRegistry::hasProperty(const QString& name) const
{
    return m_properties.find(name) != m_properties.end();
}

QStringList PropertyRegistry::propertyNames() const
{
    QStringList result;
    for (const auto& pair : m_properties) {
        result.append(pair.first);
    }
    return result;
}

QVariantMap PropertyRegistry::properties() const
{
    QVariantMap result;
    for (const auto& pair : m_properties) {
        result[pair.first] = pair.second.value;
    }
    return result;
}

void PropertyRegistry::setProperties(const QVariantMap& properties)
{
    for (auto it = properties.begin(); it != properties.end(); ++it) {
        set(it.key(), it.value());
    }
}

QStringList PropertyRegistry::propertyNamesByCategory(const QString& category) const
{
    QStringList result;
    for (const auto& pair : m_properties) {
        if (pair.second.metadata && pair.second.metadata->category() == category) {
            result.append(pair.first);
        }
    }
    return result;
}

PropertyMetadata* PropertyRegistry::getMetadata(const QString& name) const
{
    auto it = m_properties.find(name);
    if (it == m_properties.end()) {
        return nullptr;
    }
    return it->second.metadata.get();
}