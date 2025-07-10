#include "PrototypeController.h"
#include "ElementModel.h"
#include "SelectionManager.h"
#include "ConsoleMessageRepository.h"
#include "CanvasElement.h"
#include "DesignElement.h"
#include <algorithm>
#include <limits>
#include <QTimer>

PrototypeController::PrototypeController(ElementModel& model,
                                       SelectionManager& sel,
                                       QObject *parent)
    : QObject(parent)
    , m_elementModel(model)
    , m_selectionManager(sel)
{
    // Initialize default viewable area for web mode
    m_viewableArea = calculateViewportForMode("web");
    
    // Monitor selection changes to update activeOuterFrame
    connect(&m_selectionManager, &SelectionManager::selectionChanged,
            this, &PrototypeController::onSelectionChanged);
}

void PrototypeController::setIsPrototyping(bool value) {
    if (m_isPrototyping != value) {
        m_isPrototyping = value;
        emit isPrototypingChanged();
        
        if (value) {
            emit prototypingStarted();
        } else {
            emit prototypingStopped();
        }
    }
}

void PrototypeController::setViewableArea(const QRectF& area) {
    if (m_viewableArea != area) {
        m_viewableArea = area;
        emit viewableAreaChanged();
    }
}

void PrototypeController::setPrototypeMode(const QString& mode) {
    if (m_prototypeMode != mode) {
        // Store old viewable area height to calculate offset
        qreal oldViewableHeight = m_viewableArea.height();
        
        // Calculate the new viewable area
        QRectF newViewableArea = calculateViewportForMode(mode);
        
        m_prototypeMode = mode;
        
        // Update viewable area based on new mode
        m_viewableArea = newViewableArea;
        
        // If we're in prototyping mode, update the frame positions with the new viewable area
        if (m_isPrototyping) {
            // When changing modes, we need to maintain top edge alignment
            // Pass true to indicate this is a mode change
            setDeviceFrames(true, oldViewableHeight);
            
            // Compensate for the viewable area height change by adjusting canvas position
            // When viewable area grows taller, we need to move canvas down to maintain top edge
            // When viewable area shrinks, we need to move canvas up
            if (!m_activeOuterFrame.isEmpty()) {
                Element* element = m_elementModel.getElementById(m_activeOuterFrame);
                if (element && element->isVisual()) {
                    CanvasElement* canvasElement = qobject_cast<CanvasElement*>(element);
                    if (canvasElement) {
                        // Calculate the point to center in the viewport
                        // X: Center of the frame horizontally
                        // Y: Top edge of the frame (we want to align top edges)
                        QPointF alignmentPoint(
                            canvasElement->x() + canvasElement->width() / 2.0,
                            canvasElement->y()  // Just the top edge, no offset
                        );
                        
                        // Request the canvas to move to this point without animation
                        emit requestCanvasMove(alignmentPoint, false);
                    }
                }
            }
        }
        
        emit prototypeModeChanged();
        emit viewableAreaChanged();
    }
}

QPointF PrototypeController::getSnapshotCanvasPosition() const {
    if (m_prototypingStartSnapshot) {
        return m_prototypingStartSnapshot->canvasPosition;
    }
    return QPointF(0, 0);
}

qreal PrototypeController::getSnapshotCanvasZoom() const {
    if (m_prototypingStartSnapshot) {
        return m_prototypingStartSnapshot->canvasZoom;
    }
    return 1.0;
}

QRectF PrototypeController::getSnapshotElementPosition(const QString& elementId) const {
    if (m_prototypingStartSnapshot) {
        return m_prototypingStartSnapshot->elementPositions.value(elementId, QRectF());
    }
    return QRectF();
}

