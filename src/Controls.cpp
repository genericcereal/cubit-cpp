#include "Controls.h"
#include <QFrame>
#include <QMouseEvent>
#include <QDebug>

Controls::Controls(QWidget *parent) : QWidget(parent), dragMode(None), hasDragged(false), panOffset(0, 0) {
    // Set this widget to handle mouse events
    setAttribute(Qt::WA_TransparentForMouseEvents, false);
    setMouseTracking(true);  // Enable mouse tracking for hover effects
    
    // Create the four control bars
    leftBar = new QFrame(this);
    leftBar->setFixedWidth(4);  // Fixed width, adjustable height
    leftBar->setStyleSheet("background-color: red;");
    leftBar->setAttribute(Qt::WA_TransparentForMouseEvents, false);
    leftBar->show();
    
    rightBar = new QFrame(this);
    rightBar->setFixedWidth(4);  // Fixed width, adjustable height
    rightBar->setStyleSheet("background-color: red;");
    rightBar->setAttribute(Qt::WA_TransparentForMouseEvents, false);
    rightBar->show();
    
    topBar = new QFrame(this);
    topBar->setFixedHeight(4);  // Fixed height, adjustable width
    topBar->setStyleSheet("background-color: red;");
    topBar->setAttribute(Qt::WA_TransparentForMouseEvents, false);
    topBar->show();
    
    bottomBar = new QFrame(this);
    bottomBar->setFixedHeight(4);  // Fixed height, adjustable width
    bottomBar->setStyleSheet("background-color: red;");
    bottomBar->setAttribute(Qt::WA_TransparentForMouseEvents, false);
    bottomBar->show();
    
    // Create the four corner joints
    topLeftJoint = new QFrame(this);
    topLeftJoint->setFixedSize(25, 25);
    topLeftJoint->setStyleSheet("background-color: blue;");
    topLeftJoint->setAttribute(Qt::WA_TransparentForMouseEvents, false);
    topLeftJoint->show();
    
    topRightJoint = new QFrame(this);
    topRightJoint->setFixedSize(25, 25);
    topRightJoint->setStyleSheet("background-color: blue;");
    topRightJoint->setAttribute(Qt::WA_TransparentForMouseEvents, false);
    topRightJoint->show();
    
    bottomLeftJoint = new QFrame(this);
    bottomLeftJoint->setFixedSize(25, 25);
    bottomLeftJoint->setStyleSheet("background-color: blue;");
    bottomLeftJoint->setAttribute(Qt::WA_TransparentForMouseEvents, false);
    bottomLeftJoint->show();
    
    bottomRightJoint = new QFrame(this);
    bottomRightJoint->setFixedSize(25, 25);
    bottomRightJoint->setStyleSheet("background-color: blue;");
    bottomRightJoint->setAttribute(Qt::WA_TransparentForMouseEvents, false);
    bottomRightJoint->show();
    
    // Create the inner rectangle with yellow color and 5% transparency
    innerRect = new QFrame(this);
    innerRect->setStyleSheet("background-color: rgba(255, 255, 0, 0.05);"); // Yellow with 5% opacity (95% transparency)
    innerRect->setAttribute(Qt::WA_TransparentForMouseEvents, false); // Accept mouse events
    innerRect->lower(); // Place it behind other controls
    innerRect->show();
}

void Controls::updateGeometry(const QRect &targetRect) {
    currentRect = targetRect;
    // Position the controls around the target rectangle
    positionControls(targetRect);
    
    // Resize the Controls widget to encompass all control bars
    // Add margin for bars and joints
    int margin = 30; // To account for bar width and joint size
    resize(targetRect.width() + 2 * margin, targetRect.height() + 2 * margin);
    move(targetRect.left() - margin + panOffset.x(), targetRect.top() - margin + panOffset.y());
}

void Controls::positionControls(const QRect &rect) {
    // Update bar sizes to match the rectangle dimensions
    leftBar->resize(leftBar->width(), rect.height());
    rightBar->resize(rightBar->width(), rect.height());
    topBar->resize(rect.width(), topBar->height());
    bottomBar->resize(rect.width(), bottomBar->height());
    
    // Position bars relative to the Controls widget (which has been moved/resized)
    int margin = 30;
    
    // Position left bar - forms the left edge
    leftBar->move(margin - leftBar->width(), margin);
    
    // Position right bar - forms the right edge
    rightBar->move(margin + rect.width(), margin);
    
    // Position top bar - forms the top edge
    topBar->move(margin, margin - topBar->height());
    
    // Position bottom bar - forms the bottom edge
    bottomBar->move(margin, margin + rect.height());
    
    // Position corner joints - with corners at the intersections
    int jointSize = 25;
    
    // Top-left joint - bottom-right corner at intersection of left and top bars
    topLeftJoint->move(margin - jointSize, 
                       margin - jointSize);
    
    // Top-right joint - bottom-left corner at intersection of right and top bars
    topRightJoint->move(margin + rect.width() - 4, 
                        margin - jointSize);
    
    // Bottom-left joint - top-right corner at intersection of left and bottom bars
    bottomLeftJoint->move(margin - jointSize, 
                          margin + rect.height() - 4);
    
    // Bottom-right joint - top-left corner at intersection of right and bottom bars
    bottomRightJoint->move(margin + rect.width() - 4, 
                           margin + rect.height() - 4);
    
    // Position inner rectangle to fit between the bars
    innerRect->move(margin, margin);
    innerRect->resize(rect.width(), rect.height());
}

