#include "Frame.h"
#include <QDebug>
#include <QPainter>
#include <QPaintEvent>
#include <QResizeEvent>

Frame::Frame(int id, QWidget *parent) : Element(ElementType::FrameType, id, parent) {
    frameName = QString("Frame_%1").arg(elementId, 2, 10, QChar('0'));
    overflow = "hidden";  // Default overflow value
    
    // Don't set a fixed size - allow dynamic sizing
    resize(400, 400);  // Default size, but can be changed
    setFrameStyle(QFrame::NoFrame);
    setLineWidth(0);
    setStyleSheet("QFrame { background-color: rgba(128, 0, 128, 0.2); border: none; }");
    
    setObjectName(frameName);
    
    // IMPORTANT: Frames should not handle mouse events directly
    // Mouse events are handled by the associated ClientRect
    setAttribute(Qt::WA_TransparentForMouseEvents, true);
    
    // Apply initial overflow clipping
    updateOverflowClipping();
}

void Frame::updateParentVisualState() {
    if (hasParent()) {
        // Green background for elements with parents
        setStyleSheet("QFrame { background-color: rgba(0, 255, 0, 0.2); border: none; }");
    } else {
        // Original purple background for elements without parents
        setStyleSheet("QFrame { background-color: rgba(128, 0, 128, 0.2); border: none; }");
    }
}

void Frame::setOverflow(const QString& value) {
    if (value == "visible" || value == "hidden") {
        overflow = value;
        updateOverflowClipping();
        
        // Notify the canvas to update clipping for our children
        emit overflowChanged();
    }
}

void Frame::updateOverflowClipping() {
    // Qt's WA_PaintOnScreen and custom painting doesn't clip children automatically
    // We need to use a different approach - setting a mask on child widgets
    if (overflow == "hidden") {
        // We'll handle clipping by using the QWidget clipping feature
        // This requires the widget to clip its children during painting
        setAttribute(Qt::WA_ContentsPropagated, false);
    } else {
        // For visible overflow, allow normal painting
        setAttribute(Qt::WA_ContentsPropagated, true);
    }
    update();  // Trigger a repaint
}

void Frame::paintEvent(QPaintEvent *event) {
    // Call base class to handle frame painting
    QFrame::paintEvent(event);
    
    // If overflow is hidden, we need to clip children
    if (overflow == "hidden") {
        // Create a clip region for children
        QRegion clipRegion(rect());
        
        // Apply clipping to each child widget
        for (QObject *child : children()) {
            QWidget *childWidget = qobject_cast<QWidget*>(child);
            if (childWidget && childWidget->isWidgetType()) {
                // Calculate the visible region of the child
                QRect childRect = childWidget->geometry();
                QRect visibleRect = childRect.intersected(rect());
                
                if (visibleRect.isEmpty()) {
                    // Child is completely outside parent bounds
                    childWidget->setVisible(false);
                } else if (visibleRect != childRect) {
                    // Child is partially outside parent bounds
                    // Create a mask to clip the child
                    QRegion childMask = visibleRect.translated(-childRect.topLeft());
                    childWidget->setMask(childMask);
                    childWidget->setVisible(true);
                } else {
                    // Child is completely inside parent bounds
                    childWidget->clearMask();
                    childWidget->setVisible(true);
                }
            }
        }
    } else {
        // Clear masks from all children when overflow is visible
        for (QObject *child : children()) {
            QWidget *childWidget = qobject_cast<QWidget*>(child);
            if (childWidget && childWidget->isWidgetType()) {
                childWidget->clearMask();
                childWidget->setVisible(true);
            }
        }
    }
}

void Frame::resizeEvent(QResizeEvent *event) {
    QFrame::resizeEvent(event);
    
    // When the frame is resized, update clipping for children
    if (overflow == "hidden") {
        updateOverflowClipping();
    }
}