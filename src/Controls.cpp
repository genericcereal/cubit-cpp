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
    
    // Configure for use in QGraphicsProxyWidget
    setAttribute(Qt::WA_TranslucentBackground, true);
    setAttribute(Qt::WA_NoSystemBackground, true);
    setAttribute(Qt::WA_OpaquePaintEvent, false);
    setAutoFillBackground(false);
    setFocusPolicy(Qt::StrongFocus);
    setStyleSheet("QWidget { background-color: transparent; }");
    
    // Initialize single-click timer for double-click detection
    // (Currently not used since we emit clicks immediately, but kept for future use)
    singleClickTimer = new QTimer(this);
    singleClickTimer->setSingleShot(true);
    singleClickTimer->setInterval(QApplication::doubleClickInterval());
    
    // Create the four control bars
    leftBar = new QFrame(this);
    leftBar->setStyleSheet("background-color: rgba(255, 0, 0, 0.5);");
    leftBar->setAttribute(Qt::WA_TransparentForMouseEvents, false);
    leftBar->show();
    
    rightBar = new QFrame(this);
    rightBar->setStyleSheet("background-color: rgba(255, 0, 0, 0.5);");
    rightBar->setAttribute(Qt::WA_TransparentForMouseEvents, false);
    rightBar->show();
    
    topBar = new QFrame(this);
    topBar->setStyleSheet("background-color: rgba(255, 0, 0, 0.5);");
    topBar->setAttribute(Qt::WA_TransparentForMouseEvents, false);
    topBar->show();
    
    bottomBar = new QFrame(this);
    bottomBar->setStyleSheet("background-color: rgba(255, 0, 0, 0.5);");
    bottomBar->setAttribute(Qt::WA_TransparentForMouseEvents, false);
    bottomBar->show();
    
    // Create center lines for each bar
    leftBarLine = new QFrame(this);
    leftBarLine->setStyleSheet("background-color: rgba(0, 0, 0, 0.5);");
    leftBarLine->setAttribute(Qt::WA_TransparentForMouseEvents, true);
    leftBarLine->show();
    
    rightBarLine = new QFrame(this);
    rightBarLine->setStyleSheet("background-color: rgba(0, 0, 0, 0.5);");
    rightBarLine->setAttribute(Qt::WA_TransparentForMouseEvents, true);
    rightBarLine->show();
    
    topBarLine = new QFrame(this);
    topBarLine->setStyleSheet("background-color: rgba(0, 0, 0, 0.5);");
    topBarLine->setAttribute(Qt::WA_TransparentForMouseEvents, true);
    topBarLine->show();
    
    bottomBarLine = new QFrame(this);
    bottomBarLine->setStyleSheet("background-color: rgba(0, 0, 0, 0.5);");
    bottomBarLine->setAttribute(Qt::WA_TransparentForMouseEvents, true);
    bottomBarLine->show();
    
    // Create the four corner rotation joints
    topLeftRotationJoint = new QFrame(this);
    topLeftRotationJoint->setStyleSheet("background-color: rgba(0, 0, 255, 0.5);");
    topLeftRotationJoint->setAttribute(Qt::WA_TransparentForMouseEvents, false);
    topLeftRotationJoint->show();
    
    topRightRotationJoint = new QFrame(this);
    topRightRotationJoint->setStyleSheet("background-color: rgba(0, 0, 255, 0.5);");
    topRightRotationJoint->setAttribute(Qt::WA_TransparentForMouseEvents, false);
    topRightRotationJoint->show();
    
    bottomLeftRotationJoint = new QFrame(this);
    bottomLeftRotationJoint->setStyleSheet("background-color: rgba(0, 0, 255, 0.5);");
    bottomLeftRotationJoint->setAttribute(Qt::WA_TransparentForMouseEvents, false);
    bottomLeftRotationJoint->show();
    
    bottomRightRotationJoint = new QFrame(this);
    bottomRightRotationJoint->setStyleSheet("background-color: rgba(0, 0, 255, 0.5);");
    bottomRightRotationJoint->setAttribute(Qt::WA_TransparentForMouseEvents, false);
    bottomRightRotationJoint->show();
    
    // Create the four resize joints
    topLeftResizeJoint = new QFrame(this);
    topLeftResizeJoint->setStyleSheet("background-color: rgba(255, 255, 0, 0.5);");
    topLeftResizeJoint->setAttribute(Qt::WA_TransparentForMouseEvents, false);
    topLeftResizeJoint->show();
    
    topRightResizeJoint = new QFrame(this);
    topRightResizeJoint->setStyleSheet("background-color: rgba(255, 255, 0, 0.5);");
    topRightResizeJoint->setAttribute(Qt::WA_TransparentForMouseEvents, false);
    topRightResizeJoint->show();
    
    bottomLeftResizeJoint = new QFrame(this);
    bottomLeftResizeJoint->setStyleSheet("background-color: rgba(255, 255, 0, 0.5);");
    bottomLeftResizeJoint->setAttribute(Qt::WA_TransparentForMouseEvents, false);
    bottomLeftResizeJoint->show();
    
    bottomRightResizeJoint = new QFrame(this);
    bottomRightResizeJoint->setStyleSheet("background-color: rgba(255, 255, 0, 0.5);");
    bottomRightResizeJoint->setAttribute(Qt::WA_TransparentForMouseEvents, false);
    bottomRightResizeJoint->show();
    
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
    } else if (dragMode == TopLeftRotationJoint || dragMode == BottomRightRotationJoint) {
        setCursor(Qt::SizeFDiagCursor);
    } else if (dragMode == TopRightRotationJoint || dragMode == BottomLeftRotationJoint) {
        setCursor(Qt::SizeBDiagCursor);
    } else if (dragMode == TopLeftResizeJoint || dragMode == BottomRightResizeJoint) {
        setCursor(Qt::SizeFDiagCursor);
    } else if (dragMode == TopRightResizeJoint || dragMode == BottomLeftResizeJoint) {
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
    
    // Scale the delta by inverse of zoom to convert from screen to scene coordinates
    int scaledDeltaX = qRound(delta.x() / zoomScale);
    int scaledDeltaY = qRound(delta.y() / zoomScale);
    
    QRect newRect = dragStartRect;
    
    switch (dragMode) {
        case LeftBar:
            newRect.setLeft(dragStartRect.left() + scaledDeltaX);
            break;
        case RightBar:
            newRect.setRight(dragStartRect.right() + scaledDeltaX);
            break;
        case TopBar:
            newRect.setTop(dragStartRect.top() + scaledDeltaY);
            break;
        case BottomBar:
            newRect.setBottom(dragStartRect.bottom() + scaledDeltaY);
            break;
        case TopLeftRotationJoint:
            // Rotation joints don't do anything for now
            break;
        case TopRightRotationJoint:
            // Rotation joints don't do anything for now
            break;
        case BottomLeftRotationJoint:
            // Rotation joints don't do anything for now
            break;
        case BottomRightRotationJoint:
            // Rotation joints don't do anything for now
            break;
        case TopLeftResizeJoint:
            newRect.setLeft(dragStartRect.left() + scaledDeltaX);
            newRect.setTop(dragStartRect.top() + scaledDeltaY);
            break;
        case TopRightResizeJoint:
            newRect.setRight(dragStartRect.right() + scaledDeltaX);
            newRect.setTop(dragStartRect.top() + scaledDeltaY);
            break;
        case BottomLeftResizeJoint:
            newRect.setLeft(dragStartRect.left() + scaledDeltaX);
            newRect.setBottom(dragStartRect.bottom() + scaledDeltaY);
            break;
        case BottomRightResizeJoint:
            newRect.setRight(dragStartRect.right() + scaledDeltaX);
            newRect.setBottom(dragStartRect.bottom() + scaledDeltaY);
            break;
        case InnerRect:
            // Move the entire rectangle
            newRect.moveLeft(dragStartRect.left() + scaledDeltaX);
            newRect.moveTop(dragStartRect.top() + scaledDeltaY);
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
    int visualBarWidth = 10;
    int visualRotationJointSize = 20;
    
    // Scale the margin by inverse of zoom to maintain visual size
    int scaledBarWidth = qRound(visualBarWidth / zoomScale);
    int scaledRotationJointSize = qRound(visualRotationJointSize / zoomScale);
    int margin = qMax(scaledRotationJointSize, scaledBarWidth * 2);
    
    // Resize the Controls widget to encompass all control bars
    resize(targetRect.width() + 2 * margin, targetRect.height() + 2 * margin);
    
    // When used in a QGraphicsProxyWidget, don't move the widget itself
    // Instead, store the intended position for the proxy to use
    intendedPosition = QPoint(targetRect.left() - margin, targetRect.top() - margin);
    
    // Only call move() if we have a parent widget (not in a proxy)
    if (parentWidget()) {
        move(intendedPosition);
    }
}

void Controls::positionControls(const QRect &rect) {
    // Fixed visual sizes for bars and joints
    int visualBarWidth = 10;
    int visualBarHeight = 10;
    int visualRotationJointSize = 20;  // Blue rotation joints
    int visualResizeJointSize = 10;     // Yellow resize joints
    
    // Scale sizes by inverse of zoom to maintain visual size
    int barWidth = qRound(visualBarWidth / zoomScale);
    int barHeight = qRound(visualBarHeight / zoomScale);
    int rotationJointSize = qRound(visualRotationJointSize / zoomScale);
    int resizeJointSize = qRound(visualResizeJointSize / zoomScale);
    
    // Update bar sizes to match the rectangle dimensions
    leftBar->resize(barWidth, rect.height());
    rightBar->resize(barWidth, rect.height());
    topBar->resize(rect.width(), barHeight);
    bottomBar->resize(rect.width(), barHeight);
    
    // Position bars relative to the Controls widget (which has been moved/resized)
    int margin = qMax(rotationJointSize, barWidth * 2);
    
    // Position left bar - centered on the left edge of selection
    leftBar->move(margin - barWidth/2, margin);
    
    // Position right bar - centered on the right edge of selection
    rightBar->move(margin + rect.width() - barWidth/2, margin);
    
    // Position top bar - centered on the top edge of selection
    topBar->move(margin, margin - barHeight/2);
    
    // Position bottom bar - centered on the bottom edge of selection
    bottomBar->move(margin, margin + rect.height() - barHeight/2);
    
    // Update rotation joint sizes
    topLeftRotationJoint->resize(rotationJointSize, rotationJointSize);
    topRightRotationJoint->resize(rotationJointSize, rotationJointSize);
    bottomLeftRotationJoint->resize(rotationJointSize, rotationJointSize);
    bottomRightRotationJoint->resize(rotationJointSize, rotationJointSize);
    
    // Position corner rotation joints - with scaled overlap at the appropriate corners
    int overlapSize = qRound(10 / zoomScale);
    
    // Top-left joint - bottom-right corner overlaps with bars
    topLeftRotationJoint->move(margin - barWidth/2 - (rotationJointSize - overlapSize), 
                       margin - barHeight/2 - (rotationJointSize - overlapSize));
    
    // Top-right joint - bottom-left corner overlaps with bars
    topRightRotationJoint->move(margin + rect.width() - barWidth/2, 
                        margin - barHeight/2 - (rotationJointSize - overlapSize));
    
    // Bottom-left joint - top-right corner overlaps with bars
    bottomLeftRotationJoint->move(margin - barWidth/2 - (rotationJointSize - overlapSize), 
                          margin + rect.height() - barHeight/2);
    
    // Bottom-right joint - top-left corner overlaps with bars
    bottomRightRotationJoint->move(margin + rect.width() - barWidth/2, 
                           margin + rect.height() - barHeight/2);
    
    // Position inner rectangle to fit between the bars
    innerRect->move(margin, margin);
    innerRect->resize(rect.width(), rect.height());
    
    // Position center lines
    int lineWidth = qMax(1, qRound(1 / zoomScale));
    
    // Left bar line - vertical line in center of left bar
    leftBarLine->resize(lineWidth, rect.height());
    leftBarLine->move(margin - lineWidth/2, margin);
    
    // Right bar line - vertical line in center of right bar
    rightBarLine->resize(lineWidth, rect.height());
    rightBarLine->move(margin + rect.width() - lineWidth/2, margin);
    
    // Top bar line - horizontal line in center of top bar
    topBarLine->resize(rect.width(), lineWidth);
    topBarLine->move(margin, margin - lineWidth/2);
    
    // Bottom bar line - horizontal line in center of bottom bar
    bottomBarLine->resize(rect.width(), lineWidth);
    bottomBarLine->move(margin, margin + rect.height() - lineWidth/2);
    
    // Update resize joint sizes
    topLeftResizeJoint->resize(resizeJointSize, resizeJointSize);
    topRightResizeJoint->resize(resizeJointSize, resizeJointSize);
    bottomLeftResizeJoint->resize(resizeJointSize, resizeJointSize);
    bottomRightResizeJoint->resize(resizeJointSize, resizeJointSize);
    
    // Position resize joints at the intersection points
    // Top-left resize joint - at bottom-right corner of top-left rotation joint
    topLeftResizeJoint->move(margin - barWidth/2, margin - barHeight/2);
    
    // Top-right resize joint - at bottom-left corner of top-right rotation joint
    topRightResizeJoint->move(margin + rect.width() - barWidth/2, margin - barHeight/2);
    
    // Bottom-left resize joint - at top-right corner of bottom-left rotation joint
    bottomLeftResizeJoint->move(margin - barWidth/2, margin + rect.height() - barHeight/2);
    
    // Bottom-right resize joint - at top-left corner of bottom-right rotation joint
    bottomRightResizeJoint->move(margin + rect.width() - barWidth/2, margin + rect.height() - barHeight/2);
}

Controls::DragMode Controls::getBarAt(const QPoint &pos) const {
    // Check resize joints first (they have highest priority as they're on top)
    if (topLeftResizeJoint->geometry().contains(pos)) return Controls::TopLeftResizeJoint;
    if (topRightResizeJoint->geometry().contains(pos)) return Controls::TopRightResizeJoint;
    if (bottomLeftResizeJoint->geometry().contains(pos)) return Controls::BottomLeftResizeJoint;
    if (bottomRightResizeJoint->geometry().contains(pos)) return Controls::BottomRightResizeJoint;
    
    // Then check rotation joints (they should have priority over bars)
    if (topLeftRotationJoint->geometry().contains(pos)) return Controls::TopLeftRotationJoint;
    if (topRightRotationJoint->geometry().contains(pos)) return Controls::TopRightRotationJoint;
    if (bottomLeftRotationJoint->geometry().contains(pos)) return Controls::BottomLeftRotationJoint;
    if (bottomRightRotationJoint->geometry().contains(pos)) return Controls::BottomRightRotationJoint;
    
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
            
            // Set appropriate arrow cursor based on which bar/rotation joint is being dragged
            if (dragMode == LeftBar || dragMode == RightBar) {
                setCursor(Qt::SizeHorCursor);
            } else if (dragMode == TopBar || dragMode == BottomBar) {
                setCursor(Qt::SizeVerCursor);
            } else if (dragMode == TopLeftRotationJoint || dragMode == BottomRightRotationJoint) {
                setCursor(Qt::SizeFDiagCursor);  // Forward diagonal (/)
            } else if (dragMode == TopRightRotationJoint || dragMode == BottomLeftRotationJoint) {
                setCursor(Qt::SizeBDiagCursor);  // Backward diagonal (\)
            } else if (dragMode == TopLeftResizeJoint || dragMode == BottomRightResizeJoint) {
                setCursor(Qt::SizeFDiagCursor);  // Forward diagonal (/)
            } else if (dragMode == TopRightResizeJoint || dragMode == BottomLeftResizeJoint) {
                setCursor(Qt::SizeBDiagCursor);  // Backward diagonal (\)
            } else if (dragMode == InnerRect) {
                setCursor(Qt::SizeAllCursor);
            }
            
            // Important: Accept the event to prevent it from propagating
            event->accept();
        }
    }
    
    // Don't call base class if we handled the event
    if (dragMode == Controls::None) {
        QWidget::mousePressEvent(event);
    }
}

