#include "Canvas.h"
#include "Controls.h"
#include "Element.h"
#include "Frame.h"
#include "Text.h"
#include "Variable.h"
#include "ClientRect.h"
#include <QMouseEvent>
#include <QApplication>

Canvas::Canvas(QWidget *parent) : QWidget(parent), mode("Select") {

    setContentsMargins(0, 0, 0, 0);
    setMouseTracking(true);  // Enable mouse tracking for cursor changes
    
    // Create UI controls directly on Canvas
    controls = new Controls(this);
    controls->hide();  // Initially hidden
    
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

void Canvas::createFrame() {
    // Frame creation now happens via mouse drag events
    // This method is kept for API compatibility but does nothing
}

void Canvas::createText() {
    // Generate ID based on number of elements + 1
    int textId = elements.size() + 1;
    
    // Create a new text element with Canvas as parent
    Text *text = new Text(textId, this);
    
    // Position it at a default location or center it
    int textX = (width() - text->width()) / 2;
    int textY = (height() - text->height()) / 2;
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
            Frame *frame = new Frame(frameId, this);
            frame->resize(400, 400);
            frame->move(event->pos());
            frame->show();
            
            // Create a ClientRect with the same size and position
            ClientRect *clientRect = new ClientRect(frameId, this, this);
            clientRect->resize(400, 400);
            clientRect->move(event->pos());
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