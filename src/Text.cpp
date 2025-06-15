#include "Text.h"

Text::Text(const QString &id, QObject *parent)
    : CanvasElement(TextType, id, parent)
    , m_text("Text")
    , m_color(Qt::black)
{
    setName(QString("Text %1").arg(id.right(4)));  // Use last 4 digits for display
}

void Text::setText(const QString &text)
{
    if (m_text != text) {
        m_text = text;
        emit textChanged();
        emit elementChanged();
    }
}

void Text::setFont(const QFont &font)
{
    if (m_font != font) {
        m_font = font;
        emit fontChanged();
        emit elementChanged();
    }
}

void Text::setColor(const QColor &color)
{
    if (m_color != color) {
        m_color = color;
        emit colorChanged();
        emit elementChanged();
    }
}