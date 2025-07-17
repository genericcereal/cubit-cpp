#include "FlexLayoutEngine.h"
#include "Element.h"
#include "CanvasElement.h"
#include "ElementModel.h"
#include "Application.h"
#include "platforms/web/WebTextInput.h"
#include "Text.h"
#include <QDebug>
#include <QTimer>
#include <limits>
#include <cmath>

// Helper function to round to nearest 0.5
static qreal roundToHalf(qreal value)
{
    return std::round(value * 2.0) / 2.0;
}

FlexLayoutEngine::FlexLayoutEngine(QObject *parent)
    : QObject(parent)
{
    // Initialize the batch timer
    m_layoutBatchTimer = new QTimer(this);
    m_layoutBatchTimer->setSingleShot(true);
    m_layoutBatchTimer->setInterval(0); // Process in next event loop iteration
    connect(m_layoutBatchTimer, &QTimer::timeout, this, &FlexLayoutEngine::processPendingLayouts);
}

void FlexLayoutEngine::layoutChildren(Frame* parentFrame, ElementModel* elementModel, LayoutReason reason)
{
    if (!parentFrame || !elementModel) {
        return;
    }
    
    // Only apply flex layout if Frame has flex enabled
    if (!parentFrame->flex()) {
        // Clear stored margins when flex is disabled
        m_frameMargins.remove(parentFrame->getId());
        return;
    }
    
    // Prevent infinite recursion
    if (m_isLayouting) {
        qDebug() << "FlexLayoutEngine::layoutChildren - Already layouting, skipping to prevent recursion";
        return;
    }
    
    // RAII guard to ensure m_isLayouting is reset when we exit this function
    struct LayoutGuard {
        bool& flag;
        LayoutGuard(bool& f) : flag(f) { flag = true; }
        ~LayoutGuard() { flag = false; }
    } guard(m_isLayouting);
    
    qDebug() << "FlexLayoutEngine::layoutChildren - Starting layout for frame" << parentFrame->getId();
    
    // Get direct children
    QList<Element*> children = getDirectChildren(parentFrame->getId(), elementModel);
    
    // Filter to only include visual elements with position = Relative
    QList<Element*> layoutChildren;
    for (Element* child : children) {
        if (child->isVisual()) {
            CanvasElement* canvasChild = qobject_cast<CanvasElement*>(child);
            if (canvasChild) {
                // Check if child is a Frame with position = Relative
                Frame* childFrame = qobject_cast<Frame*>(child);
                if (childFrame) {
                    qDebug() << "FlexLayoutEngine - Child frame" << childFrame->getId() 
                             << "position:" << childFrame->position();
                    if (childFrame->position() == Frame::Relative) {
                        layoutChildren.append(child);
                    }
                } else {
                    // Check if child is a WebTextInput with position = Relative
                    WebTextInput* webTextInput = qobject_cast<WebTextInput*>(child);
                    if (webTextInput) {
                        qDebug() << "FlexLayoutEngine - Child WebTextInput" << webTextInput->getId() 
                                 << "position:" << webTextInput->position();
                        if (webTextInput->position() == WebTextInput::Relative) {
                            layoutChildren.append(child);
                        }
                    } else {
                        // Check if child is a Text with position = Relative
                        Text* textElement = qobject_cast<Text*>(child);
                        if (textElement) {
                            qDebug() << "FlexLayoutEngine - Child Text" << textElement->getId() 
                                     << "position:" << textElement->position();
                            if (textElement->position() == Text::Relative) {
                                layoutChildren.append(child);
                            }
                        } else {
                            qDebug() << "FlexLayoutEngine - Child" << child->getId() << "is not a Frame, WebTextInput, or Text";
                        }
                    }
                }
            }
        }
    }
    
    if (layoutChildren.isEmpty()) {
        qDebug() << "FlexLayoutEngine::layoutChildren - No children to layout";
        return;  // Guard will reset flag automatically
    }
    
    qDebug() << "FlexLayoutEngine::layoutChildren - Found" << layoutChildren.count() << "children to layout";
    
    // Calculate layout bounds (parent's inner bounds)
    QRectF containerBounds(0, 0, parentFrame->width(), parentFrame->height());
    
    // Capture initial margins if not already set
    // Don't recapture if we already have margins stored (prevents collapse on selection change)
    QString parentId = parentFrame->getId();
    if (!m_frameMargins.contains(parentId) || !m_frameMargins[parentId].isInitialized()) {
        captureInitialMargins(parentFrame, layoutChildren);
    }
    
    // Calculate required size for parent frame
    QSizeF requiredSize = calculateRequiredSize(layoutChildren, parentFrame);
    
    // Round required sizes to nearest 0.5
    qreal roundedWidth = roundToHalf(requiredSize.width());
    qreal roundedHeight = roundToHalf(requiredSize.height());
    
    qDebug() << "FlexLayoutEngine - Parent" << parentFrame->getId() 
             << "current size:" << parentFrame->width() << "x" << parentFrame->height()
             << "calculated size:" << requiredSize.width() << "x" << requiredSize.height()
             << "rounded size:" << roundedWidth << "x" << roundedHeight;
    
    // Update parent frame size if needed (allow both growing and shrinking)
    // BUT only if:
    // 1. The parent frame is not currently selected (being resized by user) OR it's a gap change
    // 2. The parent's width/height type is set to "fit content"
    bool sizeChanged = false;
    const qreal tolerance = 0.25; // Tolerance of 0.25 since we round to nearest 0.5
    
    // Only prevent resize if the parent itself is selected AND this is not a gap change
    // Gap changes should always resize the parent to maintain margins
    bool canResize = !parentFrame->isSelected() || reason == GapChanged;
    
    if (canResize) {
        // Check width type - only resize width if it's set to fit content
        if (parentFrame->widthType() == Frame::SizeFitContent) {
            if (qAbs(roundedWidth - parentFrame->width()) > tolerance) {
                qDebug() << "FlexLayoutEngine - Updating parent width from" << parentFrame->width() 
                         << "to" << roundedWidth << "(widthType is fit-content)";
                parentFrame->setWidth(roundedWidth);
                sizeChanged = true;
            }
        } else {
            qDebug() << "FlexLayoutEngine - Skipping width resize because widthType is not fit-content";
        }
        
        // Check height type - only resize height if it's set to fit content
        if (parentFrame->heightType() == Frame::SizeFitContent) {
            if (qAbs(roundedHeight - parentFrame->height()) > tolerance) {
                qDebug() << "FlexLayoutEngine - Updating parent height from" << parentFrame->height() 
                         << "to" << roundedHeight << "(heightType is fit-content)";
                parentFrame->setHeight(roundedHeight);
                sizeChanged = true;
            }
        } else {
            qDebug() << "FlexLayoutEngine - Skipping height resize because heightType is not fit-content";
        }
    } else {
        qDebug() << "FlexLayoutEngine - Skipping parent resize because parent frame is selected";
    }
    
    // If size changed, recalculate bounds
    if (sizeChanged) {
        containerBounds = QRectF(0, 0, parentFrame->width(), parentFrame->height());
    }
    
    // Temporarily disconnect geometry signals to avoid triggering additional layouts
    disconnectChildGeometrySignals(parentFrame);
    
    // Perform layout
    calculateFlexLayout(layoutChildren, parentFrame, containerBounds);
    
    // Reconnect geometry signals after layout is complete
    connectChildGeometrySignals(parentFrame, elementModel);
    
    // Guard will automatically reset m_isLayouting when function exits
}