void Controls::mouseMoveEvent(QMouseEvent *event) {
    
    if (dragMode != Controls::None && (event->buttons() & Qt::LeftButton)) {
        QPoint delta = event->globalPos() - dragStartPos;
        
        // Consider it a drag if mouse moved more than 5 pixels
        if (delta.manhattanLength() > 5) {
            hasDragged = true;
        }
        
        // Scale the delta by inverse of zoom to convert from screen to scene coordinates
        int scaledDeltaX = qRound(delta.x() / zoomScale);
        int scaledDeltaY = qRound(delta.y() / zoomScale);
        
        QRect newRect = dragStartRect;
        
        switch (dragMode) {
            case LeftBar:
                newRect.setLeft(dragStartRect.left() + scaledDeltaX);
                break;
            case RightBar:
                newRect.setRight(dragStartRect.right() + scaledDeltaX);
                break;
            case TopBar:
                newRect.setTop(dragStartRect.top() + scaledDeltaY);
                break;
            case BottomBar:
                newRect.setBottom(dragStartRect.bottom() + scaledDeltaY);
                break;
            case TopLeftRotationJoint:
                // Rotation joints don't do anything for now
                break;
            case TopRightRotationJoint:
                // Rotation joints don't do anything for now
                break;
            case BottomLeftRotationJoint:
                // Rotation joints don't do anything for now
                break;
            case BottomRightRotationJoint:
                // Rotation joints don't do anything for now
                break;
            case TopLeftResizeJoint:
                newRect.setLeft(dragStartRect.left() + scaledDeltaX);
                newRect.setTop(dragStartRect.top() + scaledDeltaY);
                break;
            case TopRightResizeJoint:
                newRect.setRight(dragStartRect.right() + scaledDeltaX);
                newRect.setTop(dragStartRect.top() + scaledDeltaY);
                break;
            case BottomLeftResizeJoint:
                newRect.setLeft(dragStartRect.left() + scaledDeltaX);
                newRect.setBottom(dragStartRect.bottom() + scaledDeltaY);
                break;
            case BottomRightResizeJoint:
                newRect.setRight(dragStartRect.right() + scaledDeltaX);
                newRect.setBottom(dragStartRect.bottom() + scaledDeltaY);
                break;
            case InnerRect:
                // Move the entire rectangle
                newRect.moveLeft(dragStartRect.left() + scaledDeltaX);
                newRect.moveTop(dragStartRect.top() + scaledDeltaY);
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
        // Only update cursor if mouse is within widget bounds
        if (rect().contains(event->pos())) {
            // Update cursor based on hover
            DragMode hoverMode = getBarAt(event->pos());
            if (hoverMode == LeftBar || hoverMode == RightBar) {
                setCursor(Qt::SizeHorCursor);
            } else if (hoverMode == TopBar || hoverMode == BottomBar) {
                setCursor(Qt::SizeVerCursor);
            } else if (hoverMode == TopLeftRotationJoint || hoverMode == BottomRightRotationJoint) {
                setCursor(Qt::SizeFDiagCursor);  // Forward diagonal (/)
            } else if (hoverMode == TopRightRotationJoint || hoverMode == BottomLeftRotationJoint) {
                setCursor(Qt::SizeBDiagCursor);  // Backward diagonal (\)
            } else if (hoverMode == TopLeftResizeJoint || hoverMode == BottomRightResizeJoint) {
                setCursor(Qt::SizeFDiagCursor);  // Forward diagonal (/)
            } else if (hoverMode == TopRightResizeJoint || hoverMode == BottomLeftResizeJoint) {
                setCursor(Qt::SizeBDiagCursor);  // Backward diagonal (\)
            } else if (hoverMode == InnerRect) {
                setCursor(Qt::SizeAllCursor);
            } else {
                setCursor(Qt::ArrowCursor);
            }
        }
    }
    QWidget::mouseMoveEvent(event);
}

void Controls::mouseReleaseEvent(QMouseEvent *event) {
    if (event->button() == Qt::LeftButton) {
        // Check if this was a click on the inner rectangle (not a drag)
        if (dragMode == InnerRect && !hasDragged) {
            // Emit immediate click for selection purposes
            emit innerRectClicked(event->globalPos());
            
            // Also store for potential double-click detection
            pendingClickPos = event->globalPos();
            // Don't start the timer - we've already handled the single click
        }
        
        dragMode = Controls::None;
        hasDragged = false;
        
        // Reset cursor based on current position
        DragMode hoverMode = getBarAt(event->pos());
        if (hoverMode == LeftBar || hoverMode == RightBar) {
            setCursor(Qt::SizeHorCursor);
        } else if (hoverMode == TopBar || hoverMode == BottomBar) {
            setCursor(Qt::SizeVerCursor);
        } else if (hoverMode == TopLeftRotationJoint || hoverMode == BottomRightRotationJoint) {
            setCursor(Qt::SizeFDiagCursor);  // Forward diagonal (/)
        } else if (hoverMode == TopRightRotationJoint || hoverMode == BottomLeftRotationJoint) {
            setCursor(Qt::SizeBDiagCursor);  // Backward diagonal (\)
        } else if (hoverMode == TopLeftResizeJoint || hoverMode == BottomRightResizeJoint) {
            setCursor(Qt::SizeFDiagCursor);  // Forward diagonal (/)
        } else if (hoverMode == TopRightResizeJoint || hoverMode == BottomLeftResizeJoint) {
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
    // Check if double-click is on the inner rectangle
    if (innerRect->geometry().contains(event->pos())) {
        // Stop the single-click timer to prevent single-click processing
        singleClickTimer->stop();
        emit innerRectDoubleClicked(event->globalPos());
    }
    QWidget::mouseDoubleClickEvent(event);
}