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
#include <QVariant>

Canvas::Canvas(QWidget *parent) : QGraphicsView(parent), mode("Select"), isSelecting(false), isSimulatingControlDrag(false) {
    setContentsMargins(0, 0, 0, 0);
    setMouseTracking(true);  // Enable mouse tracking for cursor changes
    
    // Create the graphics scene
    scene = new QGraphicsScene(this);
    setScene(scene);
    
    // Set up the view
    setRenderHint(QPainter::Antialiasing);
    setViewportUpdateMode(QGraphicsView::MinimalViewportUpdate);
    setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setBackgroundBrush(QBrush(QColor(242, 242, 242)));
    
    // Create UI controls as a proxy widget in the scene
    controls = new Controls();
    controls->updateGeometry(QRect(100, 100, 100, 300));
    
    controlsProxy = scene->addWidget(controls);
    // The controls widget positions itself relative to its target rect,
    // so we need to account for that when setting the proxy position
    controlsProxy->setPos(controls->getIntendedPosition());
    controlsProxy->setZValue(1000);  // High z-value to ensure it's on top
    controlsProxy->hide();  // Initially hide controls until something is selected
    
    // Ensure the proxy widget itself is transparent
    controlsProxy->setAutoFillBackground(false);
    
    // Create selection box as a graphics item
    selectionBox = new SelectionBox();
    scene->addItem(selectionBox);
    selectionBox->setVisible(false);
    
    // Connect controls signals
    connect(controls, &Controls::rectChanged, this, &Canvas::onControlsRectChanged);
    connect(controls, &Controls::innerRectClicked, this, &Canvas::onControlsInnerRectClicked);
    connect(controls, &Controls::innerRectDoubleClicked, this, &Canvas::onControlsInnerRectDoubleClicked);
    
}

void Canvas::resizeEvent(QResizeEvent *event) {
    QGraphicsView::resizeEvent(event);
    
    // Update scene rect to match the view
    if (scene) {
        scene->setSceneRect(rect());
    }
}

void Canvas::showControls(const QRect &rect) {
    if (controls && controlsProxy) {
        // FUTURE: Re-enable pan/zoom support
        // controls->setPanOffset(panOffset);
        // controls->setZoomScale(zoomScale);
        controls->updateGeometry(rect);
        
        // The controls widget positions itself with margins, get its intended position
        controlsProxy->setPos(controls->getIntendedPosition());
        controlsProxy->show();
        
        
        // Emit signal to raise overlay panels
        emit overlaysNeedRaise();
    }
}

