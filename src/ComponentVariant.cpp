#include "ComponentVariant.h"
#include "ComponentInstance.h"

ComponentVariant::ComponentVariant(const QString& id)
    : m_id(id)
{
}

void ComponentVariant::addInstance(ComponentInstance* instance)
{
    if (!m_instances.contains(instance)) {
        m_instances.append(instance);
        instance->setMasterVariant(this);
    }
}

void ComponentVariant::removeInstance(ComponentInstance* instance)
{
    if (m_instances.removeOne(instance)) {
        if (instance->masterVariant() == this) {
            instance->setMasterVariant(nullptr);
        }
    }
}