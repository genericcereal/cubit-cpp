#include "Variable.h"

Variable::Variable(int id, QObject *parent)
    : Element(VariableType, id, parent)
{
    setName(QString("Variable %1").arg(id));
}

void Variable::setValue(const QVariant &value)
{
    if (m_value != value) {
        m_value = value;
        emit valueChanged();
        emit elementChanged();
    }
}