void Canvas::hideControls() {
    if (controlsProxy) {
        controlsProxy->hide();
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
                    // Get the frame's scene rect
                    QRect frameRect(element->getCanvasPosition(), element->getCanvasSize());
                    if (firstFrame) {
                        boundingRect = frameRect;
                        firstFrame = false;
                    } else {
                        // Expand bounding rect to include this frame
                        boundingRect = boundingRect.united(frameRect);
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
        if (newMode == "Variable") {
            // When Variable mode is selected, create a variable immediately
            createVariable();
        }
    }
}

// FUTURE: Re-enable pan/zoom support
/*
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
*/

void Canvas::createFrame() {
    // Frame creation now happens via mouse drag events
    // This method is kept for API compatibility but does nothing
}

void Canvas::createText() {
    // TEMPORARILY DISABLED - Text creation
    /*
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
    */
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
    // First, let the scene handle the event (this allows the controls to receive it)
    QGraphicsView::mousePressEvent(event);
    
    // Check if the event was handled by a scene item
    if (event->isAccepted()) {
        return;
    }
    
    if (event->button() == Qt::LeftButton) {
        if (mode == "Frame" || mode == "Text") {
            // Generate ID based on number of elements + 1
            int frameId = elements.size() + 1;
            
            // Convert view position to scene position
            QPointF scenePos = mapToScene(event->pos());
            
            // Create a Frame widget without parent for proxy widget
            Frame *frame = new Frame(frameId, nullptr);
            frame->resize(5, 5);
            frame->setCanvasSize(QSize(5, 5));
            frame->setCanvasPosition(scenePos.toPoint());
            
            // If in Text mode, create a Text element inside the frame BEFORE adding to scene
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
            
            // Add Frame to scene as a proxy widget (after adding any children)
            QGraphicsProxyWidget *frameProxy = scene->addWidget(frame);
            frameProxy->setPos(scenePos);
            frameProxy->resize(frame->size());  // Explicitly set proxy size
            frameProxy->setZValue(10);  // Below controls but above background
            
            // Create a ClientRect without parent for proxy widget
            ClientRect *clientRect = new ClientRect(frameId, this, nullptr);
            clientRect->resize(20, 20);  // Match the frame size
            
            // Add ClientRect to scene as a proxy widget
            QGraphicsProxyWidget *clientProxy = scene->addWidget(clientRect);
            clientProxy->setPos(scenePos);
            clientProxy->resize(clientRect->size());  // Explicitly set proxy size
            clientProxy->setZValue(11);  // Slightly above frame to handle mouse events
            
            // Store proxy references (we'll need to add these as member variables)
            frame->setProperty("proxy", QVariant::fromValue(frameProxy));
            clientRect->setProperty("proxy", QVariant::fromValue(clientProxy));
            
            // Add frame to elements list (ClientRect is not an Element)
            elements.append(frame);
            
            // Add to selected elements - always select the frame
            cancelActiveTextEditing();
            selectedElements.clear();  // Clear previous selection
            selectedElements.append(QString::number(frame->getId()));
            updateControlsVisibility();
            emit selectionChanged();
            
            // Emit signal that a frame was created
            emit elementCreated("Frame", frame->getName());
            
            // Emit signal to raise overlay panels (ActionsPanel, FPSWidget, etc.)
            emit overlaysNeedRaise();
            
            // Switch back to Select mode
            setMode("Select");
            
            // Start dragging the bottom-right corner of the controls
            // Force showing the controls and start the drag
            if (controls && controlsProxy) {
                // Ensure controls are visible (updateControlsVisibility might not have taken effect yet)
                if (!controlsProxy->isVisible()) {
                    controlsProxy->show();
                }
                
                // Get the current rect of the controls (should be the frame's position)
                QRect frameRect(frame->getCanvasPosition(), frame->getCanvasSize());
                
                // For synthetic drag, we start from the current mouse position
                // The Controls widget will calculate the delta from this position
                controls->startDragMode(Controls::BottomRightResizeJoint, event->globalPos());
                
                // Store that we're in a simulated drag state
                isSimulatingControlDrag = true;
            }
            
        } else if (mode == "Select") {
            QPointF scenePos = mapToScene(event->pos());
            
            // Check if there's a ClientRect under the cursor
            QList<QGraphicsItem*> itemsAtPos = scene->items(scenePos);
            ClientRect* clickedClientRect = nullptr;
            
            for (QGraphicsItem *item : itemsAtPos) {
                QGraphicsProxyWidget *proxy = qgraphicsitem_cast<QGraphicsProxyWidget*>(item);
                if (proxy) {
                    ClientRect *clientRect = qobject_cast<ClientRect*>(proxy->widget());
                    if (clientRect) {
                        clickedClientRect = clientRect;
                        break;
                    }
                }
            }
            
            if (clickedClientRect) {
                // Select the frame associated with this ClientRect
                QString elementId = QString::number(clickedClientRect->getAssociatedElementId());
                
                // Check if shift is held for multi-selection
                bool addToSelection = (event->modifiers() & Qt::ShiftModifier);
                selectElement(elementId, addToSelection);
            } else {
                // Start selection box on empty canvas
                isSelecting = true;
                selectionBox->startSelection(scenePos);
            }
        }
    }
}

void Canvas::mouseMoveEvent(QMouseEvent *event) {
    // If we're simulating control drag, handle it first before letting the scene process events
    if (isSimulatingControlDrag && controls) {
        controls->updateDragPosition(event->globalPos());
        event->accept();  // Mark the event as handled
        return;
    }
    
    // Handle selection box dragging before passing to scene
    if (isSelecting && selectionBox) {
        QPointF scenePos = mapToScene(event->pos());
        selectionBox->updateSelection(scenePos);
        
        // Update selection in real-time
        QRectF selectionRect = selectionBox->getSelectionRect();
        if (selectionRect.width() > 2 || selectionRect.height() > 2) {
            // Find all items within the selection rect
            QList<QGraphicsItem*> itemsInRect = scene->items(selectionRect, Qt::IntersectsItemShape);
            
            // Clear previous selection
            cancelActiveTextEditing();
            selectedElements.clear();
            
            // Add frames to selection
            for (QGraphicsItem *item : itemsInRect) {
                QGraphicsProxyWidget *proxy = qgraphicsitem_cast<QGraphicsProxyWidget*>(item);
                if (proxy) {
                    // Check if it's a ClientRect (which is what we use for selection)
                    ClientRect *clientRect = qobject_cast<ClientRect*>(proxy->widget());
                    if (clientRect) {
                        QString elementId = QString::number(clientRect->getAssociatedElementId());
                        if (!selectedElements.contains(elementId)) {
                            selectedElements.append(elementId);
                        }
                    }
                }
            }
            
            // Update controls visibility and emit selection changed
            updateControlsVisibility();
            emit selectionChanged();
        }
        
        event->accept();  // Don't let the scene handle this event
        return;
    }
    
    // Otherwise, let the scene handle the event
    QGraphicsView::mouseMoveEvent(event);
    
    // Check if the event was handled by a scene item
    if (event->isAccepted()) {
        return;
    }
}

void Canvas::mouseReleaseEvent(QMouseEvent *event) {
    // If we're simulating control drag, handle it first
    if (isSimulatingControlDrag && controls) {
        controls->endDrag();
        isSimulatingControlDrag = false;  // End the simulated drag
        event->accept();
        return;
    }
    
    // Handle selection box release BEFORE passing to scene
    if (isSelecting && selectionBox) {
        isSelecting = false;
        
        // Get the final selection rect
        QRectF selectionRect = selectionBox->getSelectionRect();
        
        // End the selection box visual
        selectionBox->endSelection();
        
        // If it was just a click (no drag), clear the selection
        if (selectionRect.width() <= 2 && selectionRect.height() <= 2) {
            cancelActiveTextEditing();
            selectedElements.clear();
            updateControlsVisibility();
            emit selectionChanged();
        }
        
        event->accept();  // Don't let the scene handle this
        return;
    }
    
    // Otherwise, let the scene handle the event
    QGraphicsView::mouseReleaseEvent(event);
    
    // Check if the event was handled by a scene item
    if (event->isAccepted()) {
        return;
    }
}

void Canvas::wheelEvent(QWheelEvent *event) {
    // FUTURE: Re-enable pan/zoom support
    /*
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
    */
    
    // For now, just pass the event to the base class
    QGraphicsView::wheelEvent(event);
}

void Canvas::paintEvent(QPaintEvent *event) {
    // QGraphicsView handles all painting through the scene
    QGraphicsView::paintEvent(event);
}

void Canvas::render() {
    update();  // Triggers paintEvent
}

void Canvas::onControlsRectChanged(const QRect &newRect) {
    // When controls are dragged/resized, update the proxy position
    if (controlsProxy && controls) {
        // The controls widget stores its intended position
        controlsProxy->setPos(controls->getIntendedPosition());
    }
    
    // The newRect is already in scene coordinates
    QRect sceneRect = newRect;
    
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
                    QRect frameRect(element->getCanvasPosition(), element->getCanvasSize());
                    if (firstFrame) {
                        originalBoundingRect = frameRect;
                        firstFrame = false;
                    } else {
                        originalBoundingRect = originalBoundingRect.united(frameRect);
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
    bool flipX = sceneRect.width() < 0;
    bool flipY = sceneRect.height() < 0;
    
    // Get the normalized rectangle for proper positioning
    QRect normalizedNewRect = sceneRect.normalized();
    
    // Update each selected frame
    for (Element *element : selectedFrames) {
        Frame *frame = qobject_cast<Frame*>(element);
        if (!frame) continue;
        
        QRect oldRect(element->getCanvasPosition(), element->getCanvasSize());
        
        // Calculate relative position within the original bounding rect
        qreal relativeX = qreal(oldRect.left() - originalBoundingRect.left()) / qreal(originalBoundingRect.width());
        qreal relativeY = qreal(oldRect.top() - originalBoundingRect.top()) / qreal(originalBoundingRect.height());
        qreal relativeWidth = qreal(oldRect.width()) / qreal(originalBoundingRect.width());
        qreal relativeHeight = qreal(oldRect.height()) / qreal(originalBoundingRect.height());
        
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
        
        // Update the frame's canvas position and size
        frame->setCanvasPosition(QPoint(newX, newY));
        frame->setCanvasSize(QSize(newWidth, newHeight));
        frame->resize(newWidth, newHeight);
        
        // Update the frame proxy position and size
        QGraphicsProxyWidget *frameProxy = frame->property("proxy").value<QGraphicsProxyWidget*>();
        if (frameProxy) {
            frameProxy->setPos(newX, newY);
            frameProxy->resize(newWidth, newHeight);
        }
        
        // Find and update associated ClientRect
        // Look through all graphics items in the scene
        QList<QGraphicsItem*> items = scene->items();
        for (QGraphicsItem *item : items) {
            QGraphicsProxyWidget *proxy = qgraphicsitem_cast<QGraphicsProxyWidget*>(item);
            if (proxy) {
                ClientRect *clientRect = qobject_cast<ClientRect*>(proxy->widget());
                if (clientRect && clientRect->getAssociatedElementId() == frame->getId()) {
                    clientRect->resize(newWidth, newHeight);
                    proxy->setPos(newX, newY);
                    proxy->resize(newWidth, newHeight);
                    break;
                }
            }
        }
    }
    
    // Emit signal that properties have changed
    emit propertiesChanged();
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
    
    // Convert global position to scene position
    QPointF scenePos = mapToScene(mapFromGlobal(globalPos));
    
    // Find all items at this position in the scene
    QList<QGraphicsItem *> itemsAtPos = scene->items(scenePos);
    
    // Look for a ClientRect in the items (excluding the controls themselves)
    ClientRect* clickedClientRect = nullptr;
    for (QGraphicsItem *item : itemsAtPos) {
        QGraphicsProxyWidget *proxy = qgraphicsitem_cast<QGraphicsProxyWidget*>(item);
        if (proxy && proxy->widget()) {
            ClientRect *clientRect = qobject_cast<ClientRect*>(proxy->widget());
            if (clientRect) {
                clickedClientRect = clientRect;
                break;  // Found a ClientRect, stop looking
            }
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