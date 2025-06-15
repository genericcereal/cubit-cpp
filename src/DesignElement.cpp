#include "DesignElement.h"

DesignElement::DesignElement(const QString &id, QObject *parent)
    : CanvasElement(Element::FrameType, id, parent)  // Use FrameType as default, subclasses will override
{
}