#include "ComponentVariant.h"
#include "ComponentInstance.h"

ComponentVariant::ComponentVariant(const QString& id)
    : m_id(id)
{
}

QString ComponentVariant::id() const
{
    return m_id;
}

QString ComponentVariant::variantName() const
{
    return m_variantName;
}

void ComponentVariant::setVariantName(const QString& name)
{
    m_variantName = name;
}

const QList<ComponentInstance*>& ComponentVariant::instances() const
{
    return m_instances;
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