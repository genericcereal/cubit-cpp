#include "Canvas.h"
#include "Controls.h"
#include "Element.h"
#include "Frame.h"
#include "Text.h"
#include "Variable.h"
#include "ClientRect.h"
#include "ActionsPanel.h"
#include "FPSWidget.h"
#include "SelectionBox.h"
#include <QMouseEvent>
#include <QApplication>
#include <QPainter>
#include <QDebug>

Canvas::Canvas(QWidget *parent) : QWidget(parent), mode("Select"), isSelecting(false), isSimulatingControlDrag(false), panOffset(0, 0) {
    setContentsMargins(0, 0, 0, 0);
    setMouseTracking(true);  // Enable mouse tracking for cursor changes
    
    // Create UI controls directly on Canvas
    controls = new Controls(this);
    controls->hide();  // Initially hidden
    
    // Create selection box
    selectionBox = new SelectionBox(this);
    selectionBox->hide();
    
    // Connect controls signals
    connect(controls, &Controls::rectChanged, this, &Canvas::onControlsRectChanged);
    connect(controls, &Controls::innerRectClicked, this, &Canvas::onControlsInnerRectClicked);
    connect(controls, &Controls::innerRectDoubleClicked, this, &Canvas::onControlsInnerRectDoubleClicked);
    
    // Ensure proper z-ordering: controls first, then selection box on top
    controls->raise();
    selectionBox->raise();
}

void Canvas::resizeEvent(QResizeEvent *event) {
    QWidget::resizeEvent(event);
}

void Canvas::showControls(const QRect &rect) {
    if (controls) {
        controls->setPanOffset(panOffset);
        controls->setZoomScale(zoomScale);
        controls->updateGeometry(rect);
        controls->show();
        controls->raise();
        
        // Keep selection box above controls
        if (selectionBox) {
            selectionBox->raise();
        }
        
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
        // Cancel any active text editing before clearing selection
        cancelActiveTextEditing();
        selectedElements.clear();
    }
    
    // Only add if not already selected
    if (!selectedElements.contains(elementId)) {
        selectedElements.append(elementId);
    }
    
    updateControlsVisibility();
    emit selectionChanged();
}

