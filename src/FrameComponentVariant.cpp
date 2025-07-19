#include "FrameComponentVariant.h"
#include "FrameComponentInstance.h"
#include "Scripts.h"

FrameComponentVariant::FrameComponentVariant(const QString &id, QObject *parent)
    : Frame(id, parent)
    , ComponentVariant(id)
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
    ComponentVariant::setEditableProperties(QStringList{
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
    });
    
    // Frame variants typically accept children by default
    ComponentVariant::setInstancesAcceptChildren(true);
}

FrameComponentVariant::~FrameComponentVariant()
{
}

void FrameComponentVariant::setInstancesAcceptChildren(bool accept)
{
    ComponentVariant::setInstancesAcceptChildren(accept);
    emit instancesAcceptChildrenChanged();
}

void FrameComponentVariant::setEditableProperties(const QStringList& properties)
{
    ComponentVariant::setEditableProperties(properties);
    emit editablePropertiesChanged();
}

void FrameComponentVariant::setVariantName(const QString& name)
{
    ComponentVariant::setVariantName(name);
    emit variantNameChanged();
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

ComponentVariant* FrameComponentVariant::clone(const QString& newId) const
{
    FrameComponentVariant* newVariant = new FrameComponentVariant(newId, parent());
    
    // Copy geometry
    newVariant->setRect(rect());
    
    // Copy all Frame properties
    newVariant->setFill(fill());
    newVariant->setBorderColor(borderColor());
    newVariant->setBorderWidth(borderWidth());
    newVariant->setBorderRadius(borderRadius());
    newVariant->setOverflow(overflow());
    newVariant->setFlex(flex());
    newVariant->setOrientation(orientation());
    newVariant->setGap(gap());
    newVariant->setJustify(justify());
    newVariant->setAlign(align());
    newVariant->setWidthType(widthType());
    newVariant->setHeightType(heightType());
    newVariant->setRole(role());
    newVariant->setPlatform(platform());
    newVariant->setPosition(position());
    
    // Copy anchor properties
    newVariant->setLeft(left());
    newVariant->setRight(right());
    newVariant->setTop(top());
    newVariant->setBottom(bottom());
    newVariant->setLeftAnchored(leftAnchored());
    newVariant->setRightAnchored(rightAnchored());
    newVariant->setTopAnchored(topAnchored());
    newVariant->setBottomAnchored(bottomAnchored());
    
    // Copy variant-specific properties
    newVariant->setInstancesAcceptChildren(instancesAcceptChildren());
    newVariant->setEditableProperties(editableProperties());
    
    return newVariant;
}