#include "Element.h"
#include <QResizeEvent>
#include <QDebug>

Element::Element(ElementType type, int id, QWidget *parent) 
    : QFrame(parent), elementType(type), elementId(id), canvasPosition(0, 0), canvasSize(0, 0) {
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

void Element::updateVisualPosition(const QPoint &panOffset, qreal zoomScale) {
    // Update the widget's actual position and size based on canvas position, pan offset, and zoom
    QPoint scaledPos = canvasPosition * zoomScale + panOffset;
    QSize scaledSize = canvasSize * zoomScale;
    
    // Debug output commented out to reduce console spam
    // qDebug() << "updateVisualPosition - canvasSize:" << canvasSize << "scaledSize:" << scaledSize << "zoomScale:" << zoomScale;
    
    setGeometry(QRect(scaledPos, scaledSize));
}

void Element::resizeEvent(QResizeEvent *event) {
    // Canvas size is set explicitly through setCanvasSize
    // We don't update it here to avoid corruption from zoomed sizes
    QFrame::resizeEvent(event);
}