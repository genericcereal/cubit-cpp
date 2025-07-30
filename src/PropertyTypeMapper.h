#ifndef PROPERTYTYPEMAPPER_H
#define PROPERTYTYPEMAPPER_H

#include <QObject>
#include <QStringList>
#include <QHash>
#include <QJsonObject>

class PropertyTypeMapper : public QObject
{
    Q_OBJECT
    
public:
    static PropertyTypeMapper* instance();
    
    // Load mappings from JSON file
    bool loadMappings(const QString& filePath);
    
    // Get accepted variable types for a property
    QStringList getAcceptedTypes(const QString& propertyName) const;
    
    // Check if a variable type is accepted for a property
    Q_INVOKABLE bool isTypeAccepted(const QString& propertyName, const QString& variableType) const;
    
    // Get all available variable types
    Q_INVOKABLE QStringList getAvailableVariableTypes() const;
    
    // Get display name for a variable type
    Q_INVOKABLE QString getVariableTypeDisplayName(const QString& type) const;
    
    // Get default value for a variable type
    Q_INVOKABLE QVariant getVariableTypeDefaultValue(const QString& type) const;
    
signals:
    void mappingsLoaded();
    void loadError(const QString& error);
    
private:
    explicit PropertyTypeMapper(QObject *parent = nullptr);
    ~PropertyTypeMapper();
    
    // Prevent copying
    PropertyTypeMapper(const PropertyTypeMapper&) = delete;
    PropertyTypeMapper& operator=(const PropertyTypeMapper&) = delete;
    
    static PropertyTypeMapper* s_instance;
    
    // Property name -> list of accepted variable types
    QHash<QString, QStringList> m_propertyMappings;
    
    // Variable type definitions
    QJsonObject m_variableTypes;
};

#endif // PROPERTYTYPEMAPPER_H