void PrototypeController::restoreElementPositionsFromSnapshot() {
    if (!m_prototypingStartSnapshot) {
        return;
    }
    
    // Restore all element positions without animation
    for (auto it = m_prototypingStartSnapshot->elementPositions.constBegin(); 
         it != m_prototypingStartSnapshot->elementPositions.constEnd(); ++it) {
        const QString& elementId = it.key();
        const QRectF& rect = it.value();
        
        Element* element = m_elementModel.getElementById(elementId);
        if (element && element->isVisual()) {
            CanvasElement* canvasElement = qobject_cast<CanvasElement*>(element);
            if (canvasElement) {
                canvasElement->setX(rect.x());
                canvasElement->setY(rect.y());
                canvasElement->setWidth(rect.width());
                canvasElement->setHeight(rect.height());
            }
        }
    }
    
    // Restore all constraint values
    for (auto it = m_prototypingStartSnapshot->elementConstraints.constBegin();
         it != m_prototypingStartSnapshot->elementConstraints.constEnd(); ++it) {
        const QString& elementId = it.key();
        const ConstraintValues& constraints = it.value();
        
        Element* element = m_elementModel.getElementById(elementId);
        if (element && element->isVisual()) {
            CanvasElement* canvasElement = qobject_cast<CanvasElement*>(element);
            if (canvasElement && canvasElement->isDesignElement()) {
                DesignElement* designElement = qobject_cast<DesignElement*>(canvasElement);
                if (designElement) {
                    // Restore constraint values
                    designElement->setLeft(constraints.left);
                    designElement->setRight(constraints.right);
                    designElement->setTop(constraints.top);
                    designElement->setBottom(constraints.bottom);
                    designElement->setLeftAnchored(constraints.leftAnchored);
                    designElement->setRightAnchored(constraints.rightAnchored);
                    designElement->setTopAnchored(constraints.topAnchored);
                    designElement->setBottomAnchored(constraints.bottomAnchored);
                    
                    // Trigger layout update
                    designElement->updateFromParentGeometry();
                }
            }
        }
    }
}

void PrototypeController::startPrototyping(const QPointF& canvasCenter, qreal currentZoom) {
    // Create a new snapshot
    m_prototypingStartSnapshot = std::make_unique<PrototypeSnapshot>();
    m_prototypingStartSnapshot->canvasPosition = canvasCenter;
    m_prototypingStartSnapshot->canvasZoom = currentZoom;
    
    // Capture all element positions and constraints
    const auto elements = m_elementModel.getAllElements();
    for (Element* element : elements) {
        if (element && element->isVisual()) {
            CanvasElement* canvasElement = qobject_cast<CanvasElement*>(element);
            if (canvasElement) {
                QRectF rect(canvasElement->x(), canvasElement->y(), 
                           canvasElement->width(), canvasElement->height());
                m_prototypingStartSnapshot->elementPositions[element->getId()] = rect;
                
                // If it's a design element, also capture constraint values
                if (canvasElement->isDesignElement()) {
                    DesignElement* designElement = qobject_cast<DesignElement*>(canvasElement);
                    if (designElement) {
                        ConstraintValues constraints;
                        constraints.left = designElement->left();
                        constraints.right = designElement->right();
                        constraints.top = designElement->top();
                        constraints.bottom = designElement->bottom();
                        constraints.leftAnchored = designElement->leftAnchored();
                        constraints.rightAnchored = designElement->rightAnchored();
                        constraints.topAnchored = designElement->topAnchored();
                        constraints.bottomAnchored = designElement->bottomAnchored();
                        m_prototypingStartSnapshot->elementConstraints[element->getId()] = constraints;
                    }
                }
            }
        }
    }
    
    // Initialize animated bounds from current selection bounds before starting
    if (m_selectionManager.hasVisualSelection()) {
        m_animatedBoundingX = m_selectionManager.boundingX();
        m_animatedBoundingY = m_selectionManager.boundingY();
        m_animatedBoundingWidth = m_selectionManager.boundingWidth();
        m_animatedBoundingHeight = m_selectionManager.boundingHeight();
        // Don't emit signal yet, it will be emitted when isPrototyping changes
    }
    
    // Mark that we're initializing to prevent premature centering
    m_isInitializingPrototype = true;
    
    // Start prototyping
    setIsPrototyping(true);
    
    // Set device frames after entering prototype mode
    setDeviceFrames();
    
    // Now that frames are positioned, we can safely check selection
    m_isInitializingPrototype = false;
    
    // Check current selection to set initial activeOuterFrame
    onSelectionChanged();
    
    // After initialization, if we have an active outer frame, request centering
    if (!m_activeOuterFrame.isEmpty()) {
        Element* element = m_elementModel.getElementById(m_activeOuterFrame);
        if (element && element->isVisual()) {
            CanvasElement* canvasElement = qobject_cast<CanvasElement*>(element);
            if (canvasElement) {
                // Calculate the point to center in the viewport
                // X: Center of the frame horizontally
                // Y: Top edge of the frame (we want to align top edges)
                QPointF alignmentPoint(
                    canvasElement->x() + canvasElement->width() / 2.0,
                    canvasElement->y()  // Just the top edge, no offset
                );
                
                ConsoleMessageRepository::instance()->addOutput(
                    QString("PrototypeController::startPrototyping - Emitting initial requestCanvasMove: x=%1, y=%2")
                    .arg(alignmentPoint.x())
                    .arg(alignmentPoint.y())
                );
                
                // Request the canvas to move to this point without animation
                // Use QTimer::singleShot to ensure UI elements are fully positioned
                QTimer::singleShot(0, this, [this, alignmentPoint]() {
                    emit requestCanvasMove(alignmentPoint, false);
                });
            }
        }
    }
    
    // Prototyping started
}