void Canvas::updateControlsVisibility() {
    // Hide controls if no selection
    if (selectedElements.isEmpty()) {
        cancelActiveTextEditing();
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
                    // Get the visual rect (with zoom applied)
                    QRect visualRect = element->geometry();
                    if (firstFrame) {
                        boundingRect = visualRect;
                        firstFrame = false;
                    } else {
                        // Expand bounding rect to include this frame
                        boundingRect = boundingRect.united(visualRect);
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
            controls->setZoomScale(zoomScale);
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
        if (newMode == "Variable") {
            // When Variable mode is selected, create a variable immediately
            createVariable();
        }
    }
}

void Canvas::setPanOffset(const QPoint &offset) {
    qDebug() << "setPanOffset called with offset:" << offset << "current panOffset:" << panOffset;
    
    // Always update elements when zooming, even if just pan offset changed
    panOffset = offset;
    
    // Update controls pan offset and zoom scale
    if (controls) {
        controls->setPanOffset(panOffset);
        controls->setZoomScale(zoomScale);
    }
    
    qDebug() << "About to update" << elements.size() << "elements";
    
    // Update visual positions of all elements based on their canvas positions
    for (Element *element : elements) {
        qDebug() << "Updating element with panOffset:" << panOffset << "zoomScale:" << zoomScale;
        element->updateVisualPosition(panOffset, zoomScale);
        
        // Also update associated ClientRect if it's a Frame
            if (element->getType() == Element::FrameType) {
                Frame *frame = qobject_cast<Frame*>(element);
                if (frame) {
                    // Find ClientRect with the same ID
                    for (QObject *child : children()) {
                        ClientRect *clientRect = qobject_cast<ClientRect*>(child);
                        if (clientRect && clientRect->getAssociatedElementId() == frame->getId()) {
                            // ClientRect follows the element's geometry
                            clientRect->setGeometry(frame->geometry());
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

void Canvas::createFrame() {
    // Frame creation now happens via mouse drag events
    // This method is kept for API compatibility but does nothing
}

void Canvas::createText() {
    // Generate ID based on number of elements + 1
    int textId = elements.size() + 1;
    
    // Create a new text element with Canvas as parent
    Text *text = new Text(textId, this);
    
    // Set the canvas size (original size)
    text->setCanvasSize(text->size());
    
    // Calculate canvas position (center of viewport in canvas coordinates)
    QPoint canvasPos((width() / 2 - panOffset.x()) / zoomScale - text->width() / 2,
                     (height() / 2 - panOffset.y()) / zoomScale - text->height() / 2);
    text->setCanvasPosition(canvasPos);
    text->updateVisualPosition(panOffset, zoomScale);
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
        if (mode == "Frame" || mode == "Text") {
            // Generate ID based on number of elements + 1
            int frameId = elements.size() + 1;
            
            // Calculate canvas position (logical position without pan and zoom)
            QPoint canvasPos = (event->pos() - panOffset) / zoomScale;
            
            Frame *frame = new Frame(frameId, this);
            frame->resize(5, 5);
            frame->setCanvasSize(QSize(5, 5));
            frame->setCanvasPosition(canvasPos);
            frame->updateVisualPosition(panOffset, zoomScale);
            frame->show();
            
            // Create a ClientRect with the same size and position
            ClientRect *clientRect = new ClientRect(frameId, this, this);
            clientRect->setGeometry(frame->geometry());
            clientRect->show();
            
            // Add frame to elements list (ClientRect is not an Element)
            elements.append(frame);
            
            // If in Text mode, create a Text element inside the frame
            if (mode == "Text") {
                int textId = elements.size() + 1;
                Text *text = new Text(textId, frame);
                text->setText("Text Element");
                text->resize(5, 5);  // Start with same size as frame
                text->setCanvasSize(QSize(5, 5));
                text->setCanvasPosition(QPoint(0, 0));  // Relative to frame
                text->move(0, 0);  // Position at top-left of frame
                text->show();
                
                // Add text to elements list
                elements.append(text);
                
                // Emit signal that a text element was created
                emit elementCreated("Text", text->getName());
            }
            
            // Add to selected elements - always select the frame
            cancelActiveTextEditing();
            selectedElements.clear();  // Clear previous selection
            selectedElements.append(QString::number(frame->getId()));
            updateControlsVisibility();
            emit selectionChanged();
            
            // Ensure controls stay on top
            if (controls) {
                controls->raise();
            }
            
            // Keep selection box above controls
            if (selectionBox) {
                selectionBox->raise();
            }
            
            // Emit signal that a frame was created
            emit elementCreated("Frame", frame->getName());
            
            // Emit signal to raise overlay panels (ActionsPanel, FPSWidget, etc.)
            emit overlaysNeedRaise();
            
            // Switch back to Select mode
            setMode("Select");
            
            // Start dragging the bottom-right corner of the controls
            // The controls are already visible at this point
            if (controls && controls->isVisible()) {
                // Start drag mode directly on the bottom-right joint
                controls->startDragMode(Controls::BottomRightResizeJoint, event->globalPos());
                
                // Store that we're in a simulated drag state
                isSimulatingControlDrag = true;
            }
            
        } else if (mode == "Select") {
            // In Select mode, check if click is on empty canvas
            QWidget *widget = childAt(event->pos());
            
            // If clicked on canvas itself (not on any child widget), start selection box
            if (!widget || widget == this || widget == selectionBox) {
                // Clear selection
                cancelActiveTextEditing();
                selectedElements.clear();
                emit selectionChanged();
                updateControlsVisibility();
                
                // Start selection box
                isSelecting = true;
                selectionBox->startSelection(event->pos());
            }
        }
    }
    
    // Always propagate the event
    QWidget::mousePressEvent(event);
}

void Canvas::mouseMoveEvent(QMouseEvent *event) {
    // If we're simulating control drag, update the controls directly
    if (isSimulatingControlDrag && controls && controls->isVisible()) {
        controls->updateDragPosition(event->globalPos());
        return;
    }
    
    if (isSelecting && selectionBox) {
        // Update selection box
        selectionBox->updateSelection(event->pos());
        
        // Check for overlapping ClientRects
        QRect selectionRect = selectionBox->getSelectionRect();
        cancelActiveTextEditing();
        selectedElements.clear();
        emit selectionChanged();
        
        // Find all ClientRects that intersect with the selection box
        for (QObject *child : children()) {
            ClientRect *clientRect = qobject_cast<ClientRect*>(child);
            if (clientRect) {
                // Get the visual rect of the ClientRect
                QRect clientRectVisual = clientRect->geometry();
                
                // Check if it intersects with the selection box
                if (selectionRect.intersects(clientRectVisual)) {
                    QString elementId = QString::number(clientRect->getAssociatedElementId());
                    if (!selectedElements.contains(elementId)) {
                        selectedElements.append(elementId);
                    }
                }
            }
        }
        
        // Update controls visibility based on new selection
        updateControlsVisibility();
    }
    
    // Use normal event propagation
    QWidget::mouseMoveEvent(event);
}

void Canvas::mouseReleaseEvent(QMouseEvent *event) {
    // If we're simulating control drag, end the drag
    if (isSimulatingControlDrag && controls && controls->isVisible()) {
        controls->endDrag();
        isSimulatingControlDrag = false;  // End the simulated drag
        return;
    }
    
    if (isSelecting && selectionBox) {
        // End selection
        isSelecting = false;
        selectionBox->endSelection();
        
        // Clear the selection box geometry to prevent flickering on next use
        selectionBox->setGeometry(0, 0, 0, 0);
        
        // Keep the final selection
        updateControlsVisibility();
    }
    
    // Use normal event propagation
    QWidget::mouseReleaseEvent(event);
}

void Canvas::wheelEvent(QWheelEvent *event) {
    // Check if Ctrl is held for zooming
    if (event->modifiers() & Qt::ControlModifier) {
        // Get the scroll amount
        QPoint numDegrees = event->angleDelta() / 8;
        
        if (!numDegrees.isNull() && numDegrees.y() != 0) {
            const qreal force = 1.05;
            
            // Get mouse position relative to widget
            QPoint mousePos = event->position().toPoint();
            int x = (mousePos.x() / 2) * 2;  // Round to even number
            int y = (mousePos.y() / 2) * 2;  // Round to even number
            
            if (numDegrees.y() > 0) {
                // Zoom in (scroll up)
                panOffset.setX(x - (x - panOffset.x()) * force);
                panOffset.setY(y - (y - panOffset.y()) * force);
                zoomScale = zoomScale * force;
            } else {
                // Zoom out (scroll down)
                panOffset.setX(x - (x - panOffset.x()) / force);
                panOffset.setY(y - (y - panOffset.y()) / force);
                zoomScale = zoomScale / force;
            }
            
            qDebug() << "Zoom event - new zoomScale:" << zoomScale << "panOffset:" << panOffset;
            qDebug() << "Number of elements:" << elements.size();
            
            // Update all elements with new pan and zoom
            setPanOffset(panOffset);
            
            // Force a repaint to show the zoom changes
            update();
            
            event->accept();
        }
    } else {
        // Regular pan operation
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
    // Convert newRect from visual space to canvas space
    QRect canvasNewRect;
    canvasNewRect.setTopLeft((newRect.topLeft() - panOffset) / zoomScale);
    canvasNewRect.setSize(newRect.size() / zoomScale);
    
    // Get the original bounding rect of selected frames in canvas space
    QRect originalBoundingRect;
    bool firstFrame = true;
    QList<Element*> selectedFrames;
    
    // Collect all selected frames and calculate original bounding rect
    for (const QString &id : selectedElements) {
        for (Element *element : elements) {
            if (QString::number(element->getId()) == id) {
                if (element->getType() == Element::FrameType) {
                    selectedFrames.append(element);
                    QRect canvasRect(element->getCanvasPosition(), element->getCanvasSize());
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
    bool flipX = canvasNewRect.width() < 0;
    bool flipY = canvasNewRect.height() < 0;
    
    // Get the normalized rectangle for proper positioning
    QRect normalizedNewRect = canvasNewRect.normalized();
    
    // Update each selected frame
    for (Element *element : selectedFrames) {
        QRect oldCanvasRect(element->getCanvasPosition(), element->getCanvasSize());
        
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
        int newWidth = qMax(1, qRound(relativeWidth * normalizedNewRect.width()));  // Minimum width of 1
        int newHeight = qMax(1, qRound(relativeHeight * normalizedNewRect.height()));  // Minimum height of 1
        
        // Update the element's canvas position and size
        element->setCanvasPosition(QPoint(newX, newY));
        element->setCanvasSize(QSize(newWidth, newHeight));
        element->resize(newWidth, newHeight);
        element->updateVisualPosition(panOffset, zoomScale);
        
        // Also update any associated ClientRect
        Frame *frame = qobject_cast<Frame*>(element);
        if (frame) {
            // Find ClientRect with the same ID
            for (QObject *child : children()) {
                ClientRect *clientRect = qobject_cast<ClientRect*>(child);
                if (clientRect && clientRect->getAssociatedElementId() == frame->getId()) {
                    clientRect->setGeometry(frame->geometry());
                    break;
                }
            }
        }
    }
    
    // Ensure controls stay on top
    if (controls) {
        controls->raise();
    }
    
    // Keep selection box above controls
    if (selectionBox) {
        selectionBox->raise();
    }
    
    // Emit signal to raise overlay panels
    emit overlaysNeedRaise();
    
    // Emit signal that properties have changed
    emit propertiesChanged();
    
    // Trigger a repaint
    update();
}

void Canvas::onControlsInnerRectClicked(const QPoint &globalPos) {
    // Check if we have an active text editing session
    bool hasActiveTextEditing = false;
    for (Element *element : elements) {
        if (element->getType() == Element::TextType) {
            Text *text = qobject_cast<Text*>(element);
            if (text && text->isEditing()) {
                hasActiveTextEditing = true;
                break;
            }
        }
    }
    
    // If text is being edited, don't process the click
    if (hasActiveTextEditing) {
        return;
    }
    
    // Convert global position to widget-local position
    QPoint widgetPos = mapFromGlobal(globalPos);
    
    // Find which ClientRect is at this position
    ClientRect* clickedClientRect = nullptr;
    for (QObject *child : children()) {
        ClientRect *clientRect = qobject_cast<ClientRect*>(child);
        if (clientRect && clientRect->geometry().contains(widgetPos)) {
            clickedClientRect = clientRect;
            break;
        }
    }
    
    // If a ClientRect was found, select its associated element
    if (clickedClientRect) {
        QString elementId = QString::number(clickedClientRect->getAssociatedElementId());
        selectElement(elementId, false);  // false = don't add to selection, replace it
    } else {
        // No element found under cursor - clear selection
        cancelActiveTextEditing();
        selectedElements.clear();
        emit selectionChanged();
        updateControlsVisibility();
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

void Canvas::onControlsInnerRectDoubleClicked(const QPoint &) {
    qDebug() << "Canvas::onControlsInnerRectDoubleClicked called, selectedElements:" << selectedElements.size();
    
    // Check if only one element is selected
    if (selectedElements.size() == 1) {
        QString selectedId = selectedElements.first();
        qDebug() << "Selected element ID:" << selectedId;
        
        // Find the selected element
        for (Element *element : elements) {
            if (QString::number(element->getId()) == selectedId) {
                qDebug() << "Found element, type:" << element->getType();
                
                // Check if it's a Frame that contains a Text element
                if (element->getType() == Element::FrameType) {
                    Frame *frame = qobject_cast<Frame*>(element);
                    if (frame) {
                        // Look for a Text child element
                        for (QObject *child : frame->children()) {
                            Text *text = qobject_cast<Text*>(child);
                            if (text) {
                                qDebug() << "Found Text element in Frame, starting text editing";
                                // Make the text editable
                                text->startEditing();
                                break;
                            }
                        }
                    }
                }
                break;
            }
        }
    }
}

void Canvas::cancelActiveTextEditing() {
    // Go through all elements and cancel any active text editing
    for (Element *element : elements) {
        if (element->getType() == Element::TextType) {
            Text *text = qobject_cast<Text*>(element);
            if (text && text->isEditing()) {
                text->endEditing(true);  // true = save changes
            }
        }
    }
}

QList<Frame*> Canvas::getSelectedFrames() const {
    QList<Frame*> selectedFrames;
    
    for (const QString &id : selectedElements) {
        // Find the element with this ID
        for (Element *element : elements) {
            if (QString::number(element->getId()) == id) {
                if (element->getType() == Element::FrameType) {
                    Frame *frame = qobject_cast<Frame*>(element);
                    if (frame) {
                        selectedFrames.append(frame);
                    }
                }
                break;
            }
        }
    }
    
    return selectedFrames;
}