QList<Element*> FlexLayoutEngine::getDirectChildren(const QString& parentId, ElementModel* elementModel) const
{
    QList<Element*> children;
    QList<Element*> allElements = elementModel->getAllElements();
    
    for (Element* element : allElements) {
        if (element->getParentElementId() == parentId) {
            children.append(element);
        }
    }
    
    return children;
}

void FlexLayoutEngine::calculateFlexLayout(const QList<Element*>& children, 
                                         Frame* parentFrame,
                                         const QRectF& containerBounds)
{
    if (parentFrame->orientation() == Frame::Row) {
        layoutRow(children, parentFrame, containerBounds);
    } else {
        layoutColumn(children, parentFrame, containerBounds);
    }
}

void FlexLayoutEngine::layoutRow(const QList<Element*>& children, 
                               Frame* parentFrame,
                               const QRectF& containerBounds)
{
    if (children.isEmpty()) return;
    
    // Calculate total width of all children and gaps
    qreal totalChildrenWidth = 0;
    for (Element* child : children) {
        CanvasElement* canvasChild = qobject_cast<CanvasElement*>(child);
        if (canvasChild) {
            totalChildrenWidth += canvasChild->width();
        }
    }
    
    qreal gap = parentFrame->gap();
    qreal totalGaps = gap * (children.count() - 1);
    
    // Calculate starting X position based on justify
    qreal startX = calculateMainAxisPosition(containerBounds.width(),
                                           totalChildrenWidth,
                                           totalGaps,
                                           children.count(),
                                           parentFrame->justify());
    
    // Position each child
    qreal currentX = startX;
    for (int i = 0; i < children.count(); ++i) {
        CanvasElement* canvasChild = qobject_cast<CanvasElement*>(children[i]);
        if (!canvasChild) continue;
        
        // Set X position (relative to parent)
        qreal absoluteX = roundToHalf(parentFrame->x() + currentX);
        if (!qFuzzyCompare(canvasChild->x(), absoluteX)) {
            qDebug() << "FlexLayoutEngine::layoutRow - Setting child" << canvasChild->getId() 
                     << "X from" << canvasChild->x() << "to" << absoluteX;
            canvasChild->setX(absoluteX);
        }
        
        // Calculate Y position based on align (relative to parent)
        qreal y = calculateCrossAxisPosition(containerBounds.height(),
                                           canvasChild->height(),
                                           parentFrame->align());
        qreal absoluteY = roundToHalf(parentFrame->y() + y);
        if (!qFuzzyCompare(canvasChild->y(), absoluteY)) {
            qDebug() << "FlexLayoutEngine::layoutRow - Setting child" << canvasChild->getId() 
                     << "Y from" << canvasChild->y() << "to" << absoluteY;
            canvasChild->setY(absoluteY);
        }
        
        // Move to next position based on child's actual width
        currentX += canvasChild->width();
        if (i < children.count() - 1) {
            currentX += gap;
            
            // Handle space-between, space-around, space-evenly
            if (parentFrame->justify() == Frame::JustifySpaceBetween && children.count() > 1) {
                qreal extraSpace = (containerBounds.width() - totalChildrenWidth) / (children.count() - 1) - gap;
                currentX += extraSpace;
            } else if (parentFrame->justify() == Frame::JustifySpaceAround) {
                qreal extraSpace = (containerBounds.width() - totalChildrenWidth) / children.count() - gap;
                currentX += extraSpace;
            } else if (parentFrame->justify() == Frame::JustifySpaceEvenly) {
                qreal extraSpace = (containerBounds.width() - totalChildrenWidth) / (children.count() + 1) - gap;
                currentX += extraSpace;
            }
        }
    }
}