void PrototypeController::stopPrototyping() {
    setIsPrototyping(false);
    
    // Clear the active outer frame
    setActiveOuterFrame("");
    
    // Clear the snapshot
    m_prototypingStartSnapshot.reset();
    
    // Don't reset animated bounds here - let them maintain their values
    // so the exit animation can use them. They'll be re-initialized
    // the next time prototyping starts.
    
    // Prototyping stopped
}

QRectF PrototypeController::calculateViewportForMode(const QString& mode) const {
    if (mode == "ios") {
        return QRectF(0, 0, IOS_WIDTH, IOS_HEIGHT);
    } else if (mode == "android") {
        return QRectF(0, 0, ANDROID_WIDTH, ANDROID_HEIGHT);
    } else {
        // Default to web
        return QRectF(0, 0, WEB_WIDTH, WEB_HEIGHT);
    }
}

bool PrototypeController::isElementInViewableArea(qreal x, qreal y, qreal width, qreal height) const {
    if (!m_isPrototyping) {
        return true; // All elements are viewable when not prototyping
    }
    
    QRectF elementRect(x, y, width, height);
    return m_viewableArea.intersects(elementRect);
}

void PrototypeController::setDeviceFrames(bool isModeChange, qreal oldViewableHeight) {
    // Get all elements and filter for top-level frames and component instances
    const auto elements = m_elementModel.getAllElements();
    QList<CanvasElement*> topLevelFrames;
    
    for (Element* element : elements) {
        if (!element || !element->isVisual()) continue;
        
        // Check if it's a Frame or ComponentInstance
        if (element->getType() != Element::FrameType && 
            element->getType() != Element::ComponentInstanceType) continue;
        
        // Check if it has no parent (top-level)
        if (!element->getParentElementId().isEmpty()) continue;
        
        CanvasElement* canvasElement = qobject_cast<CanvasElement*>(element);
        if (canvasElement) {
            topLevelFrames.append(canvasElement);
        }
    }
    
    // Sort frames by their order in the element model (creation order)
    // This maintains consistent ordering based on when elements were created
    std::sort(topLevelFrames.begin(), topLevelFrames.end(), 
              [this](const CanvasElement* a, const CanvasElement* b) {
                  // Find the index of each element in the model
                  const auto elements = m_elementModel.getAllElements();
                  int indexA = -1, indexB = -1;
                  for (int i = 0; i < elements.size(); ++i) {
                      if (elements[i] == a) indexA = i;
                      if (elements[i] == b) indexB = i;
                  }
                  return indexA < indexB;
              });
    
    // Position frames horizontally with 150px spacing
    qreal currentX = 0;
    const qreal spacing = 150.0;
    
    // Find which frame is selected (if any)
    QString selectedFrameId;
    if (m_selectionManager.hasSelection()) {
        const auto selectedElements = m_selectionManager.selectedElements();
        if (selectedElements.size() == 1) {
            Element* selectedElement = selectedElements.first();
            if (selectedElement && selectedElement->isVisual() &&
                (selectedElement->getType() == Element::FrameType || 
                 selectedElement->getType() == Element::ComponentInstanceType) &&
                selectedElement->getParentElementId().isEmpty()) {
                selectedFrameId = selectedElement->getId();
            }
        }
    }
    
    // Track the position of the selected frame for centering
    m_selectedFrameX = -1;
    m_selectedFrameHeight = 0;
    
    // Check if this is the first time we're setting device frames
    // (frames don't have a consistent Y position yet)
    bool isInitialPositioning = topLevelFrames.isEmpty() ? false : 
        std::all_of(topLevelFrames.begin(), topLevelFrames.end(), 
                    [](const CanvasElement* frame) {
                        return frame->y() != 0;  // Not yet positioned for prototyping
                    });
    
    for (CanvasElement* frame : topLevelFrames) {
        if (isInitialPositioning) {
            // Initial positioning - arrange frames horizontally
            qreal originalWidth = frame->width();
            qreal originalHeight = frame->height();
            qreal finalHeight = originalHeight;
            
            // Set position with top edge at y = 0 (will be aligned via canvas movement)
            frame->setX(currentX);
            frame->setY(0);  // Top edge at y = 0
            frame->setWidth(m_viewableArea.width());
            
            // Only adjust height if it's less than viewable area height
            if (originalHeight < m_viewableArea.height()) {
                frame->setHeight(m_viewableArea.height());
                finalHeight = m_viewableArea.height();
            }
            
            // Update child layouts for the new parent size
            updateChildLayouts(frame);
            
            ConsoleMessageRepository::instance()->addOutput(
                QString("PrototypeController::setDeviceFrames - Positioned frame %1 at y=%2")
                .arg(frame->getId())
                .arg(frame->y())
            );
            
            // Track selected frame position and height
            if (frame->getId() == selectedFrameId) {
                m_selectedFrameX = currentX + m_viewableArea.width() / 2.0;  // Center of the frame
                m_selectedFrameHeight = finalHeight;
            }
            
            // Move to next position
            currentX += m_viewableArea.width() + spacing;
        } else if (isModeChange) {
            // Mode change - maintain top edge alignment and 150px spacing
            qreal originalWidth = frame->width();
            qreal originalHeight = frame->height();
            qreal topEdgeY = frame->y();  // Current top edge position
            
            // Calculate new dimensions
            qreal newWidth = m_viewableArea.width();
            qreal newHeight = originalHeight;
            
            // If the frame was using the old viewable height, update to new viewable height
            if (qFuzzyCompare(originalHeight, oldViewableHeight)) {
                newHeight = m_viewableArea.height();
            } else if (originalHeight < m_viewableArea.height()) {
                // Only adjust height if it's less than viewable area height
                newHeight = m_viewableArea.height();
            }
            
            // Set new X position with proper spacing
            frame->setX(currentX);
            frame->setY(topEdgeY);  // Keep the same top edge
            frame->setWidth(newWidth);
            frame->setHeight(newHeight);
            
            // Update child layouts for the new parent size
            updateChildLayouts(frame);
            
            // Track selected frame position and height
            if (frame->getId() == selectedFrameId) {
                m_selectedFrameX = currentX + newWidth / 2.0;  // Center of the frame
                m_selectedFrameHeight = newHeight;
            }
            
            // Move to next position with 150px spacing
            currentX += newWidth + spacing;
        } else {
            // Subsequent calls (not mode change) - resize from center
            qreal originalWidth = frame->width();
            qreal originalHeight = frame->height();
            qreal centerX = frame->x() + originalWidth / 2.0;
            qreal centerY = frame->y() + originalHeight / 2.0;
            
            // Calculate new dimensions
            qreal newWidth = m_viewableArea.width();
            qreal newHeight = originalHeight;
            qreal finalHeight = originalHeight;
            
            // Only adjust height if it's less than viewable area height
            if (originalHeight < m_viewableArea.height()) {
                newHeight = m_viewableArea.height();
                finalHeight = m_viewableArea.height();
            }
            
            // Calculate new position to maintain center point
            qreal newX = centerX - newWidth / 2.0;
            qreal newY = centerY - newHeight / 2.0;
            
            // Set new position and size
            frame->setX(newX);
            frame->setY(newY);
            frame->setWidth(newWidth);
            frame->setHeight(newHeight);
            
            // Update child layouts for the new parent size
            updateChildLayouts(frame);
            
            // Track selected frame position and height
            if (frame->getId() == selectedFrameId) {
                m_selectedFrameX = centerX;  // Use the actual center
                m_selectedFrameHeight = finalHeight;
            }
        }
    }
    
    // Emit signal if we found the selected frame
    if (m_selectedFrameX >= 0) {
        emit selectedFramePositionChanged();
    }
    
    // Positioned frames for prototype mode
}

