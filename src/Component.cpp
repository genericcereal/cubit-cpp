#include "Component.h"
#include "ComponentVariant.h"
#include "Scripts.h"

Component::Component(const QString &id, QObject *parent)
    : Element(ComponentType, id, parent)
    , m_scripts(std::make_unique<Scripts>(this))
{
    // Set the name to "Component" + last 4 digits of ID
    QString last4 = id.right(4);
    setName("Component" + last4);
}

Component::~Component()
{
}

void Component::addVariant(ComponentVariant* variant)
{
    if (variant && !m_variants.contains(variant)) {
        m_variants.append(variant);
        emit variantsChanged();
    }
}

void Component::removeVariant(ComponentVariant* variant)
{
    if (m_variants.removeOne(variant)) {
        emit variantsChanged();
    }
}

void Component::clearVariants()
{
    if (!m_variants.isEmpty()) {
        m_variants.clear();
        emit variantsChanged();
    }
}

Scripts* Component::scripts() const
{
    return m_scripts.get();
}