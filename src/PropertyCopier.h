#pragma once
#include <QObject>
#include <QMetaProperty>
#include <QStringList>
#include <QVariant>

class PropertyCopier {
public:
    // Copy specific properties from source to target
    static void copyProperties(QObject* source, QObject* target, const QStringList& properties) 
    {
        if (!source || !target) {
            return;
        }
        
        const QMetaObject* sourceMetaObj = source->metaObject();
        const QMetaObject* targetMetaObj = target->metaObject();
        
        for (const QString& propertyName : properties) {
            // Get source property
            int sourceIndex = sourceMetaObj->indexOfProperty(propertyName.toUtf8().constData());
            if (sourceIndex < 0) {
                continue;
            }
            
            QMetaProperty sourceProperty = sourceMetaObj->property(sourceIndex);
            
            // Get target property
            int targetIndex = targetMetaObj->indexOfProperty(propertyName.toUtf8().constData());
            if (targetIndex < 0) {
                continue;
            }
            
            QMetaProperty targetProperty = targetMetaObj->property(targetIndex);
            
            // Copy value if both properties exist and target is writable
            if (targetProperty.isWritable()) {
                QVariant value = sourceProperty.read(source);
                targetProperty.write(target, value);
            }
        }
    }
    
    // Copy all matching properties from source to target
    static void copyAllProperties(QObject* source, QObject* target, const QStringList& excludeList = QStringList()) 
    {
        if (!source || !target) {
            return;
        }
        
        const QMetaObject* sourceMetaObj = source->metaObject();
        const QMetaObject* targetMetaObj = target->metaObject();
        
        // Iterate through all source properties
        for (int i = 0; i < sourceMetaObj->propertyCount(); ++i) {
            QMetaProperty sourceProperty = sourceMetaObj->property(i);
            const char* propertyName = sourceProperty.name();
            
            // Skip if in exclude list
            if (excludeList.contains(QString(propertyName))) {
                continue;
            }
            
            // Find matching property in target
            int targetIndex = targetMetaObj->indexOfProperty(propertyName);
            if (targetIndex >= 0) {
                QMetaProperty targetProperty = targetMetaObj->property(targetIndex);
                
                // Copy if writable
                if (targetProperty.isWritable()) {
                    QVariant value = sourceProperty.read(source);
                    targetProperty.write(target, value);
                }
            }
        }
    }
};