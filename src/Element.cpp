#include "Element.h"

Element::Element(ElementType type, int id, QWidget *parent) 
    : QFrame(parent), elementType(type), elementId(id) {
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