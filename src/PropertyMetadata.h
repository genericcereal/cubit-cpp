#ifndef PROPERTYMETADATA_H
#define PROPERTYMETADATA_H

#include <QString>
#include <QVariant>
#include <QObject>

class PropertyMetadata : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QString name READ name CONSTANT)
    Q_PROPERTY(QString displayName READ displayName CONSTANT)
    Q_PROPERTY(QString category READ category CONSTANT)
    Q_PROPERTY(PropertyType type READ type CONSTANT)
    Q_PROPERTY(QVariant defaultValue READ defaultValue CONSTANT)
    Q_PROPERTY(QVariant minValue READ minValue CONSTANT)
    Q_PROPERTY(QVariant maxValue READ maxValue CONSTANT)
    Q_PROPERTY(QStringList enumValues READ enumValues CONSTANT)
    Q_PROPERTY(bool readOnly READ readOnly CONSTANT)
    Q_PROPERTY(QStringList acceptsVariableTypes READ acceptsVariableTypes CONSTANT)
    
public:
    enum PropertyType {
        Text,
        Number,
        Bool,
        Color,
        Enum,
        Label
    };
    Q_ENUM(PropertyType)
    
    PropertyMetadata(QObject* parent = nullptr) : QObject(parent) {}
    
    PropertyMetadata(const QString& name,
                    const QString& displayName,
                    const QString& category,
                    PropertyType type,
                    const QVariant& defaultValue = QVariant(),
                    QObject* parent = nullptr)
        : QObject(parent)
        , m_name(name)
        , m_displayName(displayName)
        , m_category(category)
        , m_type(type)
        , m_defaultValue(defaultValue)
        , m_readOnly(false)
    {}
    
    QString name() const { return m_name; }
    QString displayName() const { return m_displayName; }
    QString category() const { return m_category; }
    PropertyType type() const { return m_type; }
    QVariant defaultValue() const { return m_defaultValue; }
    QVariant minValue() const { return m_minValue; }
    QVariant maxValue() const { return m_maxValue; }
    QStringList enumValues() const { return m_enumValues; }
    bool readOnly() const { return m_readOnly; }
    QStringList acceptsVariableTypes() const { return m_acceptsVariableTypes; }
    
    void setMinValue(const QVariant& value) { m_minValue = value; }
    void setMaxValue(const QVariant& value) { m_maxValue = value; }
    void setEnumValues(const QStringList& values) { m_enumValues = values; }
    void setReadOnly(bool readOnly) { m_readOnly = readOnly; }
    void setAcceptsVariableTypes(const QStringList& types) { m_acceptsVariableTypes = types; }
    
private:
    QString m_name;
    QString m_displayName;
    QString m_category;
    PropertyType m_type;
    QVariant m_defaultValue;
    QVariant m_minValue;
    QVariant m_maxValue;
    QStringList m_enumValues;
    bool m_readOnly;
    QStringList m_acceptsVariableTypes;
};

#endif // PROPERTYMETADATA_H