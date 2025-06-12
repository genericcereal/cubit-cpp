#include "Variable.h"

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