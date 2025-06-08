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
    // No stylesheet needed - we're doing custom painting in paintEvent
    
    setObjectName(frameName);
    
    // IMPORTANT: Frames should not handle mouse events directly
    // Mouse events are handled by the associated ClientRect
    setAttribute(Qt::WA_TransparentForMouseEvents, true);
    
    // Apply initial overflow clipping
    updateOverflowClipping();
}

void Frame::updateParentVisualState() {
    // Visual state is now handled in paintEvent, just trigger a repaint
    update();
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
    // In Qt6, WA_ContentsPropagated is removed. We'll handle clipping in paintEvent
    // by setting up proper clipping regions
    update();  // Trigger a repaint
}

void Frame::paintEvent(QPaintEvent *) {
    // Custom painting with antialiasing disabled
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing, false);  // Disable antialiasing for crisp edges
    
    // Fill the background
    QColor bgColor;
    if (hasParent()) {
        bgColor = QColor(0, 255, 0, 51);  // Green with 0.2 alpha (51/255)
    } else {
        bgColor = QColor(128, 0, 128, 51);  // Purple with 0.2 alpha (51/255)
    }
    painter.fillRect(rect(), bgColor);
    
    // If overflow is hidden, we need to clip children
    if (overflow == "hidden") {
        // Create a clip region for children using QRectF for precision
        QRectF clipRectF(rect());
        
        // Apply clipping to each child widget
        for (QObject *child : children()) {
            QWidget *childWidget = qobject_cast<QWidget*>(child);
            if (childWidget && childWidget->isWidgetType()) {
                // Calculate the visible region of the child using QRectF
                QRectF childRectF(childWidget->geometry());
                QRectF visibleRectF = childRectF.intersected(clipRectF);
                
                if (visibleRectF.isEmpty()) {
                    // Child is completely outside parent bounds
                    childWidget->setVisible(false);
                } else if (visibleRectF != childRectF) {
                    // Child is partially outside parent bounds
                    // Create a mask to clip the child
                    QRect visibleRect = visibleRectF.toRect();
                    QRect childRect = childRectF.toRect();
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