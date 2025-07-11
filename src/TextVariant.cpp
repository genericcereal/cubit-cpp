#include "TextVariant.h"

TextVariant::TextVariant(const QString &id, QObject *parent)
    : Text(id, parent)
    , m_instancesAcceptChildren(false)
{
    // Set element type
    elementType = Element::TextVariantType;
    setName("TextVariant");
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