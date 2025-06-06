#include "Controls.h"
#include <QFrame>
#include <QMouseEvent>
#include <QDebug>
#include <QTimer>
#include <QApplication>

Controls::Controls(QWidget *parent) : QWidget(parent), dragMode(Controls::None), hasDragged(false), panOffset(0, 0) {
    // Set this widget to handle mouse events
    setAttribute(Qt::WA_TransparentForMouseEvents, false);
    setMouseTracking(true);  // Enable mouse tracking for hover effects
    
    // Initialize single-click timer for double-click detection
    singleClickTimer = new QTimer(this);
    singleClickTimer->setSingleShot(true);
    singleClickTimer->setInterval(QApplication::doubleClickInterval());
    connect(singleClickTimer, &QTimer::timeout, [this]() {
        // Timer expired, this was a single click
        emit innerRectClicked(pendingClickPos);
    });
    
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

void Controls::startDragMode(DragMode mode, const QPoint &globalStartPos) {
    dragMode = mode;
    dragStartPos = globalStartPos;
    dragStartRect = currentRect;
    hasDragged = false;
    
    // Set appropriate cursor based on drag mode
    if (dragMode == LeftBar || dragMode == RightBar) {
        setCursor(Qt::SizeHorCursor);
    } else if (dragMode == TopBar || dragMode == BottomBar) {
        setCursor(Qt::SizeVerCursor);
    } else if (dragMode == TopLeftJoint || dragMode == BottomRightJoint) {
        setCursor(Qt::SizeFDiagCursor);
    } else if (dragMode == TopRightJoint || dragMode == BottomLeftJoint) {
        setCursor(Qt::SizeBDiagCursor);
    } else if (dragMode == InnerRect) {
        setCursor(Qt::SizeAllCursor);
    }
}

void Controls::updateDragPosition(const QPoint &globalPos) {
    if (dragMode == Controls::None) return;
    
    QPoint delta = globalPos - dragStartPos;
    
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
        case TopLeftJoint:
            newRect.setLeft(dragStartRect.left() + delta.x());
            newRect.setTop(dragStartRect.top() + delta.y());
            break;
        case TopRightJoint:
            newRect.setRight(dragStartRect.right() + delta.x());
            newRect.setTop(dragStartRect.top() + delta.y());
            break;
        case BottomLeftJoint:
            newRect.setLeft(dragStartRect.left() + delta.x());
            newRect.setBottom(dragStartRect.bottom() + delta.y());
            break;
        case BottomRightJoint:
            newRect.setRight(dragStartRect.right() + delta.x());
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
    currentRect = newRect.normalized();
    
    // Ensure minimum size of 1x1
    if (currentRect.width() < 1) {
        currentRect.setWidth(1);
    }
    if (currentRect.height() < 1) {
        currentRect.setHeight(1);
    }
    
    updateGeometry(currentRect);
    emit rectChanged(currentRect);
}

void Controls::endDrag() {
    dragMode = Controls::None;
    hasDragged = false;
    setCursor(Qt::ArrowCursor);
}

void Controls::updateGeometry(const QRect &targetRect) {
    currentRect = targetRect;
    // Position the controls around the target rectangle
    positionControls(targetRect);
    
    // Calculate the actual margin based on zoom scale
    // Bar widths and joint sizes should remain fixed visually
    int visualBarWidth = 4;
    int visualJointSize = 25;
    int margin = qMax(visualJointSize, visualBarWidth * 2);
    
    // Resize the Controls widget to encompass all control bars
    resize(targetRect.width() + 2 * margin, targetRect.height() + 2 * margin);
    move(targetRect.left() - margin, targetRect.top() - margin);
}

void Controls::positionControls(const QRect &rect) {
    // Fixed visual sizes for bars and joints
    int barWidth = 4;
    int barHeight = 4;
    int jointSize = 25;
    
    // Update bar sizes to match the rectangle dimensions
    leftBar->setFixedWidth(barWidth);
    leftBar->setFixedHeight(rect.height());
    rightBar->setFixedWidth(barWidth);
    rightBar->setFixedHeight(rect.height());
    topBar->setFixedHeight(barHeight);
    topBar->setFixedWidth(rect.width());
    bottomBar->setFixedHeight(barHeight);
    bottomBar->setFixedWidth(rect.width());
    
    // Position bars relative to the Controls widget (which has been moved/resized)
    int margin = qMax(jointSize, barWidth * 2);
    
    // Position left bar - forms the left edge
    leftBar->move(margin - leftBar->width(), margin);
    
    // Position right bar - forms the right edge
    rightBar->move(margin + rect.width(), margin);
    
    // Position top bar - forms the top edge
    topBar->move(margin, margin - topBar->height());
    
    // Position bottom bar - forms the bottom edge
    bottomBar->move(margin, margin + rect.height());
    
    // Position corner joints - with corners at the intersections
    
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
    // Check joints first (they should have priority over bars)
    if (topLeftJoint->geometry().contains(pos)) return Controls::TopLeftJoint;
    if (topRightJoint->geometry().contains(pos)) return Controls::TopRightJoint;
    if (bottomLeftJoint->geometry().contains(pos)) return Controls::BottomLeftJoint;
    if (bottomRightJoint->geometry().contains(pos)) return Controls::BottomRightJoint;
    
    // Then check bars
    if (leftBar->geometry().contains(pos)) return Controls::LeftBar;
    if (rightBar->geometry().contains(pos)) return Controls::RightBar;
    if (topBar->geometry().contains(pos)) return Controls::TopBar;
    if (bottomBar->geometry().contains(pos)) return Controls::BottomBar;
    
    // Finally check inner rect
    if (innerRect->geometry().contains(pos)) return Controls::InnerRect;
    return Controls::None;
}

void Controls::mousePressEvent(QMouseEvent *event) {
    qDebug() << "Controls::mousePressEvent at" << event->pos();
    if (event->button() == Qt::LeftButton) {
        // If a single-click timer is running and we get another press, it might be a double-click
        // Don't start drag mode if this could be a double-click
        if (singleClickTimer->isActive() && getBarAt(event->pos()) == InnerRect) {
            // This could be the second click of a double-click
            return;
        }
        
        dragMode = getBarAt(event->pos());
        if (dragMode != Controls::None) {
            dragStartPos = event->globalPos();
            dragStartRect = currentRect;
            hasDragged = false;  // Reset drag tracking
            // Set appropriate arrow cursor based on which bar/joint is being dragged
            if (dragMode == LeftBar || dragMode == RightBar) {
                setCursor(Qt::SizeHorCursor);
            } else if (dragMode == TopBar || dragMode == BottomBar) {
                setCursor(Qt::SizeVerCursor);
            } else if (dragMode == TopLeftJoint || dragMode == BottomRightJoint) {
                setCursor(Qt::SizeFDiagCursor);  // Forward diagonal (/)
            } else if (dragMode == TopRightJoint || dragMode == BottomLeftJoint) {
                setCursor(Qt::SizeBDiagCursor);  // Backward diagonal (\)
            } else if (dragMode == InnerRect) {
                setCursor(Qt::SizeAllCursor);
            }
        }
    }
    QWidget::mousePressEvent(event);
}

void Controls::mouseMoveEvent(QMouseEvent *event) {
    if (dragMode != Controls::None && (event->buttons() & Qt::LeftButton)) {
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
            case TopLeftJoint:
                newRect.setLeft(dragStartRect.left() + delta.x());
                newRect.setTop(dragStartRect.top() + delta.y());
                break;
            case TopRightJoint:
                newRect.setRight(dragStartRect.right() + delta.x());
                newRect.setTop(dragStartRect.top() + delta.y());
                break;
            case BottomLeftJoint:
                newRect.setLeft(dragStartRect.left() + delta.x());
                newRect.setBottom(dragStartRect.bottom() + delta.y());
                break;
            case BottomRightJoint:
                newRect.setRight(dragStartRect.right() + delta.x());
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
        
        // Ensure minimum size of 1x1
        if (currentRect.width() < 1) {
            currentRect.setWidth(1);
        }
        if (currentRect.height() < 1) {
            currentRect.setHeight(1);
        }
        
        updateGeometry(currentRect);  // This will resize the Controls widget and reposition bars
        emit rectChanged(currentRect);
    } else {
        // Update cursor based on hover
        DragMode hoverMode = getBarAt(event->pos());
        if (hoverMode == LeftBar || hoverMode == RightBar) {
            setCursor(Qt::SizeHorCursor);
        } else if (hoverMode == TopBar || hoverMode == BottomBar) {
            setCursor(Qt::SizeVerCursor);
        } else if (hoverMode == TopLeftJoint || hoverMode == BottomRightJoint) {
            setCursor(Qt::SizeFDiagCursor);  // Forward diagonal (/)
        } else if (hoverMode == TopRightJoint || hoverMode == BottomLeftJoint) {
            setCursor(Qt::SizeBDiagCursor);  // Backward diagonal (\)
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
            // Start timer to wait for potential double-click
            pendingClickPos = event->globalPos();
            singleClickTimer->start();
        }
        
        dragMode = Controls::None;
        hasDragged = false;
        
        // Reset cursor based on current position
        DragMode hoverMode = getBarAt(event->pos());
        if (hoverMode == LeftBar || hoverMode == RightBar) {
            setCursor(Qt::SizeHorCursor);
        } else if (hoverMode == TopBar || hoverMode == BottomBar) {
            setCursor(Qt::SizeVerCursor);
        } else if (hoverMode == TopLeftJoint || hoverMode == BottomRightJoint) {
            setCursor(Qt::SizeFDiagCursor);  // Forward diagonal (/)
        } else if (hoverMode == TopRightJoint || hoverMode == BottomLeftJoint) {
            setCursor(Qt::SizeBDiagCursor);  // Backward diagonal (\)
        } else if (hoverMode == InnerRect) {
            setCursor(Qt::SizeAllCursor);
        } else {
            setCursor(Qt::ArrowCursor);
        }
    }
    QWidget::mouseReleaseEvent(event);
}

void Controls::mouseDoubleClickEvent(QMouseEvent *event) {
    qDebug() << "Controls::mouseDoubleClickEvent at" << event->pos();
    // Check if double-click is on the inner rectangle
    if (innerRect->geometry().contains(event->pos())) {
        // Stop the single-click timer to prevent single-click processing
        singleClickTimer->stop();
        qDebug() << "Double-click on inner rect, emitting signal";
        emit innerRectDoubleClicked(event->globalPos());
    }
    QWidget::mouseDoubleClickEvent(event);
}