#include "ClientRect.h"
#include "Canvas.h"
#include "Controls.h"
#include "Frame.h"
#include <QMouseEvent>

ClientRect::ClientRect(int associatedElementId, Canvas *canvas, QWidget *parent) 
    : QFrame(parent), elementId(associatedElementId), canvasRef(canvas) {
    // Don't set a fixed size - allow dynamic sizing
    resize(400, 400);  // Default size, but can be changed
    setFrameStyle(QFrame::NoFrame);
    setLineWidth(0);
    setStyleSheet("QFrame { background-color: rgba(173, 216, 230, 0.7); border: none; }");
    setMouseTracking(true);  // Enable mouse tracking for hover events
}

void ClientRect::mousePressEvent(QMouseEvent *event) {
    if (event->button() == Qt::LeftButton && canvasRef) {
        // Check if Shift is held for multi-selection
        bool addToSelection = (event->modifiers() & Qt::ShiftModifier);
        
        // Select the associated element
        canvasRef->selectElement(QString::number(elementId), addToSelection);
        
        // Only start drag operation if shift is NOT held
        if (!addToSelection) {
            canvasRef->startControlDrag(event->globalPos());
        }
        
        event->accept();
    }
    QFrame::mousePressEvent(event);
}

void ClientRect::mouseMoveEvent(QMouseEvent *event) {
    if (event->buttons() & Qt::LeftButton && canvasRef) {
        // Only forward drag updates if shift is NOT held
        if (!(event->modifiers() & Qt::ShiftModifier)) {
            canvasRef->updateControlDrag(event->globalPos());
        }
        event->accept();
    }
    QFrame::mouseMoveEvent(event);
}

void ClientRect::mouseReleaseEvent(QMouseEvent *event) {
    if (event->button() == Qt::LeftButton && canvasRef) {
        // Only end drag operation if shift is NOT held
        if (!(event->modifiers() & Qt::ShiftModifier)) {
            canvasRef->endControlDrag(event->globalPos());
        }
        event->accept();
    }
    QFrame::mouseReleaseEvent(event);
}

void ClientRect::enterEvent(QEvent *event) {
    if (canvasRef) {
        // Check if this element is not already selected
        bool isSelected = false;
        for (const auto& frame : canvasRef->getSelectedFrames()) {
            if (frame->getId() == elementId) {
                isSelected = true;
                break;
            }
        }
        
        // Only set as hovered if not selected
        if (!isSelected) {
            canvasRef->setHoveredElement(QString::number(elementId));
        }
    }
    QFrame::enterEvent(event);
}

void ClientRect::leaveEvent(QEvent *event) {
    // Don't clear on leave - let Canvas handle this globally
    // This allows smooth transitions between adjacent ClientRects
    QFrame::leaveEvent(event);
}