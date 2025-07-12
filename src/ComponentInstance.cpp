#include "ComponentInstance.h"
#include "ComponentVariant.h"

ComponentInstance::ComponentInstance(const QString& elementId)
    : m_elementId(elementId)
{
}

QString ComponentInstance::elementId() const
{
    return m_elementId;
}

ComponentVariant* ComponentInstance::masterVariant() const
{
    return m_masterVariant;
}

void ComponentInstance::setMasterVariant(ComponentVariant* variant)
{
    m_masterVariant = variant;
}