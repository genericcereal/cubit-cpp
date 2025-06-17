#pragma once
#include "Element.h"
#include <QVariant>
#include <QVariantList>

class Variable : public Element {
    Q_OBJECT
    Q_PROPERTY(QVariant value READ value WRITE setValue NOTIFY valueChanged)
    Q_PROPERTY(bool isArray READ isArray WRITE setIsArray NOTIFY isArrayChanged)
    Q_PROPERTY(QVariantList arrayValues READ arrayValues NOTIFY arrayValuesChanged)
    Q_PROPERTY(int arrayLength READ arrayLength NOTIFY arrayValuesChanged)
    
public:
    explicit Variable(const QString &id, QObject *parent = nullptr);
    
    // Single value access
    QVariant value() const { return m_value; }
    void setValue(const QVariant &value);
    
    // Array properties
    bool isArray() const { return m_isArray; }
    void setIsArray(bool isArray);
    
    // Array operations
    QVariantList arrayValues() const { return m_arrayValues; }
    int arrayLength() const { return m_arrayValues.length(); }
    
    Q_INVOKABLE void addArrayValue(const QVariant &value);
    Q_INVOKABLE void removeArrayValue(int index);
    Q_INVOKABLE void setArrayValue(int index, const QVariant &value);
    
signals:
    void valueChanged();
    void isArrayChanged();
    void arrayValuesChanged();
    
private:
    QVariant m_value;
    bool m_isArray = false;
    QVariantList m_arrayValues;
};