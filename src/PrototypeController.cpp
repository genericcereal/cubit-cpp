#include "PrototypeController.h"
#include "ElementModel.h"
#include "SelectionManager.h"
#include "ConsoleMessageRepository.h"
#include "CanvasElement.h"
#include "DesignElement.h"
#include "CanvasController.h"
#include "HitTestService.h"
#include "platforms/web/WebTextInput.h"
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
        CanvasElement* canvasElement = qobject_cast<CanvasElement*>(element);
        if (canvasElement) {
                canvasElement->setX(rect.x());
                canvasElement->setY(rect.y());
                canvasElement->setWidth(rect.width());
                canvasElement->setHeight(rect.height());
        }
    }
    
    // Restore all constraint values
    for (auto it = m_prototypingStartSnapshot->elementConstraints.constBegin();
         it != m_prototypingStartSnapshot->elementConstraints.constEnd(); ++it) {
        const QString& elementId = it.key();
        const ConstraintValues& constraints = it.value();
        
        Element* element = m_elementModel.getElementById(elementId);
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
    
    // Restore all WebTextInput values
    for (auto it = m_prototypingStartSnapshot->webTextInputValues.constBegin();
         it != m_prototypingStartSnapshot->webTextInputValues.constEnd(); ++it) {
        const QString& elementId = it.key();
        const QString& originalValue = it.value();
        
        Element* element = m_elementModel.getElementById(elementId);
        if (element && element->getType() == Element::WebTextInputType) {
            WebTextInput* webInput = qobject_cast<WebTextInput*>(element);
            if (webInput) {
                webInput->setValue(originalValue);
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
            
            // If it's a WebTextInput, also capture its value
            if (element->getType() == Element::WebTextInputType) {
                WebTextInput* webInput = qobject_cast<WebTextInput*>(element);
                if (webInput) {
                    m_prototypingStartSnapshot->webTextInputValues[element->getId()] = webInput->value();
                }
            }
        }
    }
    
    // Set isPrototyping to true
    m_isInitializingPrototype = true;
    setIsPrototyping(true);
    
    
    // Ensure viewable area is set based on the current prototype mode
    setViewableArea(calculateViewportForMode(m_prototypeMode));
    
    // This will update the layout based on the viewable area
    setDeviceFrames();
    
    // Mark initialization as complete without triggering canvas movement
    m_isInitializingPrototype = false;
    
    // Set the active outer frame based on current selection
    // This won't trigger canvas movement since isInitializingPrototype was just set to false
    onSelectionChanged();
}

void PrototypeController::stopPrototyping() {
    // Clear the snapshot
    m_prototypingStartSnapshot.reset();
    
    // Clear the active outer frame
    setActiveOuterFrame("");
    
    // Deactivate any active element
    if (!m_activeElement.isEmpty()) {
        Element* activeEl = m_elementModel.getElementById(m_activeElement);
        if (activeEl && activeEl->getType() == Element::WebTextInputType) {
            WebTextInput* webInput = qobject_cast<WebTextInput*>(activeEl);
            if (webInput) {
                webInput->setIsEditing(false);
            }
        }
        m_activeElement.clear();
        emit activeElementChanged();
    }
    
    // Reset scroll simulation state
    setIsSimulatingScroll(false);
    setScrollOffsetX(0.0);
    setScrollOffsetY(0.0);
    setPreviousActiveFrameId("");
    
    // Clear any animated bounds
    m_animatedBoundingX = 0;
    m_animatedBoundingY = 0;
    m_animatedBoundingWidth = 0;
    m_animatedBoundingHeight = 0;
    emit animatedBoundsChanged();
    
    // Set isPrototyping to false
    setIsPrototyping(false);
    
    // Rebuild spatial index since elements may have moved during prototyping
    if (m_canvasController) {
        HitTestService* hitTestService = m_canvasController->hitTestService();
        if (hitTestService) {
            hitTestService->rebuildSpatialIndex();
        }
    }
}

QRectF PrototypeController::calculateViewportForMode(const QString& mode) const {
    QString lowerMode = mode.toLower();
    if (lowerMode == "ios") {
        return QRectF(0, 0, IOS_WIDTH, IOS_HEIGHT);
    } else if (lowerMode == "android") {
        return QRectF(0, 0, ANDROID_WIDTH, ANDROID_HEIGHT);
    } else {
        // Default to web
        return QRectF(0, 0, WEB_WIDTH, WEB_HEIGHT);
    }
}

bool PrototypeController::isElementInViewableArea(qreal x, qreal y, qreal width, qreal height) const {
    // Check if any part of the element rect intersects with the viewable area
    QRectF elementRect(x, y, width, height);
    return m_viewableArea.intersects(elementRect);
}

void PrototypeController::updateAnimatedBounds(const QList<QRectF>& elementBounds) {
    if (elementBounds.isEmpty()) {
        m_animatedBoundingX = 0;
        m_animatedBoundingY = 0;
        m_animatedBoundingWidth = 0;
        m_animatedBoundingHeight = 0;
    } else {
        // Calculate the bounding box of all element bounds
        qreal minX = std::numeric_limits<qreal>::max();
        qreal minY = std::numeric_limits<qreal>::max();
        qreal maxX = std::numeric_limits<qreal>::lowest();
        qreal maxY = std::numeric_limits<qreal>::lowest();
        
        for (const QRectF& rect : elementBounds) {
            minX = std::min(minX, rect.left());
            minY = std::min(minY, rect.top());
            maxX = std::max(maxX, rect.right());
            maxY = std::max(maxY, rect.bottom());
        }
        
        m_animatedBoundingX = minX;
        m_animatedBoundingY = minY;
        m_animatedBoundingWidth = maxX - minX;
        m_animatedBoundingHeight = maxY - minY;
    }
    
    emit animatedBoundsChanged();
}

void PrototypeController::setActiveOuterFrame(const QString& frameId) {
    if (m_activeOuterFrame != frameId) {
        // If we're simulating scroll and changing frames, reset the previous frame
        if (m_isSimulatingScroll && !m_activeOuterFrame.isEmpty()) {
            resetFrameScrollPosition(m_activeOuterFrame);
            
            // Reset scroll simulation state
            setIsSimulatingScroll(false);
            setScrollOffsetX(0.0);
            setScrollOffsetY(0.0);
        }
        
        // Store the previous frame ID
        setPreviousActiveFrameId(m_activeOuterFrame);
        
        m_activeOuterFrame = frameId;
        
        // Update selected frame position for centering
        if (!frameId.isEmpty()) {
            Element* element = m_elementModel.getElementById(frameId);
            CanvasElement* canvasElement = qobject_cast<CanvasElement*>(element);
            if (canvasElement) {
                    m_selectedFrameX = canvasElement->x();
                    m_selectedFrameHeight = canvasElement->height();
                    emit selectedFramePositionChanged();
                    
                    // Canvas movement when activeOuterFrame changes is disabled for now
                    // if (m_isPrototyping && !m_isInitializingPrototype) {
                    //     // Calculate the point to center in the viewport
                    //     // X: Center of the frame horizontally
                    //     // Y: Top edge of the frame (we want to align top edges)
                    //     QPointF alignmentPoint(
                    //         canvasElement->x() + canvasElement->width() / 2.0,
                    //         canvasElement->y()  // Just the top edge
                    //     );
                    //     
                    //     // Request the canvas to move to this point with animation
                    //     emit requestCanvasMove(alignmentPoint, true);
                    // }
            }
        } else {
            m_selectedFrameX = -1;
            m_selectedFrameHeight = 0;
            emit selectedFramePositionChanged();
        }
        
        emit activeOuterFrameChanged();
    }
}

void PrototypeController::setDeviceFrames(bool isModeChange, qreal oldViewableHeight) {
    if (!m_isPrototyping) {
        return;
    }
    
    // Get all Frame elements
    const auto allElements = m_elementModel.getAllElements();
    for (Element* element : allElements) {
        CanvasElement* canvasElement = qobject_cast<CanvasElement*>(element);
        if (!canvasElement || !canvasElement->isDesignElement()) continue;
        
        DesignElement* designElement = qobject_cast<DesignElement*>(canvasElement);
        if (!designElement || element->getType() != Element::FrameType) continue;
        
        // Skip frames that have a parent (not top-level)
        if (!element->getParentElementId().isEmpty()) continue;
        
        // Update frame width to match viewable area
        designElement->setWidth(m_viewableArea.width());
        
        // Only update frame height if it's not already greater than viewable area height
        if (designElement->height() <= m_viewableArea.height()) {
            designElement->setHeight(m_viewableArea.height());
        }
        
        // When changing modes, adjust Y position to maintain visual alignment
        if (isModeChange && oldViewableHeight > 0) {
            // Only adjust Y position if we actually changed the frame height
            if (designElement->height() == m_viewableArea.height()) {
                // Calculate the height difference
                qreal heightDiff = m_viewableArea.height() - oldViewableHeight;
                
                // Adjust Y position to compensate for height change
                // When height increases, we need to move the frame up by half the difference
                // to maintain visual center alignment
                qreal currentY = designElement->y();
                designElement->setY(currentY - heightDiff / 2.0);
            }
        }
        
        // Recursively update all child layouts to adapt to new parent size
        updateChildLayouts(canvasElement);
    }
}

void PrototypeController::onSelectionChanged() {
    if (!m_isPrototyping || m_isInitializingPrototype) {
        return;
    }
    
    // Get selected elements
    const auto selectedElements = m_selectionManager.selectedElements();
    
    // Find the outermost frame among selected elements
    QString outermostFrameId;
    for (Element* element : selectedElements) {
        if (!element) continue;
        
        // Start from the element and traverse up to find topmost frame
        Element* current = element;
        Element* topmostFrame = nullptr;
        
        while (current) {
            // Check if current is a Frame
            CanvasElement* canvasElement = qobject_cast<CanvasElement*>(current);
            if (canvasElement && canvasElement->isDesignElement()) {
                DesignElement* designElement = qobject_cast<DesignElement*>(canvasElement);
                if (designElement && current->getType() == Element::FrameType) {
                    topmostFrame = current;
                }
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
        CanvasElement* canvasChild = qobject_cast<CanvasElement*>(child);
        if (!canvasChild) continue;
        
        // Check if it's a design element
        if (canvasChild->isDesignElement()) {
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

void PrototypeController::updateHoveredElement(const QPointF& canvasPoint) {
    if (!m_canvasController || !m_canvasController->hitTestService()) {
        return;
    }
    
    // Use hitTestForHover which excludes selected elements
    Element* element = m_canvasController->hitTestService()->hitTestForHover(canvasPoint);
    
    QString newHoveredElement;
    if (element) {
        newHoveredElement = element->getId();
    }
    
    // Check if the hovered element has changed
    if (m_hoveredElement != newHoveredElement) {
        m_hoveredElement = newHoveredElement;
        
        
        emit hoveredElementChanged();
    }
}

void PrototypeController::handlePrototypeClick(const QPointF& canvasPoint) {
    if (!m_canvasController || !m_canvasController->hitTestService()) {
        return;
    }
    
    // Hit test at the click point
    Element* element = m_canvasController->hitTestService()->hitTest(canvasPoint);
    
    if (element && element->getType() == Element::WebTextInputType) {
        // Activate the WebTextInput
        WebTextInput* webInput = qobject_cast<WebTextInput*>(element);
        if (webInput) {
            webInput->setIsEditing(true);
            m_activeElement = element->getId();
            
            emit activeElementChanged();
        }
    }
}

void PrototypeController::clearActiveInput() {
    if (!m_activeElement.isEmpty()) {
        Element* currentActive = m_elementModel.getElementById(m_activeElement);
        if (currentActive && currentActive->getType() == Element::WebTextInputType) {
            WebTextInput* webInput = qobject_cast<WebTextInput*>(currentActive);
            if (webInput) {
                webInput->setIsEditing(false);
            }
        }
        m_activeElement.clear();
        emit activeElementChanged();
    }
}

void PrototypeController::setIsSimulatingScroll(bool value) {
    if (m_isSimulatingScroll != value) {
        m_isSimulatingScroll = value;
        emit isSimulatingScrollChanged();
    }
}

void PrototypeController::setScrollOffsetX(qreal offset) {
    if (m_scrollOffsetX != offset) {
        m_scrollOffsetX = offset;
        emit scrollOffsetXChanged();
    }
}

void PrototypeController::setScrollOffsetY(qreal offset) {
    if (m_scrollOffsetY != offset) {
        m_scrollOffsetY = offset;
        emit scrollOffsetYChanged();
    }
}

void PrototypeController::setPreviousActiveFrameId(const QString& frameId) {
    if (m_previousActiveFrameId != frameId) {
        m_previousActiveFrameId = frameId;
        emit previousActiveFrameIdChanged();
    }
}

void PrototypeController::resetFrameScrollPosition(const QString& frameId) {
    if (frameId.isEmpty() || !m_prototypingStartSnapshot) {
        return;
    }
    
    Element* element = m_elementModel.getElementById(frameId);
    CanvasElement* canvasElement = qobject_cast<CanvasElement*>(element);
    if (canvasElement) {
            // Get the original position from snapshot
            QRectF snapshotRect = m_prototypingStartSnapshot->elementPositions.value(frameId, QRectF());
            if (snapshotRect.width() > 0 && snapshotRect.height() > 0) {
                canvasElement->setY(snapshotRect.y());
            }
    }
}