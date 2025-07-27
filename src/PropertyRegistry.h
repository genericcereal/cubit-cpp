#ifndef PROPERTYREGISTRY_H
#define PROPERTYREGISTRY_H

#include <QObject>
#include <QVariant>
#include <QStringList>
#include <memory>
#include <unordered_map>
#include "PropertyMetadata.h"

class PropertyRegistry : public QObject
{
    Q_OBJECT

public:
    explicit PropertyRegistry(QObject *parent = nullptr);
    ~PropertyRegistry();

    // Register a property with its name and default value
    void registerProperty(const QString& name, const QVariant& defaultValue);
    
    // Register a property with metadata
    void registerProperty(const QString& name, PropertyMetadata* metadata);
    
    // Get property value
    QVariant get(const QString& name) const;
    
    // Set property value
    void set(const QString& name, const QVariant& value);
    
    // Check if property exists
    bool hasProperty(const QString& name) const;
    
    // Get all registered property names
    QStringList propertyNames() const;
    
    // Get property names by category
    Q_INVOKABLE QStringList propertyNamesByCategory(const QString& category) const;
    
    // Get property metadata
    Q_INVOKABLE PropertyMetadata* getMetadata(const QString& name) const;
    
    // Get all properties as a map
    QVariantMap properties() const;
    
    // Batch update properties
    void setProperties(const QVariantMap& properties);

signals:
    void propertyChanged(const QString& name, const QVariant& oldValue, const QVariant& newValue);

private:
    struct PropertyInfo {
        QVariant value;
        QVariant defaultValue;
        std::unique_ptr<PropertyMetadata> metadata;
        
        // Default constructor
        PropertyInfo() = default;
        
        // Move constructor
        PropertyInfo(PropertyInfo&& other) noexcept
            : value(std::move(other.value))
            , defaultValue(std::move(other.defaultValue))
            , metadata(std::move(other.metadata))
        {}
        
        // Move assignment operator
        PropertyInfo& operator=(PropertyInfo&& other) noexcept {
            if (this != &other) {
                value = std::move(other.value);
                defaultValue = std::move(other.defaultValue);
                metadata = std::move(other.metadata);
            }
            return *this;
        }
        
        // Delete copy operations
        PropertyInfo(const PropertyInfo&) = delete;
        PropertyInfo& operator=(const PropertyInfo&) = delete;
    };
    
    std::unordered_map<QString, PropertyInfo> m_properties;
};

#endif // PROPERTYREGISTRY_H