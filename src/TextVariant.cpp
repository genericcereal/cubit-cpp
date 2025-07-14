#include "TextVariant.h"
#include "TextComponentInstance.h"
#include "Scripts.h"

TextVariant::TextVariant(const QString &id, QObject *parent)
    : Text(id, parent)
    , ComponentVariant(id)
    , m_instancesAcceptChildren(false)
{
    // Set element type
    elementType = Element::TextVariantType;
    setName("TextVariant");
    
    // Prevent scripts initialization by clearing the pointer
    // Scripts were initialized in DesignElement constructor, but variants shouldn't have scripts
    m_scripts.reset();
}

TextVariant::~TextVariant()
{
}

void TextVariant::setInstancesAcceptChildren(bool accept)
{
    if (m_instancesAcceptChildren != accept) {
        m_instancesAcceptChildren = accept;
        emit instancesAcceptChildrenChanged();
    }
}

void TextVariant::setEditableProperties(const QStringList& properties)
{
    if (m_editableProperties != properties) {
        m_editableProperties = properties;
        emit editablePropertiesChanged();
    }
}

void TextVariant::applyToInstance(ComponentInstance* instance)
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