void FlexLayoutEngine::layoutColumn(const QList<Element*>& children, 
                                  Frame* parentFrame,
                                  const QRectF& containerBounds)
{
    if (children.isEmpty()) return;
    
    // Calculate total height of all children and gaps
    qreal totalChildrenHeight = 0;
    for (Element* child : children) {
        CanvasElement* canvasChild = qobject_cast<CanvasElement*>(child);
        if (canvasChild) {
            totalChildrenHeight += canvasChild->height();
        }
    }
    
    qreal gap = parentFrame->gap();
    qreal totalGaps = gap * (children.count() - 1);
    
    // Calculate starting Y position based on justify
    qreal startY = calculateMainAxisPosition(containerBounds.height(),
                                           totalChildrenHeight,
                                           totalGaps,
                                           children.count(),
                                           parentFrame->justify());
    
    // Position each child
    qreal currentY = startY;
    for (int i = 0; i < children.count(); ++i) {
        CanvasElement* canvasChild = qobject_cast<CanvasElement*>(children[i]);
        if (!canvasChild) continue;
        
        // Set Y position (relative to parent)
        qreal absoluteY = roundToHalf(parentFrame->y() + currentY);
        if (!qFuzzyCompare(canvasChild->y(), absoluteY)) {
            canvasChild->setY(absoluteY);
        }
        
        // Calculate X position based on align (relative to parent)
        qreal x = calculateCrossAxisPosition(containerBounds.width(),
                                           canvasChild->width(),
                                           parentFrame->align());
        qreal absoluteX = roundToHalf(parentFrame->x() + x);
        if (!qFuzzyCompare(canvasChild->x(), absoluteX)) {
            canvasChild->setX(absoluteX);
        }
        
        // Move to next position based on child's actual height
        currentY += canvasChild->height();
        if (i < children.count() - 1) {
            currentY += gap;
            
            // Handle space-between, space-around, space-evenly
            if (parentFrame->justify() == Frame::JustifySpaceBetween && children.count() > 1) {
                qreal extraSpace = (containerBounds.height() - totalChildrenHeight) / (children.count() - 1) - gap;
                currentY += extraSpace;
            } else if (parentFrame->justify() == Frame::JustifySpaceAround) {
                qreal extraSpace = (containerBounds.height() - totalChildrenHeight) / children.count() - gap;
                currentY += extraSpace;
            } else if (parentFrame->justify() == Frame::JustifySpaceEvenly) {
                qreal extraSpace = (containerBounds.height() - totalChildrenHeight) / (children.count() + 1) - gap;
                currentY += extraSpace;
            }
        }
    }
}

