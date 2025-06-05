#include "Canvas.h"
#include "Controls.h"
#include "Element.h"
#include "Frame.h"
#include "Text.h"
#include "Variable.h"
#include "ClientRect.h"
#include "ActionsPanel.h"
#include "FPSWidget.h"
#include <QMouseEvent>
#include <QApplication>
#include <QPainter>
#include <QDebug>

Canvas::Canvas(QWidget *parent) : QWidget(parent), mode("Select"), panOffset(0, 0) {
    setContentsMargins(0, 0, 0, 0);
    setMouseTracking(true);  // Enable mouse tracking for cursor changes
    
    // Create UI controls directly on Canvas
    controls = new Controls(this);
    controls->hide();  // Initially hidden
    
    // Connect controls signals
    connect(controls, &Controls::rectChanged, this, &Canvas::onControlsRectChanged);
    connect(controls, &Controls::innerRectClicked, this, &Canvas::onControlsInnerRectClicked);
    
    // Ensure controls are on top
    controls->raise();
}

void Canvas::resizeEvent(QResizeEvent *event) {
    QWidget::resizeEvent(event);
}

void Canvas::showControls(const QRect &rect) {
    if (controls) {
        controls->setPanOffset(panOffset);
        controls->updateGeometry(rect);
        controls->show();
        controls->raise();
        
        // Emit signal to raise overlay panels
        emit overlaysNeedRaise();
    }
}

void Canvas::hideControls() {
    if (controls) {
        controls->hide();
    }
}

void Canvas::selectElement(const QString &elementId, bool addToSelection) {
    if (!addToSelection) {
        selectedElements.clear();
    }
    
    // Only add if not already selected
    if (!selectedElements.contains(elementId)) {
        selectedElements.append(elementId);
    }
    
    updateControlsVisibility();
}

void Canvas::updateControlsVisibility() {
    // Hide controls if no selection
    if (selectedElements.isEmpty()) {
        hideControls();
        return;
    }
    
    // Check if any selected element is a Frame and calculate bounding rect
    bool hasFrame = false;
    QRect boundingRect;
    bool firstFrame = true;
    
    for (const QString &id : selectedElements) {
        // Find the element with this ID
        for (Element *element : elements) {
            if (QString::number(element->getId()) == id) {
                if (element->getType() == Element::FrameType) {
                    hasFrame = true;
                    QRect canvasRect(element->getCanvasPosition(), element->size());
                    if (firstFrame) {
                        boundingRect = canvasRect;
                        firstFrame = false;
                    } else {
                        // Expand bounding rect to include this frame
                        boundingRect = boundingRect.united(canvasRect);
                    }
                }
                break;
            }
        }
    }
    
    // Show controls if at least one Frame is selected
    if (hasFrame) {
        if (controls) {
            controls->setPanOffset(panOffset);
        }
        showControls(boundingRect);
    } else {
        hideControls();
    }
}

void Canvas::setMode(const QString &newMode) {
    if (mode != newMode) {
        mode = newMode;
        emit modeChanged(mode);
        
        // Handle mode-specific actions
        if (newMode == "Text") {
            // When Text mode is selected, create a text element immediately
            createText();
        } else if (newMode == "Variable") {
            // When Variable mode is selected, create a variable immediately
            createVariable();
        }
    }
}

void Canvas::setPanOffset(const QPoint &offset) {
    if (panOffset != offset) {
        panOffset = offset;
        
        // Update controls pan offset
        if (controls) {
            controls->setPanOffset(panOffset);
        }
        
        // Update visual positions of all elements based on their canvas positions
        for (Element *element : elements) {
            element->updateVisualPosition(panOffset);
            
            // Also update associated ClientRect if it's a Frame
            if (element->getType() == Element::FrameType) {
                Frame *frame = qobject_cast<Frame*>(element);
                if (frame) {
                    // Find ClientRect with the same ID
                    for (QObject *child : children()) {
                        ClientRect *clientRect = qobject_cast<ClientRect*>(child);
                        if (clientRect && clientRect->getAssociatedElementId() == frame->getId()) {
                            clientRect->move(element->getCanvasPosition() + panOffset);
                            break;
                        }
                    }
                }
            }
        }
        
        // Update controls position if visible
        if (controls && controls->isVisible()) {
            updateControlsVisibility();
        }
        
        update();  // Trigger repaint
    }
}

void Canvas::createFrame() {
    // Frame creation now happens via mouse drag events
    // This method is kept for API compatibility but does nothing
}