void PrototypeController::updateAnimatedBounds(const QList<QRectF>& elementBounds) {
    if (elementBounds.isEmpty()) {
        return;
    }
    
    // Calculate the overall bounding box from the provided element bounds
    qreal minX = std::numeric_limits<qreal>::max();
    qreal minY = std::numeric_limits<qreal>::max();
    qreal maxX = std::numeric_limits<qreal>::lowest();
    qreal maxY = std::numeric_limits<qreal>::lowest();
    
    for (const QRectF& rect : elementBounds) {
        minX = std::min(minX, rect.x());
        minY = std::min(minY, rect.y());
        maxX = std::max(maxX, rect.x() + rect.width());
        maxY = std::max(maxY, rect.y() + rect.height());
    }
    
    // Update animated bounds
    bool changed = false;
    if (m_animatedBoundingX != minX) {
        m_animatedBoundingX = minX;
        changed = true;
    }
    if (m_animatedBoundingY != minY) {
        m_animatedBoundingY = minY;
        changed = true;
    }
    if (m_animatedBoundingWidth != (maxX - minX)) {
        m_animatedBoundingWidth = maxX - minX;
        changed = true;
    }
    if (m_animatedBoundingHeight != (maxY - minY)) {
        m_animatedBoundingHeight = maxY - minY;
        changed = true;
    }
    
    if (changed) {
        emit animatedBoundsChanged();
    }
}

