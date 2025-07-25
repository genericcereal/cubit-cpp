#include "TextComponentVariant.h"
#include "TextComponentInstance.h"
#include "Scripts.h"

TextComponentVariant::TextComponentVariant(const QString &id, QObject *parent)
    : Text(id, parent)
    , ComponentVariant(id)
{
    // Set element type
    elementType = Element::TextVariantType;
    setName("TextComponentVariant");
    
    // Prevent scripts initialization by clearing the pointer
    // Scripts were initialized in DesignElement constructor, but variants shouldn't have scripts
    m_scripts.reset();
    
    // Set a more appropriate name
    setName(QString("Variant %1").arg(id.right(4)));
    setVariantName(QString("Variant %1").arg(id.right(4)));
    
    // Initialize default editable properties for Text elements
    ComponentVariant::setEditableProperties(QStringList{
        "content",
        "font",
        "color",
        "position"
    });
    
    // Text variants typically don't accept children
    ComponentVariant::setInstancesAcceptChildren(false);
}

TextComponentVariant::~TextComponentVariant()
{
}

void TextComponentVariant::setInstancesAcceptChildren(bool accept)
{
    ComponentVariant::setInstancesAcceptChildren(accept);
    emit instancesAcceptChildrenChanged();
}

void TextComponentVariant::setEditableProperties(const QStringList& properties)
{
    ComponentVariant::setEditableProperties(properties);
    emit editablePropertiesChanged();
}

void TextComponentVariant::setVariantName(const QString& name)
{
    ComponentVariant::setVariantName(name);
    emit variantNameChanged();
}

void TextComponentVariant::applyToInstance(ComponentInstance* instance)
{
    if (TextComponentInstance* textInstance = dynamic_cast<TextComponentInstance*>(instance)) {
        // Apply text-specific properties to the instance
        textInstance->setContent(content());
        textInstance->setFont(font());
        textInstance->setColor(color());
        textInstance->setPosition(position());
        textInstance->setRect(QRectF(x(), y(), width(), height()));
    }
}

ComponentVariant* TextComponentVariant::clone(const QString& newId) const
{
    TextComponentVariant* newVariant = new TextComponentVariant(newId, parent());
    
    // Copy geometry
    newVariant->setRect(rect());
    
    // Copy all Text properties
    newVariant->setContent(content());
    newVariant->setFont(font());
    newVariant->setColor(color());
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