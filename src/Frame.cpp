#include "Frame.h"

Frame::Frame(const QString &id, QObject *parent)
    : DesignElement(id, parent)
    , m_fillColor(LightBlue)
    , m_borderColor(Qt::black)
    , m_borderWidth(1)
    , m_borderRadius(0)
    , m_overflow(Hidden)
{
    // Set element type
    elementType = Element::FrameType;
    
    setName(QString("Frame %1").arg(id.right(4)));  // Use last 4 digits for display
}

QColor Frame::fill() const
{
    switch (m_fillColor) {
        case LightBlue:
            return QColor(173, 216, 230); // Light blue
        case DarkBlue:
            return QColor(0, 0, 139); // Dark blue
        case Green:
            return QColor(0, 128, 0); // Green
        case Red:
            return QColor(255, 0, 0); // Red
        default:
            return QColor(173, 216, 230); // Default to light blue
    }
}

void Frame::setFillColor(FillColor color)
{
    if (m_fillColor != color) {
        m_fillColor = color;
        emit fillColorChanged();
        emit elementChanged();
    }
}

void Frame::setBorderColor(const QColor &color)
{
    if (m_borderColor != color) {
        m_borderColor = color;
        emit borderColorChanged();
        emit elementChanged();
    }
}

void Frame::setBorderWidth(int width)
{
    if (m_borderWidth != width) {
        m_borderWidth = width;
        emit borderWidthChanged();
        emit elementChanged();
    }
}

void Frame::setBorderRadius(int radius)
{
    if (m_borderRadius != radius) {
        m_borderRadius = radius;
        emit borderRadiusChanged();
        emit elementChanged();
    }
}

void Frame::setOverflow(OverflowMode mode)
{
    if (m_overflow != mode) {
        m_overflow = mode;
        emit overflowChanged();
        emit elementChanged();
    }
}