void PrototypeController::setActiveOuterFrame(const QString& frameId) {
    if (m_activeOuterFrame != frameId) {
        m_activeOuterFrame = frameId;
        emit activeOuterFrameChanged();
        
        ConsoleMessageRepository::instance()->addOutput(
            QString("PrototypeController::setActiveOuterFrame - frameId: %1, isPrototyping: %2, isInitializing: %3")
            .arg(frameId.isEmpty() ? "empty" : frameId)
            .arg(m_isPrototyping ? "true" : "false")
            .arg(m_isInitializingPrototype ? "true" : "false")
        );
        
        // If we're in prototyping mode and have a valid frame, center it
        // But skip centering during initialization to allow setDeviceFrames to position correctly
        if (m_isPrototyping && !frameId.isEmpty() && !m_isInitializingPrototype) {
            Element* element = m_elementModel.getElementById(frameId);
            if (element && element->isVisual()) {
                CanvasElement* canvasElement = qobject_cast<CanvasElement*>(element);
                if (canvasElement) {
                    // Calculate the point to center in the viewport
                    // X: Center of the frame horizontally
                    // Y: Top edge of the frame (we want to align top edges)
                    QPointF alignmentPoint(
                        canvasElement->x() + canvasElement->width() / 2.0,
                        canvasElement->y()  // Just the top edge, no offset
                    );
                    
                    ConsoleMessageRepository::instance()->addOutput(
                        QString("PrototypeController - Emitting requestCanvasMove: x=%1, y=%2")
                        .arg(alignmentPoint.x())
                        .arg(alignmentPoint.y())
                    );
                    
                    // Request the canvas to move to this point without animation
                    emit requestCanvasMove(alignmentPoint, false);
                }
            }
        }
    }
}

void PrototypeController::onSelectionChanged() {
    // Only update activeOuterFrame if we're in prototyping mode
    if (!m_isPrototyping) {
        return;
    }
    
    if (!m_selectionManager.hasSelection()) {
        // No selection, clear activeOuterFrame
        setActiveOuterFrame("");
        return;
    }
    
    const auto selectedElements = m_selectionManager.selectedElements();
    
    // Find the outermost frame among selected elements or their ancestors
    QString outermostFrameId;
    
    for (Element* element : selectedElements) {
        if (!element) continue;
        
        // Walk up the parent hierarchy to find the topmost frame
        Element* current = element;
        Element* topmostFrame = nullptr;
        
        while (current) {
            // Check if current element is a top-level frame or component instance
            if ((current->getType() == Element::FrameType || 
                 current->getType() == Element::ComponentInstanceType) &&
                current->getParentElementId().isEmpty()) {
                topmostFrame = current;
                break;
            }
            
            // Move to parent
            QString parentId = current->getParentElementId();
            if (parentId.isEmpty()) {
                break;
            }
            current = m_elementModel.getElementById(parentId);
        }
        
        // If we found a topmost frame, use it
        if (topmostFrame) {
            outermostFrameId = topmostFrame->getId();
            break;  // Use the first topmost frame found
        }
    }
    
    // Update activeOuterFrame if we found one
    if (!outermostFrameId.isEmpty()) {
        setActiveOuterFrame(outermostFrameId);
    }
}

void PrototypeController::updateChildLayouts(CanvasElement* parent) {
    // Get all child elements
    const auto children = m_elementModel.getChildrenRecursive(parent->getId());
    
    for (Element* child : children) {
        if (!child || !child->isVisual()) continue;
        
        // Cast to CanvasElement first to check if it's a design element
        CanvasElement* canvasChild = qobject_cast<CanvasElement*>(child);
        if (canvasChild && canvasChild->isDesignElement()) {
            DesignElement* designElement = qobject_cast<DesignElement*>(canvasChild);
            if (designElement) {
                // For anchored constraints, we need to update the element's position/size
                // based on the new parent size while keeping the constraint values fixed
                
                // The constraint values themselves should NOT be scaled - they represent
                // fixed distances from the parent edges
                
                // Just trigger layout update to reposition the element based on the
                // new parent size with the existing constraint values
                designElement->updateFromParentGeometry();
            }
        }
    }
}