void Canvas::createText() {
    // Generate ID based on number of elements + 1
    int textId = elements.size() + 1;
    
    // Create a new text element with Canvas as parent
    Text *text = new Text(textId, this);
    
    // Calculate canvas position (center of viewport in canvas coordinates)
    QPoint canvasPos((width() - text->width()) / 2 - panOffset.x(),
                     (height() - text->height()) / 2 - panOffset.y());
    text->setCanvasPosition(canvasPos);
    text->updateVisualPosition(panOffset);
    text->show();
    
    // Add to elements list
    elements.append(text);
    
    // Emit signal that a text element was created
    emit elementCreated("Text", text->getName());
    
    // Switch back to Select mode
    setMode("Select");
}

void Canvas::createVariable() {
    // Generate ID based on number of elements + 1
    int variableId = elements.size() + 1;
    
    // Create a new variable - NOTE: Variables don't appear on canvas
    Variable *variable = new Variable(variableId, nullptr);
    
    // Add to elements list
    elements.append(variable);
    
    // Emit signal that a variable was created
    emit elementCreated("Variable", variable->getName());
    
    // Switch back to Select mode
    setMode("Select");
}

void Canvas::mousePressEvent(QMouseEvent *event) {
    if (event->button() == Qt::LeftButton) {
        if (mode == "Frame") {
            // Generate ID based on number of elements + 1
            int frameId = elements.size() + 1;
            
            // Calculate canvas position (logical position without pan)
            QPoint canvasPos = event->pos() - panOffset;
            
            Frame *frame = new Frame(frameId, this);
            frame->resize(400, 400);
            frame->setCanvasPosition(canvasPos);
            frame->updateVisualPosition(panOffset);
            frame->show();
            
            // Create a ClientRect with the same size and position
            ClientRect *clientRect = new ClientRect(frameId, this, this);
            clientRect->resize(400, 400);
            clientRect->move(canvasPos + panOffset);
            clientRect->show();
            
            // Add frame to elements list (ClientRect is not an Element)
            elements.append(frame);
            
            // Add to selected elements
            selectedElements.clear();  // Clear previous selection
            selectedElements.append(QString::number(frame->getId()));
            updateControlsVisibility();
            
            // Ensure controls stay on top
            if (controls) {
                controls->raise();
            }
            
            // Emit signal that a frame was created
            emit elementCreated("Frame", frame->getName());
            
            // Emit signal to raise overlay panels (ActionsPanel, FPSWidget, etc.)
            emit overlaysNeedRaise();
            
            // Switch back to Select mode
            setMode("Select");
            
        } else {
            // In Select mode or other modes, check if click is on empty canvas
            QWidget *widget = childAt(event->pos());
            
            // If clicked on canvas itself (not on any child widget), clear selection
            if (!widget || widget == this) {
                selectedElements.clear();
                updateControlsVisibility();
            }
        }
    }
    
    // Always propagate the event
    QWidget::mousePressEvent(event);
}

void Canvas::mouseMoveEvent(QMouseEvent *event) {
    // Use normal event propagation
    QWidget::mouseMoveEvent(event);
}

void Canvas::mouseReleaseEvent(QMouseEvent *event) {
    // Use normal event propagation
    QWidget::mouseReleaseEvent(event);
}

void Canvas::wheelEvent(QWheelEvent *event) {
    // Get the scroll amount (positive = up, negative = down)
    QPoint numPixels = event->pixelDelta();
    QPoint numDegrees = event->angleDelta() / 8;
    
    int delta = 0;
    if (!numPixels.isNull()) {
        delta = numPixels.y();
    } else if (!numDegrees.isNull()) {
        delta = numDegrees.y() * 3;  // Convert degrees to pixels (approximate)
    }
    
    // Update pan offset on Y axis
    if (delta != 0) {
        QPoint newOffset = panOffset;
        newOffset.setY(panOffset.y() + delta);
        setPanOffset(newOffset);
        
        // Accept the event to prevent propagation
        event->accept();
    } else {
        QWidget::wheelEvent(event);
    }
}

void Canvas::paintEvent(QPaintEvent *) {
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);
    
    // Clear background
    painter.fillRect(rect(), QColor(242, 242, 242));
}

void Canvas::render() {
    update();  // Triggers paintEvent
}

