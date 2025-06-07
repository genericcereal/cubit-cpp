#include "Canvas.h"
#include "Config.h"
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
#include <QPainterPath>
#include <QVariant>
#include <QScrollBar>
#include <QDebug>
#include <QCursor>

Canvas::Canvas(QWidget *parent) : QGraphicsView(parent), mode("Select"), isSelecting(false), isSimulatingControlDrag(false), isPanning(false), hoveredFrame(nullptr) {
    setContentsMargins(0, 0, 0, 0);
    setMouseTracking(true);  // Enable mouse tracking for cursor changes
    
    // Create the graphics scene with a larger initial size to allow panning
    scene = new QGraphicsScene(this);
    scene->setSceneRect(Config::Scene::MIN_X, Config::Scene::MIN_Y, 
                       Config::Scene::WIDTH, Config::Scene::HEIGHT);  // Large scene to allow panning
    setScene(scene);
    
    // Set up the view
    setRenderHint(QPainter::Antialiasing);
    setViewportUpdateMode(QGraphicsView::MinimalViewportUpdate);
    setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setBackgroundBrush(QBrush(QColor(Config::Colors::CANVAS_BG_R, 
                                     Config::Colors::CANVAS_BG_G, 
                                     Config::Colors::CANVAS_BG_B)));
    
    // IMPORTANT: Set alignment to top-left so elements don't move when window is resized
    setAlignment(Qt::AlignLeft | Qt::AlignTop);
    
    // Create UI controls as a proxy widget in the scene
    controls = new Controls();
    controls->updateGeometry(QRect(100, 100, 100, 300));
    
    controlsProxy = scene->addWidget(controls);
    // The controls widget positions itself relative to its target rect,
    // so we need to account for that when setting the proxy position
    controlsProxy->setPos(controls->getIntendedPosition());
    controlsProxy->setZValue(Config::ZIndex::CONTROLS);  // High z-value to ensure it's on top
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
    // Elements stay in their absolute scene positions
}

void Canvas::showControls(const QRect &rect) {
    if (controls && controlsProxy) {
        // Set the current zoom scale so controls can properly scale mouse deltas
        controls->setZoomScale(zoomScale);
        controls->updateGeometry(rect);
        
        // The controls widget positions itself with margins, get its intended position
        controlsProxy->setPos(controls->getIntendedPosition());
        // Don't show the proxy - we draw in foreground instead
        controlsProxy->hide();
        
        // Update scene rect for foreground drawing
        controlsSceneRect = rect;
        controlsVisible = true;
        
        // Trigger viewport update
        viewport()->update();
        
        // Emit signal to raise overlay panels
        emit overlaysNeedRaise();
    }
}

void Canvas::hideControls() {
    if (controlsProxy) {
        controlsProxy->hide();
    }
    controlsVisible = false;
    controlsSceneRect = QRectF();
    viewport()->update();
}