Controls::DragMode Controls::getBarAt(const QPoint &pos) const {
    // Check if position is within any of the bars
    if (leftBar->geometry().contains(pos)) return LeftBar;
    if (rightBar->geometry().contains(pos)) return RightBar;
    if (topBar->geometry().contains(pos)) return TopBar;
    if (bottomBar->geometry().contains(pos)) return BottomBar;
    if (innerRect->geometry().contains(pos)) return InnerRect;
    return None;
}

void Controls::mousePressEvent(QMouseEvent *event) {
    qDebug() << "Controls::mousePressEvent at" << event->pos();
    if (event->button() == Qt::LeftButton) {
        dragMode = getBarAt(event->pos());
        if (dragMode != None) {
            dragStartPos = event->globalPos();
            dragStartRect = currentRect;
            hasDragged = false;  // Reset drag tracking
            // Set appropriate arrow cursor based on which bar is being dragged
            if (dragMode == LeftBar || dragMode == RightBar) {
                setCursor(Qt::SizeHorCursor);
            } else if (dragMode == TopBar || dragMode == BottomBar) {
                setCursor(Qt::SizeVerCursor);
            } else if (dragMode == InnerRect) {
                setCursor(Qt::SizeAllCursor);
            }
        }
    }
    QWidget::mousePressEvent(event);
}

void Controls::mouseMoveEvent(QMouseEvent *event) {
    if (dragMode != None && (event->buttons() & Qt::LeftButton)) {
        QPoint delta = event->globalPos() - dragStartPos;
        
        // Consider it a drag if mouse moved more than 5 pixels
        if (delta.manhattanLength() > 5) {
            hasDragged = true;
        }
        
        QRect newRect = dragStartRect;
        
        switch (dragMode) {
            case LeftBar:
                newRect.setLeft(dragStartRect.left() + delta.x());
                break;
            case RightBar:
                newRect.setRight(dragStartRect.right() + delta.x());
                break;
            case TopBar:
                newRect.setTop(dragStartRect.top() + delta.y());
                break;
            case BottomBar:
                newRect.setBottom(dragStartRect.bottom() + delta.y());
                break;
            case InnerRect:
                // Move the entire rectangle
                newRect.moveLeft(dragStartRect.left() + delta.x());
                newRect.moveTop(dragStartRect.top() + delta.y());
                break;
            default:
                break;
        }
        
        // Normalize the rectangle if bars have crossed
        // This ensures left is always less than right, and top is always less than bottom
        currentRect = newRect.normalized();
        updateGeometry(currentRect);  // This will resize the Controls widget and reposition bars
        emit rectChanged(currentRect);
    } else {
        // Update cursor based on hover
        DragMode hoverMode = getBarAt(event->pos());
        if (hoverMode == LeftBar || hoverMode == RightBar) {
            setCursor(Qt::SizeHorCursor);
        } else if (hoverMode == TopBar || hoverMode == BottomBar) {
            setCursor(Qt::SizeVerCursor);
        } else if (hoverMode == InnerRect) {
            setCursor(Qt::SizeAllCursor);
        } else {
            setCursor(Qt::ArrowCursor);
        }
    }
    QWidget::mouseMoveEvent(event);
}

void Controls::mouseReleaseEvent(QMouseEvent *event) {
    if (event->button() == Qt::LeftButton) {
        // Check if this was a click on the inner rectangle (not a drag)
        if (dragMode == InnerRect && !hasDragged) {
            emit innerRectClicked(event->globalPos());
        }
        
        dragMode = None;
        hasDragged = false;
        
        // Reset cursor based on current position
        DragMode hoverMode = getBarAt(event->pos());
        if (hoverMode == LeftBar || hoverMode == RightBar) {
            setCursor(Qt::SizeHorCursor);
        } else if (hoverMode == TopBar || hoverMode == BottomBar) {
            setCursor(Qt::SizeVerCursor);
        } else if (hoverMode == InnerRect) {
            setCursor(Qt::SizeAllCursor);
        } else {
            setCursor(Qt::ArrowCursor);
        }
    }
    QWidget::mouseReleaseEvent(event);
}