void Canvas::onControlsRectChanged(const QRect &newRect) {
    // Get the original bounding rect of selected frames
    QRect originalBoundingRect;
    bool firstFrame = true;
    QList<Element*> selectedFrames;
    
    // Collect all selected frames and calculate original bounding rect
    for (const QString &id : selectedElements) {
        for (Element *element : elements) {
            if (QString::number(element->getId()) == id) {
                if (element->getType() == Element::FrameType) {
                    selectedFrames.append(element);
                    QRect canvasRect(element->getCanvasPosition(), element->size());
                    if (firstFrame) {
                        originalBoundingRect = canvasRect;
                        firstFrame = false;
                    } else {
                        originalBoundingRect = originalBoundingRect.united(canvasRect);
                    }
                }
                break;
            }
        }
    }
    
    // If no frames selected, nothing to update
    if (selectedFrames.isEmpty()) {
        return;
    }
    
    // Check if we need to flip the elements
    bool flipX = newRect.width() < 0;
    bool flipY = newRect.height() < 0;
    
    // Get the normalized rectangle for proper positioning
    QRect normalizedNewRect = newRect.normalized();
    
    // Update each selected frame
    for (Element *element : selectedFrames) {
        QRect oldCanvasRect(element->getCanvasPosition(), element->size());
        
        // Calculate relative position within the original bounding rect
        qreal relativeX = qreal(oldCanvasRect.left() - originalBoundingRect.left()) / qreal(originalBoundingRect.width());
        qreal relativeY = qreal(oldCanvasRect.top() - originalBoundingRect.top()) / qreal(originalBoundingRect.height());
        qreal relativeWidth = qreal(oldCanvasRect.width()) / qreal(originalBoundingRect.width());
        qreal relativeHeight = qreal(oldCanvasRect.height()) / qreal(originalBoundingRect.height());
        
        // If flipped horizontally, adjust the relative X position
        if (flipX) {
            relativeX = 1.0 - relativeX - relativeWidth;
        }
        
        // If flipped vertically, adjust the relative Y position
        if (flipY) {
            relativeY = 1.0 - relativeY - relativeHeight;
        }
        
        // Calculate new geometry based on normalized rectangle
        int newX = normalizedNewRect.left() + qRound(relativeX * normalizedNewRect.width());
        int newY = normalizedNewRect.top() + qRound(relativeY * normalizedNewRect.height());
        int newWidth = qRound(relativeWidth * normalizedNewRect.width());
        int newHeight = qRound(relativeHeight * normalizedNewRect.height());
        
        // Update the element's canvas position and size
        element->setCanvasPosition(QPoint(newX, newY));
        element->resize(newWidth, newHeight);
        element->updateVisualPosition(panOffset);
        
        // Also update any associated ClientRect
        Frame *frame = qobject_cast<Frame*>(element);
        if (frame) {
            // Find ClientRect with the same ID
            for (QObject *child : children()) {
                ClientRect *clientRect = qobject_cast<ClientRect*>(child);
                if (clientRect && clientRect->getAssociatedElementId() == frame->getId()) {
                    clientRect->setGeometry(QRect(newX + panOffset.x(), newY + panOffset.y(), newWidth, newHeight));
                    break;
                }
            }
        }
    }
    
    // Ensure controls stay on top
    if (controls) {
        controls->raise();
    }
    
    // Emit signal to raise overlay panels
    emit overlaysNeedRaise();
    
    // Trigger a repaint
    update();
}

void Canvas::onControlsInnerRectClicked(const QPoint &globalPos) {
    // Convert global position to local canvas position
    QPoint localPos = mapFromGlobal(globalPos) - panOffset;
    
    // Find which ClientRect is at this position
    ClientRect* clickedClientRect = nullptr;
    for (QObject *child : children()) {
        ClientRect *clientRect = qobject_cast<ClientRect*>(child);
        if (clientRect && clientRect->geometry().contains(localPos)) {
            clickedClientRect = clientRect;
            break;
        }
    }
    
    // If a ClientRect was found, select its associated element
    if (clickedClientRect) {
        QString elementId = QString::number(clickedClientRect->getAssociatedElementId());
        selectElement(elementId, false);  // false = don't add to selection, replace it
    }
}

void Canvas::startControlDrag(const QPoint &globalPos) {
    if (controls && controls->isVisible()) {
        // Simulate a mouse press on the controls' inner rect
        QPoint controlsLocalPos = controls->mapFromGlobal(globalPos);
        QMouseEvent pressEvent(QEvent::MouseButtonPress, controlsLocalPos, globalPos,
                               Qt::LeftButton, Qt::LeftButton, Qt::NoModifier);
        QApplication::sendEvent(controls, &pressEvent);
    }
}

void Canvas::updateControlDrag(const QPoint &globalPos) {
    if (controls && controls->isVisible()) {
        // Forward mouse move to controls
        QPoint controlsLocalPos = controls->mapFromGlobal(globalPos);
        QMouseEvent moveEvent(QEvent::MouseMove, controlsLocalPos, globalPos,
                              Qt::NoButton, Qt::LeftButton, Qt::NoModifier);
        QApplication::sendEvent(controls, &moveEvent);
    }
}

void Canvas::endControlDrag(const QPoint &globalPos) {
    if (controls && controls->isVisible()) {
        // Forward mouse release to controls
        QPoint controlsLocalPos = controls->mapFromGlobal(globalPos);
        QMouseEvent releaseEvent(QEvent::MouseButtonRelease, controlsLocalPos, globalPos,
                                 Qt::LeftButton, Qt::NoButton, Qt::NoModifier);
        QApplication::sendEvent(controls, &releaseEvent);
    }
}