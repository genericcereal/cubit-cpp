#ifndef PROPERTYREGISTRATIONHELPER_H
#define PROPERTYREGISTRATIONHELPER_H

#include "PropertyRegistry.h"
#include "PropertyMetadata.h"
#include "PropertyTypeMapper.h"
#include <QString>
#include <QVariant>

class PropertyRegistrationHelper
{
public:
    // Helper function to register a property with accepted variable types
    static void registerPropertyWithTypes(PropertyRegistry* registry,
                                         const QString& name,
                                         const QString& displayName,
                                         const QString& category,
                                         PropertyMetadata::PropertyType type,
                                         const QVariant& defaultValue)
    {
        if (!registry) return;
        
        PropertyMetadata* metadata = new PropertyMetadata(
            name,
            displayName,
            category,
            type,
            defaultValue
        );
        
        // Get accepted variable types from the mapper
        PropertyTypeMapper* mapper = PropertyTypeMapper::instance();
        QStringList acceptedTypes = mapper->getAcceptedTypes(name);
        metadata->setAcceptsVariableTypes(acceptedTypes);
        
        registry->registerProperty(name, metadata);
    }
    
    // Convenience overloads for common property types
    static void registerNumberProperty(PropertyRegistry* registry,
                                     const QString& name,
                                     const QString& displayName,
                                     const QString& category,
                                     double defaultValue,
                                     double minValue = 0.0,
                                     double maxValue = 999999.0)
    {
        PropertyMetadata* metadata = new PropertyMetadata(
            name,
            displayName,
            category,
            PropertyMetadata::Number,
            defaultValue
        );
        
        metadata->setMinValue(minValue);
        metadata->setMaxValue(maxValue);
        
        // Get accepted variable types from the mapper
        PropertyTypeMapper* mapper = PropertyTypeMapper::instance();
        QStringList acceptedTypes = mapper->getAcceptedTypes(name);
        metadata->setAcceptsVariableTypes(acceptedTypes);
        
        registry->registerProperty(name, metadata);
    }
    
    static void registerColorProperty(PropertyRegistry* registry,
                                    const QString& name,
                                    const QString& displayName,
                                    const QString& category,
                                    const QColor& defaultValue)
    {
        PropertyMetadata* metadata = new PropertyMetadata(
            name,
            displayName,
            category,
            PropertyMetadata::Color,
            defaultValue
        );
        
        // Get accepted variable types from the mapper
        PropertyTypeMapper* mapper = PropertyTypeMapper::instance();
        QStringList acceptedTypes = mapper->getAcceptedTypes(name);
        metadata->setAcceptsVariableTypes(acceptedTypes);
        
        registry->registerProperty(name, metadata);
    }
    
    static void registerBoolProperty(PropertyRegistry* registry,
                                   const QString& name,
                                   const QString& displayName,
                                   const QString& category,
                                   bool defaultValue)
    {
        PropertyMetadata* metadata = new PropertyMetadata(
            name,
            displayName,
            category,
            PropertyMetadata::Bool,
            defaultValue
        );
        
        // Get accepted variable types from the mapper
        PropertyTypeMapper* mapper = PropertyTypeMapper::instance();
        QStringList acceptedTypes = mapper->getAcceptedTypes(name);
        metadata->setAcceptsVariableTypes(acceptedTypes);
        
        registry->registerProperty(name, metadata);
    }
    
    static void registerEnumProperty(PropertyRegistry* registry,
                                   const QString& name,
                                   const QString& displayName,
                                   const QString& category,
                                   int defaultValue,
                                   const QStringList& enumValues)
    {
        PropertyMetadata* metadata = new PropertyMetadata(
            name,
            displayName,
            category,
            PropertyMetadata::Enum,
            defaultValue
        );
        
        metadata->setEnumValues(enumValues);
        
        // Get accepted variable types from the mapper
        PropertyTypeMapper* mapper = PropertyTypeMapper::instance();
        QStringList acceptedTypes = mapper->getAcceptedTypes(name);
        metadata->setAcceptsVariableTypes(acceptedTypes);
        
        registry->registerProperty(name, metadata);
    }
    
    static void registerTextProperty(PropertyRegistry* registry,
                                   const QString& name,
                                   const QString& displayName,
                                   const QString& category,
                                   const QString& defaultValue)
    {
        PropertyMetadata* metadata = new PropertyMetadata(
            name,
            displayName,
            category,
            PropertyMetadata::Text,
            defaultValue
        );
        
        // Get accepted variable types from the mapper
        PropertyTypeMapper* mapper = PropertyTypeMapper::instance();
        QStringList acceptedTypes = mapper->getAcceptedTypes(name);
        metadata->setAcceptsVariableTypes(acceptedTypes);
        
        registry->registerProperty(name, metadata);
    }
};

#endif // PROPERTYREGISTRATIONHELPER_H