qreal FlexLayoutEngine::calculateMainAxisPosition(qreal containerSize,
                                                qreal totalChildrenSize,
                                                qreal totalGaps,
                                                int childCount,
                                                Frame::JustifyContent justify) const
{
    switch (justify) {
        case Frame::JustifyStart:
            return 0;
            
        case Frame::JustifyEnd:
            return containerSize - totalChildrenSize - totalGaps;
            
        case Frame::JustifyCenter:
            return (containerSize - totalChildrenSize - totalGaps) / 2;
            
        case Frame::JustifySpaceBetween:
            return 0; // Start at 0, spacing handled in layout methods
            
        case Frame::JustifySpaceAround:
            // Space on sides is half of space between elements
            if (childCount > 0) {
                qreal totalSpace = containerSize - totalChildrenSize - totalGaps;
                qreal spacePerItem = totalSpace / childCount;
                return spacePerItem / 2;
            }
            return 0;
            
        case Frame::JustifySpaceEvenly:
            // Equal space before, between, and after all elements
            if (childCount > 0) {
                qreal totalSpace = containerSize - totalChildrenSize - totalGaps;
                qreal spacePerGap = totalSpace / (childCount + 1);
                return spacePerGap;
            }
            return 0;
            
        default:
            return 0;
    }
}

qreal FlexLayoutEngine::calculateCrossAxisPosition(qreal containerSize,
                                                 qreal childSize,
                                                 Frame::AlignItems align) const
{
    switch (align) {
        case Frame::AlignStart:
            return 0;
            
        case Frame::AlignEnd:
            return containerSize - childSize;
            
        case Frame::AlignCenter:
            return (containerSize - childSize) / 2;
            
        case Frame::AlignBaseline:
            // For now, treat baseline like start
            // TODO: Implement proper baseline alignment
            return 0;
            
        case Frame::AlignStretch:
            // Stretch is handled differently - child should fill container
            // For now, just position at start
            // TODO: Implement stretch by modifying child size
            return 0;
            
        default:
            return 0;
    }
}

