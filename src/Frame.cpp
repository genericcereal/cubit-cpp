#include "Frame.h"

Frame::Frame(int id, QObject *parent)
    : Element(FrameType, id, parent)
    , m_backgroundColor(Qt::white)
    , m_borderColor(Qt::black)
    , m_borderWidth(1)
    , m_borderRadius(0)
    , m_clipContent(true)
{
    setName(QString("Frame %1").arg(id));
}

void Frame::setBackgroundColor(const QColor &color)
{
    if (m_backgroundColor != color) {
        m_backgroundColor = color;
        emit backgroundColorChanged();
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

void Frame::setClipContent(bool clip)
{
    if (m_clipContent != clip) {
        m_clipContent = clip;
        emit clipContentChanged();
        emit elementChanged();
    }
}