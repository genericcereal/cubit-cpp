#include "Text.h"
#include <QDebug>

Text::Text(const QString &id, QObject *parent)
    : DesignElement(id, parent)
    , m_content("Text")
    , m_color(Qt::black)
{
    // Set element type
    elementType = Element::TextType;
    
    setName(QString("Text %1").arg(id.right(4)));  // Use last 4 digits for display
}

void Text::setContent(const QString &content)
{
    if (m_content != content) {
        m_content = content;
        emit contentChanged();
        emit elementChanged();
    }
}

void Text::setFont(const QFont &font)
{
    // Force emit fontChanged even if Qt thinks fonts are equal
    // This ensures pixel size changes are propagated
    m_font = font;
    emit fontChanged();
    emit elementChanged();
}

void Text::setColor(const QColor &color)
{
    if (m_color != color) {
        m_color = color;
        emit colorChanged();
        emit elementChanged();
    }
}

void Text::setIsEditing(bool editing)
{
    if (m_isEditing != editing) {
        m_isEditing = editing;
        emit isEditingChanged();
        emit elementChanged();
    }
}

void Text::setPosition(PositionType position)
{
    if (m_position != position) {
        m_position = position;
        emit positionChanged();
        emit elementChanged();
    }
}

void Text::exitEditMode()
{
    // Simply exit edit mode - the TextField will handle saving via onIsEditingChanged
    setIsEditing(false);
}