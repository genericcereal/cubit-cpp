#ifndef PROPERTYDEFINITION_H
#define PROPERTYDEFINITION_H

#include <QString>
#include <QVariant>
#include <QMetaType>
#include <QMetaEnum>

class PropertyDefinition
{
public:
    enum PropertyCategory {
        Appearance,
        Layout,
        Behavior,
        Typography,
        Advanced
    };
    
    PropertyDefinition() = default;
    
    PropertyDefinition(const QString& name,
                      QMetaType::Type type,
                      const QVariant& defaultValue,
                      PropertyCategory category = Appearance,
                      bool isEditable = true)
        : m_name(name)
        , m_type(type)
        , m_defaultValue(defaultValue)
        , m_category(category)
        , m_isEditable(isEditable)
    {
    }
    
    // For enum properties
    PropertyDefinition(const QString& name,
                      const QMetaEnum& metaEnum,
                      const QVariant& defaultValue,
                      PropertyCategory category = Appearance,
                      bool isEditable = true)
        : m_name(name)
        , m_type(QMetaType::User)
        , m_defaultValue(defaultValue)
        , m_category(category)
        , m_isEditable(isEditable)
        , m_isEnum(true)
        , m_metaEnum(metaEnum)
    {
    }
    
    QString name() const { return m_name; }
    QMetaType::Type type() const { return m_type; }
    QVariant defaultValue() const { return m_defaultValue; }
    PropertyCategory category() const { return m_category; }
    bool isEditable() const { return m_isEditable; }
    bool isEnum() const { return m_isEnum; }
    QMetaEnum metaEnum() const { return m_metaEnum; }
    
    // Validation
    bool isValid() const { return !m_name.isEmpty(); }
    
private:
    QString m_name;
    QMetaType::Type m_type = QMetaType::UnknownType;
    QVariant m_defaultValue;
    PropertyCategory m_category = Appearance;
    bool m_isEditable = true;
    bool m_isEnum = false;
    QMetaEnum m_metaEnum;
};

#endif // PROPERTYDEFINITION_H