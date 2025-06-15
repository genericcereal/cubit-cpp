#include "Element.h"

Element::Element(ElementType type, const QString &id, QObject *parent)
    : QObject(parent)
    , elementType(type)
    , elementId(id)
    , parentElementId()
    , selected(false)
{
}

QString Element::getTypeName() const
{
    switch (elementType) {
        case FrameType:
            return "Frame";
        case TextType:
            return "Text";
        case VariableType:
            return "Variable";
        case HtmlType:
            return "Html";
        case NodeType:
            return "Node";
        case EdgeType:
            return "Edge";
        default:
            return "Unknown";
    }
}

void Element::setName(const QString &newName)
{
    if (name != newName) {
        name = newName;
        emit nameChanged();
        emit elementChanged();
    }
}

void Element::setParentElementId(const QString &parentId)
{
    if (parentElementId != parentId) {
        parentElementId = parentId;
        emit parentIdChanged();
        emit elementChanged();
    }
}

void Element::setSelected(bool sel)
{
    if (selected != sel) {
        selected = sel;
        emit selectedChanged();
    }
}