#include "Element.h"

Element::Element(ElementType type, int id, QWidget *parent) 
    : QFrame(parent), elementType(type), elementId(id), canvasPosition(0, 0) {
}

QString Element::getTypeName() const {
    switch (elementType) {
        case FrameType:
            return "Frame";
        case TextType:
            return "Text";
        case VariableType:
            return "Variable";
        default:
            return "Unknown";
    }
}

void Element::updateVisualPosition(const QPoint &panOffset) {
    // Update the widget's actual position based on canvas position + pan offset
    move(canvasPosition + panOffset);
}