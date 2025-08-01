#pragma once
#include "Element.h"
#include <QVariant>
#include <QVariantList>

class Variable : public Element {
    Q_OBJECT
    Q_PROPERTY(QVariant value READ value WRITE setValue NOTIFY valueChanged)
    Q_PROPERTY(QString variableType READ variableType WRITE setVariableType NOTIFY variableTypeChanged)
    Q_PROPERTY(QString platform READ platform WRITE setPlatform NOTIFY platformChanged)
    Q_PROPERTY(bool isArray READ isArray WRITE setIsArray NOTIFY isArrayChanged)
    Q_PROPERTY(QVariantList arrayValues READ arrayValues NOTIFY arrayValuesChanged)
    Q_PROPERTY(int arrayLength READ arrayLength NOTIFY arrayValuesChanged)
    Q_PROPERTY(QString variableScope READ variableScope WRITE setVariableScope NOTIFY variableScopeChanged)
    Q_PROPERTY(QString linkedElementId READ linkedElementId WRITE setLinkedElementId NOTIFY linkedElementIdChanged)
    
public:
    explicit Variable(const QString &id, QObject *parent = nullptr);
    
    // Single value access
    QVariant value() const { return m_value; }
    void setValue(const QVariant &value);
    
    // Variable type
    QString variableType() const { return m_variableType; }
    void setVariableType(const QString &type);
    
    // Platform
    QString platform() const { return m_platform; }
    void setPlatform(const QString &platform);
    
    // Array properties
    bool isArray() const { return m_isArray; }
    void setIsArray(bool isArray);
    
    // Array operations
    QVariantList arrayValues() const { return m_arrayValues; }
    int arrayLength() const { return m_arrayValues.length(); }
    
    Q_INVOKABLE void addArrayValue(const QVariant &value);
    Q_INVOKABLE void removeArrayValue(int index);
    Q_INVOKABLE void setArrayValue(int index, const QVariant &value);
    
    // Variable scope - "global" for user-created variables, "element" for auto-created element variables
    QString variableScope() const { return m_variableScope; }
    void setVariableScope(const QString &scope);
    
    // For element variables, the ID of the linked design element
    QString linkedElementId() const { return m_linkedElementId; }
    void setLinkedElementId(const QString &elementId);
    
signals:
    void valueChanged();
    void variableTypeChanged();
    void platformChanged();
    void isArrayChanged();
    void arrayValuesChanged();
    void variableScopeChanged();
    void linkedElementIdChanged();
    
private:
    QVariant m_value;
    QString m_variableType = "string"; // "string" or "number"
    QString m_platform; // Default to empty string (undefined)
    bool m_isArray = false;
    QVariantList m_arrayValues;
    QString m_variableScope = "global"; // "global" or "element"
    QString m_linkedElementId; // For element variables, the ID of the linked design element
};