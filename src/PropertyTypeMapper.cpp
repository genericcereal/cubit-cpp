#include "PropertyTypeMapper.h"
#include <QFile>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QDebug>

PropertyTypeMapper* PropertyTypeMapper::s_instance = nullptr;

PropertyTypeMapper::PropertyTypeMapper(QObject *parent)
    : QObject(parent)
{
}

PropertyTypeMapper::~PropertyTypeMapper()
{
}

PropertyTypeMapper* PropertyTypeMapper::instance()
{
    if (!s_instance) {
        s_instance = new PropertyTypeMapper();
    }
    return s_instance;
}

bool PropertyTypeMapper::loadMappings(const QString& filePath)
{
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly)) {
        emit loadError(QString("Could not open file: %1").arg(filePath));
        return false;
    }
    
    QByteArray data = file.readAll();
    file.close();
    
    QJsonParseError error;
    QJsonDocument doc = QJsonDocument::fromJson(data, &error);
    
    if (error.error != QJsonParseError::NoError) {
        emit loadError(QString("JSON parse error: %1").arg(error.errorString()));
        return false;
    }
    
    if (!doc.isObject()) {
        emit loadError("Invalid JSON format: root element must be an object");
        return false;
    }
    
    QJsonObject root = doc.object();
    
    // Load property type mappings
    if (root.contains("propertyTypeMappings") && root["propertyTypeMappings"].isObject()) {
        QJsonObject mappings = root["propertyTypeMappings"].toObject();
        m_propertyMappings.clear();
        
        for (auto it = mappings.begin(); it != mappings.end(); ++it) {
            QString key = it.key();
            
            // Skip comment entries
            if (key.startsWith("//")) {
                continue;
            }
            
            if (it.value().isArray()) {
                QJsonArray types = it.value().toArray();
                QStringList typeList;
                
                for (const QJsonValue& type : types) {
                    if (type.isString()) {
                        typeList.append(type.toString());
                    }
                }
                
                if (!typeList.isEmpty()) {
                    m_propertyMappings[key] = typeList;
                }
            }
        }
    }
    
    // Load variable type definitions
    if (root.contains("variableTypes") && root["variableTypes"].isObject()) {
        m_variableTypes = root["variableTypes"].toObject();
    }
    
    emit mappingsLoaded();
    return true;
}

QStringList PropertyTypeMapper::getAcceptedTypes(const QString& propertyName) const
{
    if (m_propertyMappings.contains(propertyName)) {
        return m_propertyMappings[propertyName];
    }
    return QStringList();
}

bool PropertyTypeMapper::isTypeAccepted(const QString& propertyName, const QString& variableType) const
{
    if (m_propertyMappings.contains(propertyName)) {
        return m_propertyMappings[propertyName].contains(variableType);
    }
    return false;
}

QStringList PropertyTypeMapper::getAvailableVariableTypes() const
{
    QStringList types;
    for (auto it = m_variableTypes.begin(); it != m_variableTypes.end(); ++it) {
        types.append(it.key());
    }
    return types;
}

QString PropertyTypeMapper::getVariableTypeDisplayName(const QString& type) const
{
    if (m_variableTypes.contains(type) && m_variableTypes[type].isObject()) {
        QJsonObject typeObj = m_variableTypes[type].toObject();
        if (typeObj.contains("displayName") && typeObj["displayName"].isString()) {
            return typeObj["displayName"].toString();
        }
    }
    return type; // Return the type itself if no display name is found
}

QVariant PropertyTypeMapper::getVariableTypeDefaultValue(const QString& type) const
{
    if (m_variableTypes.contains(type) && m_variableTypes[type].isObject()) {
        QJsonObject typeObj = m_variableTypes[type].toObject();
        if (typeObj.contains("defaultValue")) {
            QJsonValue defaultVal = typeObj["defaultValue"];
            if (defaultVal.isString()) {
                return defaultVal.toString();
            } else if (defaultVal.isDouble()) {
                return defaultVal.toDouble();
            } else if (defaultVal.isBool()) {
                return defaultVal.toBool();
            }
        }
    }
    return QVariant(); // Return empty variant if no default value is found
}