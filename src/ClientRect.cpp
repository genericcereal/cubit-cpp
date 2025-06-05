#include "ClientRect.h"
#include "CanvasBase.h"
#include <QMouseEvent>

ClientRect::ClientRect(int associatedElementId, CanvasBase *canvas, QWidget *parent) 
    : QFrame(parent), elementId(associatedElementId), canvasRef(canvas) {
    // Don't set a fixed size - allow dynamic sizing
    resize(400, 400);  // Default size, but can be changed
    setFrameStyle(QFrame::Box);
    setLineWidth(1);
    setStyleSheet("QFrame { background-color: rgba(173, 216, 230, 0.7); border: 2px solid #4682B4; }");
}

void ClientRect::mousePressEvent(QMouseEvent *event) {
    if (event->button() == Qt::LeftButton && canvasRef) {
        // Check if Shift is held for multi-selection
        bool addToSelection = (event->modifiers() & Qt::ShiftModifier);
        
        // Select the associated element
        canvasRef->selectElement(QString::number(elementId), addToSelection);
        event->accept();
    }
    QFrame::mousePressEvent(event);
}