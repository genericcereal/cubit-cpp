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
                    if (firstFrame) {
                        boundingRect = element->geometry();
                        firstFrame = false;
                    } else {
                        // Expand bounding rect to include this frame
                        boundingRect = boundingRect.united(element->geometry());
                    }
                }
                break;
            }
        }
    }
    
    // Show controls if at least one Frame is selected
    if (hasFrame) {
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
        QPoint delta = offset - panOffset;
        panOffset = offset;
        
        // Update positions of all child widgets (elements, client rects, controls)
        for (QObject *child : children()) {
            QWidget *widget = qobject_cast<QWidget*>(child);
            if (widget) {
                widget->move(widget->pos() + delta);
            }
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
    
    // Position it at a default location or center it, accounting for pan
    int textX = (width() - text->width()) / 2 - panOffset.x();
    int textY = (height() - text->height()) / 2 - panOffset.y();
    text->move(textX, textY);
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
            
            // Create a new frame at the click position (400x400)
            // Account for pan offset
            QPoint adjustedPos = event->pos() - panOffset;
            
            Frame *frame = new Frame(frameId, this);
            frame->resize(400, 400);
            frame->move(adjustedPos);
            frame->show();
            
            // Create a ClientRect with the same size and position
            ClientRect *clientRect = new ClientRect(frameId, this, this);
            clientRect->resize(400, 400);
            clientRect->move(adjustedPos);
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
                    if (firstFrame) {
                        originalBoundingRect = element->geometry();
                        firstFrame = false;
                    } else {
                        originalBoundingRect = originalBoundingRect.united(element->geometry());
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
        QRect oldGeometry = element->geometry();
        
        // Calculate relative position within the original bounding rect
        qreal relativeX = qreal(oldGeometry.left() - originalBoundingRect.left()) / qreal(originalBoundingRect.width());
        qreal relativeY = qreal(oldGeometry.top() - originalBoundingRect.top()) / qreal(originalBoundingRect.height());
        qreal relativeWidth = qreal(oldGeometry.width()) / qreal(originalBoundingRect.width());
        qreal relativeHeight = qreal(oldGeometry.height()) / qreal(originalBoundingRect.height());
        
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
        
        // Update the element's geometry
        element->setGeometry(QRect(newX, newY, newWidth, newHeight));
        
        // Also update any associated ClientRect
        Frame *frame = qobject_cast<Frame*>(element);
        if (frame) {
            // Find ClientRect with the same ID
            for (QObject *child : children()) {
                ClientRect *clientRect = qobject_cast<ClientRect*>(child);
                if (clientRect && clientRect->getAssociatedElementId() == frame->getId()) {
                    clientRect->setGeometry(QRect(newX, newY, newWidth, newHeight));
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