#include "ScriptElement.h"

ScriptElement::ScriptElement(const QString &id, QObject *parent)
    : CanvasElement(Element::NodeType, id, parent)  // Use NodeType as default, subclasses will override
{
}