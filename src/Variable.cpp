#include "Variable.h"
#include <QDebug>

Variable::Variable(const QString &id, QObject *parent)
    : Element(VariableType, id, parent)
{
    setName(QString("Variable %1").arg(id.right(4)));  // Use last 4 digits for display
}

void Variable::setValue(const QVariant &value)
{
    if (m_value != value) {
        m_value = value;
        emit valueChanged();
        emit elementChanged();
    }
}

void Variable::setVariableType(const QString &type)
{
    if (type == "string" || type == "number" || type == "array") {
        if (m_variableType != type) {
            m_variableType = type;
            emit variableTypeChanged();
            emit elementChanged();
        }
    } else {
        qWarning() << "Invalid variable type:" << type << ". Must be 'string', 'number', or 'array'.";
    }
}

void Variable::setPlatform(const QString &platform)
{
    if (m_platform != platform) {
        m_platform = platform;
        emit platformChanged();
        emit elementChanged();
    }
}

void Variable::setIsArray(bool isArray)
{
    if (m_isArray != isArray) {
        m_isArray = isArray;
        
        if (isArray && m_arrayValues.isEmpty()) {
            // Initialize with one empty value when switching to array
            m_arrayValues.append(QString(""));
            emit arrayValuesChanged();
        }
        
        emit isArrayChanged();
        emit elementChanged();
    }
}

void Variable::addArrayValue(const QVariant &value)
{
    if (!m_isArray) return;
    
    m_arrayValues.append(value);
    emit arrayValuesChanged();
    emit elementChanged();
}

void Variable::removeArrayValue(int index)
{
    if (!m_isArray) return;
    
    // Ensure we maintain at least one element
    if (m_arrayValues.length() > 1 && index >= 0 && index < m_arrayValues.length()) {
        m_arrayValues.removeAt(index);
        emit arrayValuesChanged();
        emit elementChanged();
    }
}

void Variable::setArrayValue(int index, const QVariant &value)
{
    if (!m_isArray) return;
    
    if (index >= 0 && index < m_arrayValues.length()) {
        if (m_arrayValues[index] != value) {
            m_arrayValues[index] = value;
            emit arrayValuesChanged();
            emit elementChanged();
        }
    }
}

void Variable::setVariableScope(const QString &scope)
{
    if (scope == "global" || scope == "element") {
        if (m_variableScope != scope) {
            m_variableScope = scope;
            emit variableScopeChanged();
            emit elementChanged();
        }
    } else {
        qWarning() << "Invalid variable scope:" << scope << ". Must be 'global' or 'element'.";
    }
}

void Variable::setLinkedElementId(const QString &elementId)
{
    if (m_linkedElementId != elementId) {
        m_linkedElementId = elementId;
        emit linkedElementIdChanged();
        emit elementChanged();
    }
}