void Canvas::selectElement(const QString &elementId, bool addToSelection) {
    if (!addToSelection) {
        // Cancel any active text editing before clearing selection
        cancelActiveTextEditing();
        selectedElements.clear();
    }
    
    // Clear hovered element when making a selection
    setHoveredElement("");
    
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
        controlsVisible = false;
        viewport()->update();  // Redraw to remove controls
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
    
    // Update controls state
    if (hasFrame) {
        controlsSceneRect = boundingRect;
        controlsVisible = true;
        
        // Update the hidden Controls widget position for event handling
        if (controls && controlsProxy) {
            controls->setZoomScale(zoomScale);
            controls->updateGeometry(boundingRect);
            controlsProxy->setPos(controls->getIntendedPosition());
            // Keep the proxy hidden - we'll draw in foreground instead
            controlsProxy->hide();
            
            // Make sure the controls widget itself is shown (just the proxy is hidden)
            controls->show();
        }
        
        // Emit signal to raise overlay panels
        emit overlaysNeedRaise();
    } else {
        controlsVisible = false;
        controlsSceneRect = QRectF();  // Clear the scene rect
        hideControls();
    }
    
    viewport()->update();  // Redraw to show/update controls
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



Frame* Canvas::createFrameElement(const QPoint &scenePos, bool withText) {
    // Generate unique ID for the frame
    int frameId = getNextElementId();
    
    // Create a Frame widget without parent for proxy widget
    Frame *frame = new Frame(frameId, nullptr);
    
    // Create initial rect with integer position and size
    QRectF initialRect(scenePos, QSizeF(Config::ElementDefaults::INITIAL_SIZE, Config::ElementDefaults::INITIAL_SIZE));
    QRectF alignedRect = alignRectToPixels(initialRect);
    
    frame->resize(alignedRect.size().toSize());
    frame->setCanvasSize(alignedRect.size().toSize());
    frame->setCanvasPosition(alignedRect.topLeft().toPoint());
    
    // Add frame to elements list first (before creating Text to get correct ID)
    elements.append(frame);
    
    // Connect overflow change signal
    connect(frame, &Frame::overflowChanged, [this, frame]() {
        updateChildClipping(frame);
    });
    
    // If withText is true, create a Text element inside the frame
    if (withText) {
        int textId = getNextElementId();
        Text *text = new Text(textId, frame);
        text->setText("Text Element");
        text->resize(Config::ElementDefaults::INITIAL_SIZE, Config::ElementDefaults::INITIAL_SIZE);  // Start with same size as frame
        text->setCanvasSize(QSize(Config::ElementDefaults::INITIAL_SIZE, Config::ElementDefaults::INITIAL_SIZE));
        text->setCanvasPosition(QPoint(0, 0));  // Relative to frame
        text->move(0, 0);  // Position at top-left of frame
        text->show();
        
        // Set the parent-child relationship
        text->setParentElementId(frameId);
        
        // Add text to elements list
        elements.append(text);
        
        // Emit signal that a text element was created
        emit elementCreated("Text", text->getName());
    }
    
    // Add Frame to scene as a proxy widget (after adding any children)
    QGraphicsProxyWidget *frameProxy = scene->addWidget(frame);
    frameProxy->setPos(alignedRect.topLeft());
    frameProxy->resize(alignedRect.size().toSize());
    frameProxy->setZValue(Config::ZIndex::FRAME);  // Below controls but above background
    
    // Create a ClientRect without parent for proxy widget
    ClientRect *clientRect = new ClientRect(frameId, this, nullptr);
    clientRect->resize(alignedRect.size().toSize());
    
    // Add ClientRect to scene as a proxy widget
    QGraphicsProxyWidget *clientProxy = scene->addWidget(clientRect);
    clientProxy->setPos(alignedRect.topLeft());
    clientProxy->resize(alignedRect.size().toSize());
    clientProxy->setZValue(Config::ZIndex::CLIENT_RECT);  // Slightly above frame to handle mouse events
    
    // Store proxy references
    frame->setProperty("proxy", QVariant::fromValue(frameProxy));
    clientRect->setProperty("proxy", QVariant::fromValue(clientProxy));
    
    // Add to selected elements - always select the frame
    cancelActiveTextEditing();
    selectedElements.clear();  // Clear previous selection
    selectedElements.append(QString::number(frame->getId()));
    updateControlsVisibility();
    emit selectionChanged();
    
    // Emit signal that a frame was created
    emit elementCreated("Frame", frame->getName());
    
    return frame;
}

void Canvas::createVariable() {
    // Generate unique ID for the variable
    int variableId = getNextElementId();
    
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
    // Check if clicking on a control element first
    if (event->button() == Qt::LeftButton && controlsVisible) {
        Controls::DragMode controlElement = getControlElementAt(event->pos());
        if (controlElement != Controls::None) {
            // Start control dragging
            if (controls) {
                // When using scene coordinates, we don't need zoom scaling
                controls->setZoomScale(1.0);
                controls->updateGeometry(controlsSceneRect.toRect());
                
                // Convert viewport position to scene position
                QPointF scenePos = mapToScene(event->pos());
                // Use scene position as "global" position for Controls widget
                // This ensures the drag deltas are calculated in scene coordinates
                controls->startDragMode(controlElement, scenePos.toPoint());
                isSimulatingControlDrag = true;
            }
            event->accept();
            return;
        }
    }
    
    // First, let the scene handle the event (this allows the controls to receive it)
    QGraphicsView::mousePressEvent(event);
    
    // Check if the event was handled by a scene item
    if (event->isAccepted()) {
        return;
    }
    
    if (event->button() == Qt::LeftButton) {
        if (mode == "Frame" || mode == "Text") {
            // Convert view position to scene position and round to integers
            QPointF scenePosF = mapToScene(event->pos());
            QPoint scenePos(qRound(scenePosF.x()), qRound(scenePosF.y()));
            
            // Create frame element (with text if in Text mode)
            Frame *frame = createFrameElement(scenePos, mode == "Text");
            
            // Emit signal to raise overlay panels (ActionsPanel, FPSWidget, etc.)
            emit overlaysNeedRaise();
            
            // Switch back to Select mode
            setMode("Select");
            
            // Start dragging the bottom-right corner of the controls
            // Force showing the controls and start the drag
            if (frame && controls && controlsProxy) {
                // Get the frame's rect for the controls
                QRect frameRect(frame->getCanvasPosition(), frame->getCanvasSize());
                
                // When using scene coordinates, we don't need zoom scaling
                controls->setZoomScale(1.0);
                controls->updateGeometry(frameRect);
                
                // Convert viewport position to scene position
                QPointF scenePos = mapToScene(event->pos());
                // Use scene position as "global" position for Controls widget
                controls->startDragMode(Controls::BottomRightResizeJoint, scenePos.toPoint());
                
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
        // Convert viewport position to scene position
        QPointF scenePos = mapToScene(event->pos());
        // Use scene position as "global" position for Controls widget
        controls->updateDragPosition(scenePos.toPoint());
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
    
    // Check if mouse is over any ClientRect - if not, clear hover
    // Do this before letting scene handle the event to ensure hover is always updated
    QPointF scenePos = mapToScene(event->pos());
    QList<QGraphicsItem*> itemsUnderMouse = scene->items(scenePos);
    
    int hoveredElementId = -1;
    
    // If we have selected elements, look for non-selected elements under the cursor
    if (!selectedElements.isEmpty()) {
        for (QGraphicsItem *item : itemsUnderMouse) {
            QGraphicsProxyWidget *proxy = qgraphicsitem_cast<QGraphicsProxyWidget*>(item);
            if (proxy) {
                ClientRect *clientRect = qobject_cast<ClientRect*>(proxy->widget());
                if (clientRect) {
                    int elementId = clientRect->getAssociatedElementId();
                    // Skip if this element is selected
                    if (!selectedElements.contains(QString::number(elementId))) {
                        hoveredElementId = elementId;
                        break;
                    }
                }
            }
        }
    } else {
        // No selection, just find the first ClientRect
        for (QGraphicsItem *item : itemsUnderMouse) {
            QGraphicsProxyWidget *proxy = qgraphicsitem_cast<QGraphicsProxyWidget*>(item);
            if (proxy) {
                ClientRect *clientRect = qobject_cast<ClientRect*>(proxy->widget());
                if (clientRect) {
                    hoveredElementId = clientRect->getAssociatedElementId();
                    break;
                }
            }
        }
    }
    
    // Update the hovered element based on what we found
    if (hoveredElementId > 0) {
        setHoveredElement(QString::number(hoveredElementId));
    } else {
        setHoveredElement(QString());
    }
    
    // Check if mouse is over a control element and update cursor
    Controls::DragMode controlElement = getControlElementAt(event->pos());
    if (controlElement != hoveredControlElement) {
        hoveredControlElement = controlElement;
        
        // Update cursor based on control element
        switch (controlElement) {
            case Controls::LeftBar:
            case Controls::RightBar:
                setCursor(Qt::SizeHorCursor);
                break;
            case Controls::TopBar:
            case Controls::BottomBar:
                setCursor(Qt::SizeVerCursor);
                break;
            case Controls::TopLeftResizeJoint:
            case Controls::BottomRightResizeJoint:
                setCursor(Qt::SizeFDiagCursor);
                break;
            case Controls::TopRightResizeJoint:
            case Controls::BottomLeftResizeJoint:
                setCursor(Qt::SizeBDiagCursor);
                break;
            case Controls::InnerRect:
                setCursor(Qt::SizeAllCursor);
                break;
            default:
                unsetCursor();
                break;
        }
    }
    
    // Otherwise, let the scene handle the event
    QGraphicsView::mouseMoveEvent(event);
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
    // Check if Ctrl is held for zooming
    if (event->modifiers() & Qt::ControlModifier) {
        // Zooming
        QPoint numDegrees = event->angleDelta() / 8;
        
        if (!numDegrees.isNull()) {
            qreal scaleFactor = 1.0;
            // Positive = zoom in, negative = zoom out
            if (numDegrees.y() > 0) {
                scaleFactor = 1.1;  // Zoom in by 10%
            } else {
                scaleFactor = 0.9;  // Zoom out by 10%
            }
            
            // Get the position of the mouse in the view
            QPointF mousePos = mapToScene(event->position().toPoint());
            
            // Apply the scale transformation
            scale(scaleFactor, scaleFactor);
            
            // Update our zoom scale tracking
            zoomScale *= scaleFactor;
            // Round to nearest tenth
            zoomScale = qRound(zoomScale * 10.0) / 10.0;
            
            // Update controls zoom scale if they're visible
            if (controls && controlsVisible) {
                controls->setZoomScale(zoomScale);
                // Force controls to update their geometry to account for new zoom
                updateControlsVisibility();
            }
            
            // Trigger viewport update to redraw hover outlines with new zoom
            viewport()->update();
            
            // Ensure the mouse position stays at the same scene coordinate
            // This makes zooming feel natural - zoom centers on mouse position
            centerOn(mousePos);
            QPointF delta = mapToScene(event->position().toPoint()) - mousePos;
            centerOn(mapToScene(viewport()->rect().center()) - delta);
            
            event->accept();
        }
    } else {
        // Regular scrolling (panning)
        QPoint numPixels = event->pixelDelta();
        QPoint numDegrees = event->angleDelta() / 8;
        
        int deltaY = 0;
        if (!numPixels.isNull()) {
            deltaY = numPixels.y();
        } else if (!numDegrees.isNull()) {
            deltaY = numDegrees.y() * 3;  // Convert degrees to pixels (approximate)
        }
        
        if (deltaY != 0) {
            // Get the current scrollbar values
            int currentY = verticalScrollBar()->value();
            
            // Invert the delta for natural scrolling (scroll up = content moves up = scrollbar value decreases)
            verticalScrollBar()->setValue(currentY - deltaY);
            
            event->accept();
        } else {
            // If no vertical scroll, pass to base class
            QGraphicsView::wheelEvent(event);
        }
    }
}

void Canvas::paintEvent(QPaintEvent *event) {
    // QGraphicsView handles all painting through the scene
    QGraphicsView::paintEvent(event);
}

void Canvas::render() {
    update();  // Triggers paintEvent
}

void Canvas::resetZoom() {
    // Reset the view transformation to identity
    resetTransform();
    zoomScale = 1.0;
    
    // Center the view on the origin
    centerOn(0, 0);
}

void Canvas::onControlsRectChanged(const QRect &newRect) {
    // When controls are dragged/resized, update the proxy position
    if (controlsProxy && controls) {
        // The controls widget stores its intended position
        controlsProxy->setPos(controls->getIntendedPosition());
    }
    
    // Update the controls scene rect for foreground drawing
    controlsSceneRect = newRect;
    
    // Trigger viewport update to redraw controls in new position
    viewport()->update();
    
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
        
        // Create new rect and align it to pixel boundaries
        QRectF newRect(newX, newY, newWidth, newHeight);
        QRectF alignedRect = alignRectToPixels(newRect);
        
        // Calculate the movement delta for children
        QPoint oldPos = element->getCanvasPosition();
        QPoint delta = alignedRect.topLeft().toPoint() - oldPos;
        
        // Update the frame's canvas position and size
        frame->setCanvasPosition(alignedRect.topLeft().toPoint());
        frame->setCanvasSize(alignedRect.size().toSize());
        frame->resize(alignedRect.size().toSize());
        
        // Update the frame proxy position and size
        QGraphicsProxyWidget *frameProxy = frame->property("proxy").value<QGraphicsProxyWidget*>();
        if (frameProxy) {
            frameProxy->setPos(alignedRect.topLeft());
            frameProxy->resize(alignedRect.size().toSize());
        }
        
        // Find and update associated ClientRect
        // Look through all graphics items in the scene
        QList<QGraphicsItem*> items = scene->items();
        for (QGraphicsItem *item : items) {
            QGraphicsProxyWidget *proxy = qgraphicsitem_cast<QGraphicsProxyWidget*>(item);
            if (proxy) {
                ClientRect *clientRect = qobject_cast<ClientRect*>(proxy->widget());
                if (clientRect && clientRect->getAssociatedElementId() == frame->getId()) {
                    clientRect->resize(alignedRect.size().toSize());
                    proxy->setPos(alignedRect.topLeft());
                    proxy->resize(alignedRect.size().toSize());
                    break;
                }
            }
        }
        
        // Move all child elements with the parent
        moveChildElements(element, delta);
        
        // Update clipping for this element's children
        updateChildClipping(element);
    }
    
    // Also update clipping for elements whose parents might have moved
    for (Element *element : selectedFrames) {
        if (element->hasParent()) {
            Element* parent = getElementById(element->getParentElementId());
            if (parent) {
                updateChildClipping(parent);
            }
        }
    }
    
    // Emit signal that properties have changed
    emit propertiesChanged();
    
    // Check for parenting changes during drag
    // Use the actual cursor position to determine which frame we're over
    QPoint currentMousePos = QCursor::pos();
    QPoint viewMousePos = mapFromGlobal(currentMousePos);
    
    Frame* frameUnderCursor = findFrameAtPosition(viewMousePos);
    
    // Process each selected element for parenting
    for (const QString &selectedId : selectedElements) {
        Element* selectedElement = getElementById(selectedId.toInt());
        if (!selectedElement) continue;
        
        // Skip if the element is a variable (variables don't get parented through dragging)
        if (selectedElement->getType() == Element::VariableType) continue;
        
        // Skip if we're hovering over the selected element itself
        if (frameUnderCursor && frameUnderCursor->getId() == selectedElement->getId()) {
            continue;  // Check next selected element instead of clearing frameUnderCursor
        }
        
        // Skip if trying to parent a frame to its own child (prevent circular relationships)
        if (frameUnderCursor && selectedElement->getType() == Element::FrameType) {
            Element* parent = frameUnderCursor;
            while (parent && parent->hasParent()) {
                if (parent->getParentElementId() == selectedElement->getId()) {
                    frameUnderCursor = nullptr;  // Can't parent to descendant
                    break;
                }
                parent = getElementById(parent->getParentElementId());
            }
        }
        
        int currentParentId = selectedElement->getParentElementId();
        int newParentId = frameUnderCursor ? frameUnderCursor->getId() : -1;
        
        // Update parenting if it changed
        if (currentParentId != newParentId) {
            setElementParent(selectedElement->getId(), newParentId);
        }
    }
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
        
        // Check for parenting changes during drag
        QPoint viewPos = mapFromGlobal(globalPos);
        Frame* frameUnderCursor = findFrameAtPosition(viewPos);
        
        // Process each selected element
        for (const QString &selectedId : selectedElements) {
            Element* selectedElement = getElementById(selectedId.toInt());
            if (!selectedElement) continue;
            
            // Skip if the element is a variable (variables don't get parented through dragging)
            if (selectedElement->getType() == Element::VariableType) continue;
            
            // Skip if we're hovering over the selected element itself
            if (frameUnderCursor && frameUnderCursor->getId() == selectedElement->getId()) {
                frameUnderCursor = nullptr;
            }
            
            // Skip if trying to parent a frame to its own child (prevent circular relationships)
            if (frameUnderCursor && selectedElement->getType() == Element::FrameType) {
                Element* parent = frameUnderCursor;
                while (parent && parent->hasParent()) {
                    if (parent->getParentElementId() == selectedElement->getId()) {
                        frameUnderCursor = nullptr;  // Can't parent to descendant
                        break;
                    }
                    parent = getElementById(parent->getParentElementId());
                }
            }
            
            int currentParentId = selectedElement->getParentElementId();
            int newParentId = frameUnderCursor ? frameUnderCursor->getId() : -1;
            
            // Update parenting if it changed
            if (currentParentId != newParentId) {
                setElementParent(selectedElement->getId(), newParentId);
            }
        }
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
    // Check if only one element is selected
    if (selectedElements.size() == 1) {
        QString selectedId = selectedElements.first();
        
        // Find the selected element
        for (Element *element : elements) {
            if (QString::number(element->getId()) == selectedId) {
                // Check if it's a Frame that contains a Text element
                if (element->getType() == Element::FrameType) {
                    Frame *frame = qobject_cast<Frame*>(element);
                    if (frame) {
                        // Look for a Text child element
                        for (QObject *child : frame->children()) {
                            Text *text = qobject_cast<Text*>(child);
                            if (text) {
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

void Canvas::setHoveredElement(const QString &elementId) {
    if (hoveredElement != elementId) {
        hoveredElement = elementId;
        emit hoveredElementChanged(elementId);
        
        // Update hovered frame pointer
        hoveredFrame = nullptr;
        
        if (!elementId.isEmpty()) {
            // Find the element
            for (Element *element : elements) {
                if (QString::number(element->getId()) == elementId) {
                    if (element->getType() == Element::FrameType) {
                        hoveredFrame = qobject_cast<Frame*>(element);
                    }
                    break;
                }
            }
        }
        
        // Trigger viewport update to redraw hover outline
        viewport()->update();
    }
}

void Canvas::setElementParent(int childId, int parentId) {
    Element* child = getElementById(childId);
    if (child) {
        // Verify that text and variable elements cannot have children
        Element* parent = getElementById(parentId);
        if (parent && (parent->getType() == Element::TextType || parent->getType() == Element::VariableType)) {
            // Don't allow text or variable elements to have children
            return;
        }
        
        // If unparenting (parentId is -1 or invalid), clear any clipping
        if (parentId <= 0 || !parent) {
            // Find the child's proxy widget and clear its mask
            QList<QGraphicsItem*> items = scene->items();
            for (QGraphicsItem *item : items) {
                QGraphicsProxyWidget *proxy = qgraphicsitem_cast<QGraphicsProxyWidget*>(item);
                if (proxy) {
                    if (child->getType() == Element::FrameType) {
                        Frame* frame = qobject_cast<Frame*>(proxy->widget());
                        if (frame && frame->getId() == child->getId()) {
                            // Clear the mask and ensure visibility
                            proxy->setVisible(true);
                            QWidget* widget = proxy->widget();
                            if (widget) {
                                widget->clearMask();
                            }
                            break;
                        }
                    }
                }
            }
        }
        
        child->setParentElementId(parentId);
        
        // If we have a valid new parent, update clipping based on parent's overflow
        if (parent && parent->getType() == Element::FrameType) {
            updateChildClipping(parent);
        }
    }
}

QList<Element*> Canvas::getChildElements(int parentId) const {
    QList<Element*> children;
    for (Element* element : elements) {
        if (element->getParentElementId() == parentId) {
            children.append(element);
        }
    }
    return children;
}

Element* Canvas::getElementById(int id) const {
    for (Element* element : elements) {
        if (element->getId() == id) {
            return element;
        }
    }
    return nullptr;
}

Frame* Canvas::findFrameAtPosition(const QPoint &pos) const {
    // Convert the position to scene coordinates
    QPointF scenePos = mapToScene(pos);
    
    // Check all frames to see if the position is inside any of them
    // Start from the end (top-most elements) and work backwards
    for (int i = elements.size() - 1; i >= 0; i--) {
        Element* element = elements[i];
        if (element->getType() == Element::FrameType) {
            Frame* frame = qobject_cast<Frame*>(element);
            if (frame) {
                // Skip if this frame is currently selected (being dragged)
                bool isSelected = false;
                for (const QString& selectedId : selectedElements) {
                    if (selectedId.toInt() == frame->getId()) {
                        isSelected = true;
                        break;
                    }
                }
                if (isSelected) continue;
                
                // Get the frame's bounding rectangle in scene coordinates
                QPointF framePos = frame->getCanvasPosition();
                QSizeF frameSize = frame->getCanvasSize();
                QRectF frameRect(framePos, frameSize);
                
                if (frameRect.contains(scenePos)) {
                    return frame;
                }
            }
        }
    }
    return nullptr;
}

void Canvas::moveChildElements(Element* parent, const QPoint& delta) {
    if (!parent) return;
    
    // Get all children of this parent
    QList<Element*> children = getChildElements(parent->getId());
    
    // Move each child
    for (Element* child : children) {
        if (!child) continue;
        
        // Get current position
        QPoint currentPos = child->getCanvasPosition();
        QPoint newPos = currentPos + delta;
        
        // Update the element's position
        child->setCanvasPosition(newPos);
        
        // Update visual representation based on element type
        if (child->getType() == Element::FrameType) {
            Frame* frame = qobject_cast<Frame*>(child);
            if (frame) {
                frame->move(newPos);
                
                // Update the frame proxy position
                QGraphicsProxyWidget *frameProxy = frame->property("proxy").value<QGraphicsProxyWidget*>();
                if (frameProxy) {
                    frameProxy->setPos(newPos);
                }
                
                // Find and update associated ClientRect
                QList<QGraphicsItem*> items = scene->items();
                for (QGraphicsItem *item : items) {
                    QGraphicsProxyWidget *proxy = qgraphicsitem_cast<QGraphicsProxyWidget*>(item);
                    if (proxy) {
                        ClientRect *clientRect = qobject_cast<ClientRect*>(proxy->widget());
                        if (clientRect && clientRect->getAssociatedElementId() == frame->getId()) {
                            proxy->setPos(newPos);
                            break;
                        }
                    }
                }
            }
        } else if (child->getType() == Element::TextType) {
            Text* text = qobject_cast<Text*>(child);
            if (text) {
                // Text elements move with their parent frame, so we don't need to do anything special
            }
        }
        
        // Recursively move this child's children
        moveChildElements(child, delta);
    }
}

void Canvas::updateChildClipping(Element* parent) {
    if (!parent || parent->getType() != Element::FrameType) return;
    
    Frame* parentFrame = qobject_cast<Frame*>(parent);
    if (!parentFrame) return;
    
    // Get parent frame's bounds
    QRect parentBounds(parentFrame->getCanvasPosition(), parentFrame->getCanvasSize());
    bool shouldClip = (parentFrame->getOverflow() == "hidden");
    
    // Get all children of this parent
    QList<Element*> children = getChildElements(parent->getId());
    
    // Update clipping for each child
    for (Element* child : children) {
        if (!child) continue;
        
        // Find the child's proxy widget in the scene
        QGraphicsProxyWidget* childProxy = nullptr;
        QList<QGraphicsItem*> items = scene->items();
        for (QGraphicsItem *item : items) {
            QGraphicsProxyWidget *proxy = qgraphicsitem_cast<QGraphicsProxyWidget*>(item);
            if (proxy) {
                // Check if this proxy contains our child element
                if (child->getType() == Element::FrameType) {
                    Frame* frame = qobject_cast<Frame*>(proxy->widget());
                    if (frame && frame->getId() == child->getId()) {
                        childProxy = proxy;
                        break;
                    }
                }
            }
        }
        
        if (childProxy && shouldClip) {
            // Calculate the visible region of the child relative to parent
            QRect childBounds(child->getCanvasPosition(), child->getCanvasSize());
            QRect visibleRect = childBounds.intersected(parentBounds);
            
            if (visibleRect.isEmpty()) {
                // Child is completely outside parent bounds - hide it
                childProxy->setVisible(false);
            } else {
                childProxy->setVisible(true);
                
                // Create a clipping path for the proxy widget
                if (visibleRect != childBounds) {
                    // Child extends beyond parent - need to clip
                    QRectF localClipRect = childProxy->mapFromScene(
                        QRectF(visibleRect)).boundingRect();
                    
                    // Apply clipping using QGraphicsItem's setClip
                    childProxy->setFlag(QGraphicsItem::ItemClipsChildrenToShape, true);
                    
                    // For more precise clipping, we can use a clip path
                    QPainterPath clipPath;
                    clipPath.addRect(localClipRect);
                    
                    // Unfortunately QGraphicsProxyWidget doesn't support setClipPath directly
                    // So we'll use widget masking on the embedded widget
                    QWidget* childWidget = childProxy->widget();
                    if (childWidget) {
                        // Convert the visible rect to widget coordinates
                        QRect widgetClipRect(
                            visibleRect.x() - childBounds.x(),
                            visibleRect.y() - childBounds.y(),
                            visibleRect.width(),
                            visibleRect.height()
                        );
                        QRegion mask(widgetClipRect);
                        childWidget->setMask(mask);
                    }
                } else {
                    // Child is fully within parent bounds - clear any clipping
                    QWidget* childWidget = childProxy->widget();
                    if (childWidget) {
                        childWidget->clearMask();
                    }
                }
            }
        } else if (childProxy) {
            // No clipping needed - ensure child is visible and unclipped
            childProxy->setVisible(true);
            QWidget* childWidget = childProxy->widget();
            if (childWidget) {
                childWidget->clearMask();
            }
        }
        
        // Recursively update clipping for this child's children
        updateChildClipping(child);
    }
}


QRectF Canvas::alignRectToPixels(const QRectF& rect) {
    // For pixel-perfect alignment with floating point precision
    // Convert top-left to view coordinates
    QPointF sceneTopLeft = rect.topLeft();
    QPoint viewTopLeft = mapFromScene(sceneTopLeft);
    
    // Convert back to scene coordinates - this gives us the pixel-aligned position
    QPointF alignedTopLeft = mapToScene(viewTopLeft);
    
    // Also align the bottom-right corner
    QPointF sceneBottomRight = rect.bottomRight();
    QPoint viewBottomRight = mapFromScene(sceneBottomRight);
    QPointF alignedBottomRight = mapToScene(viewBottomRight);
    
    // Create the aligned rectangle from the aligned corners
    return QRectF(alignedTopLeft, alignedBottomRight);
}

QPoint Canvas::alignPointToPixels(const QPoint& point) {
    // Convert to view coordinates
    QPointF scenePoint = point;
    QPoint viewPoint = mapFromScene(scenePoint);
    
    // Convert back to scene coordinates
    QPointF alignedPoint = mapToScene(viewPoint);
    return alignedPoint.toPoint();
}

void Canvas::drawForeground(QPainter* painter, const QRectF& /*rect*/) {
    painter->save();
    painter->setRenderHint(QPainter::Antialiasing, false);
    
    // Reset any transformations - we're drawing in viewport coordinates
    painter->resetTransform();
    
    // Draw hover outline if there's a hovered frame
    if (hoveredFrame) {
        // Check if hovered frame is selected
        bool isSelected = false;
        for (const QString &id : selectedElements) {
            if (QString::number(hoveredFrame->getId()) == id) {
                isSelected = true;
                break;
            }
        }
        
        if (!isSelected) {
            drawFrameOutline(painter, hoveredFrame, Qt::blue);
        }
    }
    
    // Draw controls if visible
    if (controlsVisible && !controlsSceneRect.isEmpty()) {
        drawControlsForRect(painter, controlsSceneRect);
    }
    
    painter->restore();
}

void Canvas::drawFrameOutline(QPainter* painter, Frame* frame, const QColor& color) {
    // Find the frame's proxy widget
    QGraphicsProxyWidget* frameProxy = nullptr;
    QList<QGraphicsItem*> items = scene->items();
    for (QGraphicsItem* item : items) {
        if (QGraphicsProxyWidget* proxy = qgraphicsitem_cast<QGraphicsProxyWidget*>(item)) {
            if (proxy->widget() == frame) {
                frameProxy = proxy;
                break;
            }
        }
    }
    
    if (!frameProxy) return;
    
    // Get the scene rect
    QRectF sceneRect(frameProxy->pos(), frame->size());
    
    // Map corners to viewport
    QPointF tl = mapFromScene(sceneRect.topLeft());
    QPointF tr = mapFromScene(sceneRect.topRight());
    QPointF bl = mapFromScene(sceneRect.bottomLeft());
    QPointF br = mapFromScene(sceneRect.bottomRight());
    
    // Get bounding rect
    qreal minX = qMin(qMin(tl.x(), tr.x()), qMin(bl.x(), br.x()));
    qreal minY = qMin(qMin(tl.y(), tr.y()), qMin(bl.y(), br.y()));
    qreal maxX = qMax(qMax(tl.x(), tr.x()), qMax(bl.x(), br.x()));
    qreal maxY = qMax(qMax(tl.y(), tr.y()), qMax(bl.y(), br.y()));
    
    QRectF viewRect(minX, minY, maxX - minX, maxY - minY);
    
    // Draw outline
    QPen pen(color, 1);
    pen.setCosmetic(true);
    painter->setPen(pen);
    painter->setBrush(Qt::NoBrush);
    painter->drawRect(viewRect.adjusted(0.5, 0.5, -0.5, -0.5));
}

void Canvas::drawControlsForRect(QPainter* painter, const QRectF& sceneRect) {
    // Map the scene rect to viewport coordinates
    QPointF tl = mapFromScene(sceneRect.topLeft());
    QPointF tr = mapFromScene(sceneRect.topRight());
    QPointF bl = mapFromScene(sceneRect.bottomLeft());
    QPointF br = mapFromScene(sceneRect.bottomRight());
    
    qreal minX = qMin(qMin(tl.x(), tr.x()), qMin(bl.x(), br.x()));
    qreal minY = qMin(qMin(tl.y(), tr.y()), qMin(bl.y(), br.y()));
    qreal maxX = qMax(qMax(tl.x(), tr.x()), qMax(bl.x(), br.x()));
    qreal maxY = qMax(qMax(tl.y(), tr.y()), qMax(bl.y(), br.y()));
    
    QRectF viewRect(minX, minY, maxX - minX, maxY - minY);
    
    // Visual sizes for controls (matching Controls.cpp)
    const int visualBarWidth = 10;
    const int visualBarHeight = 10;
    const int visualRotationJointSize = 20;
    const int visualResizeJointSize = 10;
    const int margin = qMax(visualRotationJointSize, visualBarWidth * 2);
    
    // Expand rect by margin
    QRectF expandedRect = viewRect.adjusted(-margin, -margin, margin, margin);
    
    // Draw the yellow inner rect with 5% opacity (matching Controls.cpp)
    painter->fillRect(viewRect, QColor(255, 255, 0, 13));  // 5% of 255 = 12.75 ≈ 13
    
    // Draw red edge bars with 50% opacity
    painter->fillRect(expandedRect.left() + margin - visualBarWidth/2, expandedRect.top() + margin, 
                     visualBarWidth, viewRect.height(), QColor(255, 0, 0, 128));  // Left
    painter->fillRect(expandedRect.right() - margin - visualBarWidth/2, expandedRect.top() + margin,
                     visualBarWidth, viewRect.height(), QColor(255, 0, 0, 128));  // Right
    painter->fillRect(expandedRect.left() + margin, expandedRect.top() + margin - visualBarHeight/2,
                     viewRect.width(), visualBarHeight, QColor(255, 0, 0, 128));  // Top
    painter->fillRect(expandedRect.left() + margin, expandedRect.bottom() - margin - visualBarHeight/2,
                     viewRect.width(), visualBarHeight, QColor(255, 0, 0, 128));  // Bottom
    
    // Draw black center lines with 50% opacity
    const int lineWidth = 1;
    painter->fillRect(expandedRect.left() + margin - lineWidth/2, expandedRect.top() + margin,
                     lineWidth, viewRect.height(), QColor(0, 0, 0, 128));  // Left line
    painter->fillRect(expandedRect.right() - margin - lineWidth/2, expandedRect.top() + margin,
                     lineWidth, viewRect.height(), QColor(0, 0, 0, 128));  // Right line
    painter->fillRect(expandedRect.left() + margin, expandedRect.top() + margin - lineWidth/2,
                     viewRect.width(), lineWidth, QColor(0, 0, 0, 128));  // Top line
    painter->fillRect(expandedRect.left() + margin, expandedRect.bottom() - margin - lineWidth/2,
                     viewRect.width(), lineWidth, QColor(0, 0, 0, 128));  // Bottom line
    
    // Draw rotation joints (blue circles) with 50% opacity
    painter->setPen(Qt::NoPen);
    painter->setBrush(QColor(0, 0, 255, 128));
    
    const int overlapSize = 10;  // From Controls.cpp
    
    // Top-left joint
    painter->drawEllipse(QPointF(expandedRect.left() + margin - visualBarWidth/2 - (visualRotationJointSize - overlapSize)/2,
                                expandedRect.top() + margin - visualBarHeight/2 - (visualRotationJointSize - overlapSize)/2), 
                        visualRotationJointSize/2, visualRotationJointSize/2);
    // Top-right joint
    painter->drawEllipse(QPointF(expandedRect.right() - margin + visualBarWidth/2 - overlapSize/2,
                                expandedRect.top() + margin - visualBarHeight/2 - (visualRotationJointSize - overlapSize)/2), 
                        visualRotationJointSize/2, visualRotationJointSize/2);
    // Bottom-left joint
    painter->drawEllipse(QPointF(expandedRect.left() + margin - visualBarWidth/2 - (visualRotationJointSize - overlapSize)/2,
                                expandedRect.bottom() - margin + visualBarHeight/2 - overlapSize/2), 
                        visualRotationJointSize/2, visualRotationJointSize/2);
    // Bottom-right joint
    painter->drawEllipse(QPointF(expandedRect.right() - margin + visualBarWidth/2 - overlapSize/2,
                                expandedRect.bottom() - margin + visualBarHeight/2 - overlapSize/2), 
                        visualRotationJointSize/2, visualRotationJointSize/2);
    
    // Draw resize joints (yellow squares) with 50% opacity
    painter->setBrush(QColor(255, 255, 0, 128));
    
    // Position at intersection of bars
    painter->drawRect(expandedRect.left() + margin - visualBarWidth/2 - visualResizeJointSize/2, 
                     expandedRect.top() + margin - visualBarHeight/2 - visualResizeJointSize/2,
                     visualResizeJointSize, visualResizeJointSize);
    painter->drawRect(expandedRect.right() - margin + visualBarWidth/2 - visualResizeJointSize/2, 
                     expandedRect.top() + margin - visualBarHeight/2 - visualResizeJointSize/2,
                     visualResizeJointSize, visualResizeJointSize);
    painter->drawRect(expandedRect.left() + margin - visualBarWidth/2 - visualResizeJointSize/2, 
                     expandedRect.bottom() - margin + visualBarHeight/2 - visualResizeJointSize/2,
                     visualResizeJointSize, visualResizeJointSize);
    painter->drawRect(expandedRect.right() - margin + visualBarWidth/2 - visualResizeJointSize/2, 
                     expandedRect.bottom() - margin + visualBarHeight/2 - visualResizeJointSize/2,
                     visualResizeJointSize, visualResizeJointSize);
}

Controls::DragMode Canvas::getControlElementAt(const QPoint& viewportPos) {
    if (!controlsVisible || controlsSceneRect.isEmpty()) {
        return Controls::None;
    }
    
    // Map the controls scene rect to viewport
    QPointF tl = mapFromScene(controlsSceneRect.topLeft());
    QPointF tr = mapFromScene(controlsSceneRect.topRight());
    QPointF bl = mapFromScene(controlsSceneRect.bottomLeft());
    QPointF br = mapFromScene(controlsSceneRect.bottomRight());
    
    qreal minX = qMin(qMin(tl.x(), tr.x()), qMin(bl.x(), br.x()));
    qreal minY = qMin(qMin(tl.y(), tr.y()), qMin(bl.y(), br.y()));
    qreal maxX = qMax(qMax(tl.x(), tr.x()), qMax(bl.x(), br.x()));
    qreal maxY = qMax(qMax(tl.y(), tr.y()), qMax(bl.y(), br.y()));
    
    QRectF viewRect(minX, minY, maxX - minX, maxY - minY);
    
    // Visual sizes
    const int visualBarWidth = 10;
    const int visualBarHeight = 10;
    const int visualRotationJointSize = 20;
    const int visualResizeJointSize = 10;
    const int margin = qMax(visualRotationJointSize, visualBarWidth * 2);
    const int hitTolerance = 5;  // Extra pixels for easier clicking
    
    QRectF expandedRect = viewRect.adjusted(-margin, -margin, margin, margin);
    
    // Check resize joints first (they're on top)
    QRectF topLeftResize(expandedRect.left() + margin - visualBarWidth/2 - visualResizeJointSize/2 - hitTolerance,
                        expandedRect.top() + margin - visualBarHeight/2 - visualResizeJointSize/2 - hitTolerance,
                        visualResizeJointSize + 2*hitTolerance, visualResizeJointSize + 2*hitTolerance);
    if (topLeftResize.contains(viewportPos)) return Controls::TopLeftResizeJoint;
    
    QRectF topRightResize(expandedRect.right() - margin + visualBarWidth/2 - visualResizeJointSize/2 - hitTolerance,
                         expandedRect.top() + margin - visualBarHeight/2 - visualResizeJointSize/2 - hitTolerance,
                         visualResizeJointSize + 2*hitTolerance, visualResizeJointSize + 2*hitTolerance);
    if (topRightResize.contains(viewportPos)) return Controls::TopRightResizeJoint;
    
    QRectF bottomLeftResize(expandedRect.left() + margin - visualBarWidth/2 - visualResizeJointSize/2 - hitTolerance,
                           expandedRect.bottom() - margin + visualBarHeight/2 - visualResizeJointSize/2 - hitTolerance,
                           visualResizeJointSize + 2*hitTolerance, visualResizeJointSize + 2*hitTolerance);
    if (bottomLeftResize.contains(viewportPos)) return Controls::BottomLeftResizeJoint;
    
    QRectF bottomRightResize(expandedRect.right() - margin + visualBarWidth/2 - visualResizeJointSize/2 - hitTolerance,
                            expandedRect.bottom() - margin + visualBarHeight/2 - visualResizeJointSize/2 - hitTolerance,
                            visualResizeJointSize + 2*hitTolerance, visualResizeJointSize + 2*hitTolerance);
    if (bottomRightResize.contains(viewportPos)) return Controls::BottomRightResizeJoint;
    
    // Check rotation joints (under resize joints)
    // For now, rotation joints don't do anything, so we skip them
    
    // Check edge bars
    QRectF leftBar(expandedRect.left() + margin - visualBarWidth/2 - hitTolerance, 
                   expandedRect.top() + margin,
                   visualBarWidth + 2*hitTolerance, viewRect.height());
    if (leftBar.contains(viewportPos)) return Controls::LeftBar;
    
    QRectF rightBar(expandedRect.right() - margin - visualBarWidth/2 - hitTolerance,
                    expandedRect.top() + margin,
                    visualBarWidth + 2*hitTolerance, viewRect.height());
    if (rightBar.contains(viewportPos)) return Controls::RightBar;
    
    QRectF topBar(expandedRect.left() + margin, 
                  expandedRect.top() + margin - visualBarHeight/2 - hitTolerance,
                  viewRect.width(), visualBarHeight + 2*hitTolerance);
    if (topBar.contains(viewportPos)) return Controls::TopBar;
    
    QRectF bottomBar(expandedRect.left() + margin,
                     expandedRect.bottom() - margin - visualBarHeight/2 - hitTolerance,
                     viewRect.width(), visualBarHeight + 2*hitTolerance);
    if (bottomBar.contains(viewportPos)) return Controls::BottomBar;
    
    // Check inner rect
    if (viewRect.contains(viewportPos)) return Controls::InnerRect;
    
    return Controls::None;
}