void FlexLayoutEngine::connectChildGeometrySignals(Frame* parentFrame, ElementModel* elementModel)
{
    if (!parentFrame || !elementModel) {
        return;
    }
    
    // Get all direct children
    QList<Element*> children = getDirectChildren(parentFrame->getId(), elementModel);
    
    for (Element* child : children) {
        if (child->isVisual()) {
            CanvasElement* canvasChild = qobject_cast<CanvasElement*>(child);
            if (canvasChild) {
                QString childId = canvasChild->getId();
                
                // Disconnect any existing connections for this child
                if (m_childConnections.contains(childId)) {
                    QObject::disconnect(m_childConnections[childId]);
                    m_childConnections.remove(childId);
                }
                
                // Also disconnect position connection if it exists
                QString positionConnId = childId + "_position";
                if (m_childConnections.contains(positionConnId)) {
                    QObject::disconnect(m_childConnections[positionConnId]);
                    m_childConnections.remove(positionConnId);
                }
                
                // Connect to geometry changes
                QMetaObject::Connection conn = connect(canvasChild, &CanvasElement::geometryChanged,
                                                     this, [this, parentFrame, canvasChild]() {
                                                         if (parentFrame->flex()) {
                                                             // Check if this child is currently selected (being resized)
                                                             Application* app = Application::instance();
                                                             if (app && app->activeCanvas() && app->activeCanvas()->selectionManager()) {
                                                                 SelectionManager* selectionManager = app->activeCanvas()->selectionManager();
                                                                 if (selectionManager->isSelected(canvasChild)) {
                                                                     qDebug() << "FlexLayoutEngine - Skipping layout for selected child" << canvasChild->getId();
                                                                     return; // Skip scheduling layout when child is being resized
                                                                 }
                                                             }
                                                             scheduleLayout(parentFrame);
                                                         }
                                                     });
                
                m_childConnections[childId] = conn;
                
                // For Frame children, also connect to position changes
                Frame* childFrame = qobject_cast<Frame*>(child);
                if (childFrame) {
                    QString positionConnId = childId + "_position";
                    QMetaObject::Connection posConn = connect(childFrame, &Frame::positionChanged,
                                                           this, [this, parentFrame]() {
                                                               if (parentFrame->flex()) {
                                                                   scheduleLayout(parentFrame);
                                                               }
                                                           });
                    m_childConnections[positionConnId] = posConn;
                } else {
                    // For WebTextInput children, also connect to position changes
                    WebTextInput* webTextInput = qobject_cast<WebTextInput*>(child);
                    if (webTextInput) {
                        QString positionConnId = childId + "_position";
                        QMetaObject::Connection posConn = connect(webTextInput, &WebTextInput::positionChanged,
                                                               this, [this, parentFrame]() {
                                                                   if (parentFrame->flex()) {
                                                                       scheduleLayout(parentFrame);
                                                                   }
                                                               });
                        m_childConnections[positionConnId] = posConn;
                    } else {
                        // For Text children, also connect to position changes
                        Text* textElement = qobject_cast<Text*>(child);
                        if (textElement) {
                            QString positionConnId = childId + "_position";
                            QMetaObject::Connection posConn = connect(textElement, &Text::positionChanged,
                                                                   this, [this, parentFrame]() {
                                                                       if (parentFrame->flex()) {
                                                                           scheduleLayout(parentFrame);
                                                                       }
                                                                   });
                            m_childConnections[positionConnId] = posConn;
                        }
                    }
                }
                
                qDebug() << "FlexLayoutEngine - Connected geometry signal for child" << childId 
                         << "of parent" << parentFrame->getId();
            }
        }
    }
}

void FlexLayoutEngine::disconnectChildGeometrySignals(Frame* parentFrame)
{
    if (!parentFrame) {
        return;
    }
    
    // Disconnect all tracked connections
    for (auto it = m_childConnections.begin(); it != m_childConnections.end(); ++it) {
        QObject::disconnect(it.value());
    }
    m_childConnections.clear();
    
    qDebug() << "FlexLayoutEngine - Disconnected all child geometry signals for parent" 
             << parentFrame->getId();
}

void FlexLayoutEngine::captureInitialMargins(Frame* parentFrame, const QList<Element*>& children)
{
    if (!parentFrame || children.isEmpty()) {
        return;
    }
    
    QString parentId = parentFrame->getId();
    FrameMargins& margins = m_frameMargins[parentId];
    
    // Find the bounding box of all children
    qreal minX = std::numeric_limits<qreal>::max();
    qreal maxX = std::numeric_limits<qreal>::lowest();
    qreal minY = std::numeric_limits<qreal>::max();
    qreal maxY = std::numeric_limits<qreal>::lowest();
    
    for (Element* child : children) {
        CanvasElement* canvasChild = qobject_cast<CanvasElement*>(child);
        if (canvasChild) {
            // Get child bounds relative to parent
            qreal childLeft = canvasChild->x() - parentFrame->x();
            qreal childTop = canvasChild->y() - parentFrame->y();
            qreal childRight = childLeft + canvasChild->width();
            qreal childBottom = childTop + canvasChild->height();
            
            minX = qMin(minX, childLeft);
            maxX = qMax(maxX, childRight);
            minY = qMin(minY, childTop);
            maxY = qMax(maxY, childBottom);
        }
    }
    
    // Calculate margins as the distance from children bounds to parent edges
    margins.left = minX;
    margins.right = parentFrame->width() - maxX;
    margins.top = minY;
    margins.bottom = parentFrame->height() - maxY;
    
    qDebug() << "FlexLayoutEngine::captureInitialMargins - Frame" << parentId
             << "margins: left=" << margins.left << "right=" << margins.right
             << "top=" << margins.top << "bottom=" << margins.bottom;
}

void FlexLayoutEngine::resetMargins(const QString& frameId)
{
    m_frameMargins.remove(frameId);
    qDebug() << "FlexLayoutEngine::resetMargins - Reset margins for frame" << frameId;
}

