#include "PrototypeController.h"
#include "ElementModel.h"
#include "SelectionManager.h"
#include "ConsoleMessageRepository.h"
#include "CanvasElement.h"
#include <algorithm>
#include <limits>

PrototypeController::PrototypeController(ElementModel& model,
                                       SelectionManager& sel,
                                       QObject *parent)
    : QObject(parent)
    , m_elementModel(model)
    , m_selectionManager(sel)
{
    // Initialize default viewable area for Web mode
    m_viewableArea = calculateViewportForMode("Web");
    
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
        
        m_prototypeMode = mode;
        
        // Update viewable area based on new mode
        m_viewableArea = calculateViewportForMode(mode);
        
        // If we're in prototyping mode, update the frame positions with the new viewable area
        if (m_isPrototyping) {
            // When changing modes, we need to maintain top edge alignment
            // Pass true to indicate this is a mode change
            setDeviceFrames(true, oldViewableHeight);
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
}

void PrototypeController::startPrototyping(const QPointF& canvasCenter, qreal currentZoom) {
    // Create a new snapshot
    m_prototypingStartSnapshot = std::make_unique<PrototypeSnapshot>();
    m_prototypingStartSnapshot->canvasPosition = canvasCenter;
    m_prototypingStartSnapshot->canvasZoom = currentZoom;
    
    // Capture all element positions
    const auto elements = m_elementModel.getAllElements();
    for (Element* element : elements) {
        if (element && element->isVisual()) {
            CanvasElement* canvasElement = qobject_cast<CanvasElement*>(element);
            if (canvasElement) {
                QRectF rect(canvasElement->x(), canvasElement->y(), 
                           canvasElement->width(), canvasElement->height());
                m_prototypingStartSnapshot->elementPositions[element->getId()] = rect;
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
    if (mode == "Mobile") {
        return QRectF(0, 0, MOBILE_WIDTH, MOBILE_HEIGHT);
    } else {
        // Default to Web
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
                        return frame->y() != -350;  // Not yet positioned for prototyping
                    });
    
    for (CanvasElement* frame : topLevelFrames) {
        if (isInitialPositioning) {
            // Initial positioning - arrange frames horizontally
            qreal originalHeight = frame->height();
            qreal finalHeight = originalHeight;
            
            // Set position with top edge 350px above canvas axis
            frame->setX(currentX);
            frame->setY(-350);  // Top edge at y = -350
            frame->setWidth(m_viewableArea.width());
            
            // Only adjust height if it's less than viewable area height
            if (originalHeight < m_viewableArea.height()) {
                frame->setHeight(m_viewableArea.height());
                finalHeight = m_viewableArea.height();
            }
            
            // Track selected frame position and height
            if (frame->getId() == selectedFrameId) {
                m_selectedFrameX = currentX + m_viewableArea.width() / 2.0;  // Center of the frame
                m_selectedFrameHeight = finalHeight;
            }
            
            // Move to next position
            currentX += m_viewableArea.width() + spacing;
        } else if (isModeChange) {
            // Mode change - maintain top edge alignment
            qreal originalWidth = frame->width();
            qreal originalHeight = frame->height();
            qreal topEdgeY = frame->y();  // Current top edge position
            qreal centerX = frame->x() + originalWidth / 2.0;
            
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
            
            // Calculate new X position to maintain horizontal center
            qreal newX = centerX - newWidth / 2.0;
            
            // Maintain the same top edge Y position
            frame->setX(newX);
            frame->setY(topEdgeY);  // Keep the same top edge
            frame->setWidth(newWidth);
            frame->setHeight(newHeight);
            
            // Track selected frame position and height
            if (frame->getId() == selectedFrameId) {
                m_selectedFrameX = centerX;  // Use the actual center
                m_selectedFrameHeight = newHeight;
            }
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
                    
                    // Request the canvas to move to this point with animation
                    emit requestCanvasMove(alignmentPoint, true);
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