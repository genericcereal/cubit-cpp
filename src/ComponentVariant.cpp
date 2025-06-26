#include "ComponentVariant.h"

ComponentVariant::ComponentVariant(const QString &id, QObject *parent)
    : Frame(id, parent)
    , m_instancesAcceptChildren(true)  // Default to true
{
    // Override the element type set by Frame
    elementType = Element::ComponentVariantType;
    
    // Set a more appropriate name
    setName(QString("Variant %1").arg(id.right(4)));
}

ComponentVariant::~ComponentVariant()
{
}

void ComponentVariant::setInstancesAcceptChildren(bool accept)
{
    if (m_instancesAcceptChildren != accept) {
        m_instancesAcceptChildren = accept;
        emit instancesAcceptChildrenChanged();
        emit elementChanged();
    }
}