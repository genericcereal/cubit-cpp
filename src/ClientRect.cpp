#include "ClientRect.h"

ClientRect::ClientRect(QWidget *parent) : QFrame(parent) {
    // Don't set a fixed size - allow dynamic sizing
    resize(400, 400);  // Default size, but can be changed
    setFrameStyle(QFrame::Box);
    setLineWidth(1);
    setStyleSheet("QFrame { background-color: rgba(173, 216, 230, 0.7); border: 2px solid #4682B4; }");
}