QSizeF FlexLayoutEngine::calculateRequiredSize(const QList<Element*>& children, 
                                              Frame* parentFrame) const
{
    if (children.isEmpty()) {
        return QSizeF(0, 0);
    }
    
    QString parentId = parentFrame->getId();
    
    // Calculate required size based on child dimensions and layout properties
    qreal totalWidth = 0;
    qreal totalHeight = 0;
    qreal maxWidth = 0;
    qreal maxHeight = 0;
    
    // Calculate total size of children
    for (Element* child : children) {
        CanvasElement* canvasChild = qobject_cast<CanvasElement*>(child);
        if (canvasChild) {
            if (parentFrame->orientation() == Frame::Row) {
                totalWidth += canvasChild->width();
                maxHeight = qMax(maxHeight, canvasChild->height());
            } else {
                totalHeight += canvasChild->height();
                maxWidth = qMax(maxWidth, canvasChild->width());
            }
        }
    }
    
    // Add gaps
    qreal gap = parentFrame->gap();
    qreal totalGaps = gap * (children.count() - 1);
    
    // Use stored margins if available, otherwise use default padding
    qreal paddingLeft = 10.0;
    qreal paddingRight = 10.0;
    qreal paddingTop = 10.0;
    qreal paddingBottom = 10.0;
    
    if (m_frameMargins.contains(parentId) && m_frameMargins[parentId].isInitialized()) {
        const FrameMargins& margins = m_frameMargins[parentId];
        paddingLeft = margins.left;
        paddingRight = margins.right;
        paddingTop = margins.top;
        paddingBottom = margins.bottom;
    }
    
    QSizeF result;
    if (parentFrame->orientation() == Frame::Row) {
        totalWidth += totalGaps;
        result = QSizeF(totalWidth + paddingLeft + paddingRight, 
                       maxHeight + paddingTop + paddingBottom);
    } else {
        totalHeight += totalGaps;
        result = QSizeF(maxWidth + paddingLeft + paddingRight, 
                       totalHeight + paddingTop + paddingBottom);
    }
    
    return result;
}

void FlexLayoutEngine::scheduleLayout(Frame* parentFrame, LayoutReason reason)
{
    if (!parentFrame || !parentFrame->flex()) {
        return;
    }
    
    qDebug() << "FlexLayoutEngine::scheduleLayout - Scheduling layout for frame" << parentFrame->getId() 
             << "reason:" << reason;
    
    // Add to pending layouts (update reason if more specific)
    PendingLayout pending = { parentFrame, reason };
    if (m_pendingLayoutFrames.contains(parentFrame)) {
        // If we already have a pending layout, keep the most specific reason
        if (reason != General) {
            m_pendingLayoutFrames[parentFrame] = pending;
        }
        qDebug() << "FlexLayoutEngine::scheduleLayout - Frame already pending, updating reason";
    } else {
        m_pendingLayoutFrames[parentFrame] = pending;
    }
    
    // Start the timer if not already running
    if (!m_layoutBatchTimer->isActive()) {
        m_layoutBatchTimer->start();
        qDebug() << "FlexLayoutEngine::scheduleLayout - Starting batch timer";
    } else {
        qDebug() << "FlexLayoutEngine::scheduleLayout - Batch timer already active";
    }
}

void FlexLayoutEngine::processPendingLayouts()
{
    qDebug() << "FlexLayoutEngine::processPendingLayouts - Processing" << m_pendingLayoutFrames.size() << "pending layouts";
    
    // Process all pending layout requests
    QMap<Frame*, PendingLayout> pendingLayouts = m_pendingLayoutFrames;
    m_pendingLayoutFrames.clear();
    
    // Get the application and element model
    Application* app = Application::instance();
    if (!app || !app->activeCanvas() || !app->activeCanvas()->elementModel()) {
        return;
    }
    
    ElementModel* elementModel = app->activeCanvas()->elementModel();
    
    // Layout each frame
    for (auto it = pendingLayouts.begin(); it != pendingLayouts.end(); ++it) {
        Frame* frame = it.key();
        LayoutReason reason = it.value().reason;
        if (frame && frame->flex()) {
            qDebug() << "FlexLayoutEngine::processPendingLayouts - Laying out frame" << frame->getId() << "with reason" << reason;
            layoutChildren(frame, elementModel, reason);
        }
    }
}