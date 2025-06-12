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

void Element::setX(qreal x)
{
    if (!qFuzzyCompare(canvasPosition.x(), x)) {
        canvasPosition.setX(x);
        emit xChanged();
        emit geometryChanged();
        emit elementChanged();
    }
}

void Element::setY(qreal y)
{
    if (!qFuzzyCompare(canvasPosition.y(), y)) {
        canvasPosition.setY(y);
        emit yChanged();
        emit geometryChanged();
        emit elementChanged();
    }
}

void Element::setWidth(qreal w)
{
    if (!qFuzzyCompare(canvasSize.width(), w)) {
        canvasSize.setWidth(w);
        emit widthChanged();
        emit geometryChanged();
        emit elementChanged();
    }
}

void Element::setHeight(qreal h)
{
    if (!qFuzzyCompare(canvasSize.height(), h)) {
        canvasSize.setHeight(h);
        emit heightChanged();
        emit geometryChanged();
        emit elementChanged();
    }
}

void Element::setRect(const QRectF &rect)
{
    bool posChanged = canvasPosition != rect.topLeft();
    bool sizeChanged = canvasSize != rect.size();
    
    if (posChanged || sizeChanged) {
        canvasPosition = rect.topLeft();
        canvasSize = rect.size();
        
        if (posChanged) {
            emit xChanged();
            emit yChanged();
        }
        if (sizeChanged) {
            emit widthChanged();
            emit heightChanged();
        }
        emit geometryChanged();
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