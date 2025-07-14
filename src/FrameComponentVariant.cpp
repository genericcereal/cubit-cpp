#include "FrameComponentVariant.h"
#include "FrameComponentInstance.h"
#include "Scripts.h"

FrameComponentVariant::FrameComponentVariant(const QString &id, QObject *parent)
    : Frame(id, parent)
    , ComponentVariant(id)
    , m_instancesAcceptChildren(true)  // Default to true
{
    // Override the element type set by Frame
    elementType = Element::FrameComponentVariantType;
    
    // Prevent scripts initialization by clearing the pointer
    // Scripts were initialized in DesignElement constructor, but variants shouldn't have scripts
    m_scripts.reset();
    
    // Set a more appropriate name
    setName(QString("Variant %1").arg(id.right(4)));
    setVariantName(QString("Variant %1").arg(id.right(4)));
    
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

void FrameComponentVariant::applyToInstance(ComponentInstance* instance)
{
    if (FrameComponentInstance* frameInstance = dynamic_cast<FrameComponentInstance*>(instance)) {
        // Apply frame-specific properties to the instance
        frameInstance->setFill(fill());
        frameInstance->setBorderColor(borderColor());
        frameInstance->setBorderWidth(borderWidth());
        frameInstance->setBorderRadius(borderRadius());
        frameInstance->setOverflow(overflow());
        frameInstance->setFlex(flex());
        frameInstance->setOrientation(orientation());
        frameInstance->setGap(gap());
        frameInstance->setPosition(position());
        frameInstance->setJustify(justify());
        frameInstance->setAlign(align());
        frameInstance->setWidthType(widthType());
        frameInstance->setHeightType(heightType());
        frameInstance->setRect(QRectF(x(), y(), width(), height()));
    }
}