#pragma once
#include <QString>
#include <QStringList>
#include <QMap>
#include <QSet>

// Port types for node connections
namespace PortType {
    // Control flow type
    const QString Flow = "Flow";
    
    // Data types
    const QString Boolean = "Boolean";
    const QString String = "String";
    const QString Number = "Number";
    
    // Type categories for future extensibility
    enum Category {
        Control,
        Primitive,
        Collection,
        Custom
    };
    
    // Type information structure
    struct TypeInfo {
        QString name;
        Category category;
        QString displayName;
        QString description;
        QSet<QString> compatibleTypes; // For future type coercion
    };
    
    // Registry of all port types
    inline QMap<QString, TypeInfo> getTypeRegistry() {
        static QMap<QString, TypeInfo> registry;
        if (registry.isEmpty()) {
            // Control types
            registry[Flow] = {Flow, Control, "Flow", "Control flow connection", {Flow}};
            
            // Primitive data types
            registry[Boolean] = {Boolean, Primitive, "Boolean", "True/False value", {Boolean}};
            registry[String] = {String, Primitive, "String", "Text value", {String}};
            registry[Number] = {Number, Primitive, "Number", "Numeric value", {Number}};
        }
        return registry;
    }
    
    // Get all registered type names
    inline QStringList getAllTypes() {
        return getTypeRegistry().keys();
    }
    
    // Get types by category
    inline QStringList getTypesByCategory(Category category) {
        QStringList types;
        const auto registry = getTypeRegistry();
        for (auto it = registry.begin(); it != registry.end(); ++it) {
            if (it.value().category == category) {
                types << it.key();
            }
        }
        return types;
    }
    
    // Check if a type exists
    inline bool isValidType(const QString &type) {
        return getTypeRegistry().contains(type);
    }
    
    // Validate if two port types can connect
    inline bool canConnect(const QString &sourceType, const QString &targetType) {
        const auto registry = getTypeRegistry();
        
        // Check if both types are valid
        if (!registry.contains(sourceType) || !registry.contains(targetType)) {
            return false;
        }
        
        // For now, only exact type matches are allowed
        // In the future, we can check compatibleTypes for type coercion
        return sourceType == targetType;
    }
    
    // Helper to migrate old "Variable" type to new types
    inline QString migrateVariableType(const QString &type) {
        if (type == "Variable") {
            // Default migration to String type
            return String;
        }
        return type;
    }
}