#include "FrameComponentVariant.h"

FrameComponentVariant::FrameComponentVariant(const QString &id, QObject *parent)
    : Frame(id, parent)
    , m_instancesAcceptChildren(true)  // Default to true
{
    // Override the element type set by Frame
    elementType = Element::FrameComponentVariantType;
    
    // Set a more appropriate name
    setName(QString("Variant %1").arg(id.right(4)));
    
    // Initialize default editable properties to include all Frame properties
    m_editableProperties = QStringList{
        // Frame appearance properties
        "fill",
        "colorFormat",
        "borderColor",
        "borderWidth",
        "borderRadius",
        "overflow",
        "acceptsChildren",
        // Flex layout properties
        "flex",
        "orientation",
        "gap",
        "position",
        "justify",
        "align",
        "widthType",
        "heightType",
        // Other Frame properties
        "controlled",
        "role",
        "platform"
    };
}

FrameComponentVariant::~FrameComponentVariant()
{
}

void FrameComponentVariant::setInstancesAcceptChildren(bool accept)
{
    if (m_instancesAcceptChildren != accept) {
        m_instancesAcceptChildren = accept;
        emit instancesAcceptChildrenChanged();
        emit elementChanged();
    }
}

void FrameComponentVariant::setEditableProperties(const QStringList& properties)
{
    if (m_editableProperties != properties) {
        m_editableProperties = properties;
        emit editablePropertiesChanged();
        emit elementChanged();
    }
}