#include "WebTextInput.h"

WebTextInput::WebTextInput(const QString &id, QObject *parent)
    : DesignElement(id, parent)
{
    // Set element type
    elementType = Element::WebTextInputType;
    
    setName(QString("Input %1").arg(id.right(4)));  // Use last 4 digits for display
}

void WebTextInput::setPlaceholder(const QString &placeholder)
{
    if (m_placeholder != placeholder) {
        m_placeholder = placeholder;
        emit placeholderChanged();
    }
}

void WebTextInput::setValue(const QString &value)
{
    if (m_value != value) {
        m_value = value;
        emit valueChanged();
    }
}

void WebTextInput::setBorderColor(const QColor &color)
{
    if (m_borderColor != color) {
        m_borderColor = color;
        emit borderColorChanged();
    }
}

void WebTextInput::setBorderWidth(qreal width)
{
    if (!qFuzzyCompare(m_borderWidth, width)) {
        m_borderWidth = width;
        emit borderWidthChanged();
    }
}

void WebTextInput::setBorderRadius(qreal radius)
{
    if (!qFuzzyCompare(m_borderRadius, radius)) {
        m_borderRadius = radius;
        emit borderRadiusChanged();
    }
}

void WebTextInput::setIsEditing(bool editing)
{
    if (m_isEditing != editing) {
        m_isEditing = editing;
        emit isEditingChanged();
    }
}