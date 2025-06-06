#include "Frame.h"

Frame::Frame(int id, QWidget *parent) : Element(ElementType::FrameType, id, parent), frameId(id) {
    frameName = QString("Frame_%1").arg(frameId, 2, 10, QChar('0'));
    // Don't set a fixed size - allow dynamic sizing
    resize(400, 400);  // Default size, but can be changed
    setFrameStyle(QFrame::NoFrame);
    setLineWidth(0);
    setStyleSheet("QFrame { background-color: rgba(128, 0, 128, 0.2); border: none; }");
    
    setObjectName(frameName);
    
    // IMPORTANT: Frames should not handle mouse events directly
    // Mouse events are handled by the associated ClientRect
    setAttribute(Qt::WA_TransparentForMouseEvents, true);
}