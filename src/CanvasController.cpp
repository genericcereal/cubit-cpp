#include "CanvasController.h"
#include "Element.h"
#include "Frame.h"
#include "Text.h"
#include "Html.h"
#include "Variable.h"
#include "Node.h"
#include "Edge.h"
#include "ElementModel.h"
#include "SelectionManager.h"
#include "Config.h"
#include "UniqueIdGenerator.h"
#include <QDebug>
#include <QtMath>

CanvasController::CanvasController(QObject *parent)
    : QObject(parent)
    , m_mode("select")
    , m_canvasType("design")
    , m_elementModel(nullptr)
    , m_selectionManager(nullptr)
    , m_isDragging(false)
    , m_dragElement(nullptr)
    , m_hasDraggedMinDistance(false)
{
}

void CanvasController::setMode(const QString &mode)
{
    qDebug() << "CanvasController::setMode called - current:" << m_mode << "new:" << mode;
    if (m_mode != mode) {
        m_mode = mode;
        qDebug() << "Mode changed, emitting signal";
        emit modeChanged();
    }
}

void CanvasController::setCanvasType(const QString &type)
{
    if (m_canvasType != type) {
        m_canvasType = type;
        emit canvasTypeChanged();
    }
}

void CanvasController::handleMousePress(qreal x, qreal y)
{
    if (!m_elementModel || !m_selectionManager) return;
    
    if (m_mode == "select") {
        Element *element = hitTest(x, y);
        if (element) {
            // Don't change selection on press - wait to see if it's a click or drag
            // Just prepare for potential drag
            startDrag(x, y);
        } else {
            m_selectionManager->clearSelection();
        }
    } else {
        // Start element creation
        m_dragStartPos = QPointF(x, y);
        m_isDragging = true;
        
        // Create element immediately for frame mode
        if (m_mode == "frame" || m_mode == "text" || m_mode == "html") {
            createElement(m_mode, x, y, 1, 1);
            // The last created element becomes the drag element
            if (m_elementModel && m_elementModel->rowCount() > 0) {
                m_dragElement = m_elementModel->elementAt(m_elementModel->rowCount() - 1);
                qDebug() << "Frame created, dragElement:" << m_dragElement << "isDragging:" << m_isDragging;
            }
        }
    }
}

void CanvasController::handleMouseMove(qreal x, qreal y)
{
    if (!m_isDragging) return;
    
    if (m_dragElement) {
        if (m_mode == "select") {
            updateDrag(x, y);
        } else if (m_mode == "frame" || m_mode == "text" || m_mode == "html") {
            // Update frame size during creation
            qreal width = qAbs(x - m_dragStartPos.x());
            qreal height = qAbs(y - m_dragStartPos.y());
            qreal left = qMin(x, m_dragStartPos.x());
            qreal top = qMin(y, m_dragStartPos.y());
            
            m_dragElement->setX(left);
            m_dragElement->setY(top);
            m_dragElement->setWidth(qMax(width, 1.0));
            m_dragElement->setHeight(qMax(height, 1.0));
            
            qDebug() << "Frame resize:" << left << top << width << height;
        }
    } else if (m_mode != "select") {
        m_dragCurrentPos = QPointF(x, y);
    }
}

void CanvasController::handleMouseRelease(qreal x, qreal y)
{
    if (!m_isDragging) return;
    
    if (m_mode == "select") {
        // Check if this was a click (no significant movement) vs a drag
        if (!m_hasDraggedMinDistance && m_dragElement) {
            // This was a click - update selection
            if (m_selectionManager->selectionCount() > 1 && m_dragElement->isSelected()) {
                // Multiple elements were selected and user clicked on one of them
                // Select only the clicked element
                m_selectionManager->selectOnly(m_dragElement);
            } else if (!m_dragElement->isSelected()) {
                // Clicked on an unselected element
                m_selectionManager->selectOnly(m_dragElement);
            }
        }
        endDrag();
    } else if (m_mode == "frame" || m_mode == "text" || m_mode == "html") {
        // Finalize frame creation
        if (m_dragElement) {
            // Ensure minimum size
            if (m_dragElement->width() < 10) m_dragElement->setWidth(Config::DEFAULT_ELEMENT_WIDTH);
            if (m_dragElement->height() < 10) m_dragElement->setHeight(Config::DEFAULT_ELEMENT_HEIGHT);
            
            // Select the newly created frame
            if (m_selectionManager) {
                m_selectionManager->selectOnly(m_dragElement);
            }
            
            // Switch back to select mode
            setMode("select");
        }
    } else {
        // Create other element types on release
        qreal width = qAbs(x - m_dragStartPos.x());
        qreal height = qAbs(y - m_dragStartPos.y());
        qreal left = qMin(x, m_dragStartPos.x());
        qreal top = qMin(y, m_dragStartPos.y());
        
        if (width < 10) width = Config::DEFAULT_ELEMENT_WIDTH;
        if (height < 10) height = Config::DEFAULT_ELEMENT_HEIGHT;
        
        createElement(m_mode, left, top, width, height);
    }
    
    m_isDragging = false;
    m_dragElement = nullptr;
}

void CanvasController::createElement(const QString &type, qreal x, qreal y, qreal width, qreal height)
{
    if (!m_elementModel) return;
    
    // Only create elements for design canvas
    if (m_canvasType != "design") {
        qDebug() << "Cannot create elements on" << m_canvasType << "canvas";
        return;
    }
    
    QString id = m_elementModel->generateId();
    Element *element = nullptr;
    
    qDebug() << "Creating element with unique ID:" << id;
    
    if (type == "frame") {
        element = new Frame(id);
    } else if (type == "text") {
        // Create frame with text
        element = new Text(id);
    } else if (type == "html") {
        element = new Frame(id);
    } else if (type == "variable") {
        element = new Variable(id);
    }
    
    if (element) {
        element->setRect(QRectF(x, y, width, height));
        m_elementModel->addElement(element);
        emit elementCreated(element);
        
        // Select the newly created element
        if (m_selectionManager) {
            m_selectionManager->selectOnly(element);
        }
    }
}

Element* CanvasController::hitTest(qreal x, qreal y)
{
    if (!m_elementModel) return nullptr;
    
    // Test in reverse order (top to bottom) for all canvas types
    QList<Element*> elements = m_elementModel->getAllElements();
    for (int i = elements.size() - 1; i >= 0; --i) {
        Element *element = elements[i];
        
        // For script canvas, test nodes and edges
        if (m_canvasType == "script") {
            if (element->getType() != Element::NodeType && element->getType() != Element::EdgeType) {
                continue;
            }
        }
        // For design canvas, skip nodes
        else if (m_canvasType == "design") {
            if (element->getType() == Element::NodeType || element->getType() == Element::EdgeType) {
                continue;
            }
        }
        
        if (element->containsPoint(QPointF(x, y))) {
            return element;
        }
    }
    
    return nullptr;
}

void CanvasController::startDrag(qreal x, qreal y)
{
    m_dragElement = hitTest(x, y);
    if (m_dragElement) {
        m_isDragging = true;
        m_dragStartPos = QPointF(x, y);
        m_hasDraggedMinDistance = false;
        
        // Clear previous drag states
        m_draggedElements.clear();
        
        // Get all selected elements
        QList<Element*> selectedElements = m_selectionManager->selectedElements();
        
        // If the clicked element is not selected, we'll only drag it alone
        // Otherwise, drag all selected elements
        if (m_dragElement->isSelected()) {
            // Store initial positions of all selected elements
            for (Element *element : selectedElements) {
                ElementDragState state;
                state.element = element;
                state.originalPosition = QPointF(element->x(), element->y());
                m_draggedElements.append(state);
            }
        } else {
            // Only drag the clicked element (it's not selected)
            ElementDragState state;
            state.element = m_dragElement;
            state.originalPosition = QPointF(m_dragElement->x(), m_dragElement->y());
            m_draggedElements.append(state);
        }
    }
}

void CanvasController::updateDrag(qreal x, qreal y)
{
    if (m_isDragging && !m_draggedElements.isEmpty()) {
        // Calculate the delta from the drag start position
        qreal deltaX = x - m_dragStartPos.x();
        qreal deltaY = y - m_dragStartPos.y();
        
        // Check if we've moved enough to consider this a drag (3 pixel threshold)
        if (!m_hasDraggedMinDistance) {
            qreal distance = qSqrt(deltaX * deltaX + deltaY * deltaY);
            if (distance > 3.0) {
                m_hasDraggedMinDistance = true;
            }
        }
        
        // Only move elements if we've established this is a drag
        if (m_hasDraggedMinDistance) {
            // Move all selected elements by the same delta
            for (const ElementDragState &state : m_draggedElements) {
                state.element->setX(state.originalPosition.x() + deltaX);
                state.element->setY(state.originalPosition.y() + deltaY);
            }
        }
    }
}

void CanvasController::endDrag()
{
    m_isDragging = false;
    m_dragElement = nullptr;
    m_draggedElements.clear();
    m_hasDraggedMinDistance = false;
}

void CanvasController::selectElementsInRect(const QRectF &rect)
{
    if (!m_elementModel || !m_selectionManager) return;
    
    QList<Element*> elementsToSelect;
    QList<Element*> allElements = m_elementModel->getAllElements();
    
    for (Element *element : allElements) {
        // For script canvas, only select nodes
        if (m_canvasType == "script") {
            if (element->getType() != Element::NodeType) {
                continue;
            }
        }
        // For design canvas, skip nodes and edges
        else if (m_canvasType == "design") {
            if (element->getType() == Element::NodeType || element->getType() == Element::EdgeType) {
                continue;
            }
        }
        
        if (rect.intersects(element->rect())) {
            elementsToSelect.append(element);
        }
    }
    
    if (!elementsToSelect.isEmpty()) {
        m_selectionManager->selectAll(elementsToSelect);
    } else {
        // Clear selection if no elements are in the rect
        m_selectionManager->clearSelection();
    }
}

void CanvasController::selectAll()
{
    if (!m_elementModel || !m_selectionManager) return;
    
    m_selectionManager->selectAll(m_elementModel->getAllElements());
}

void CanvasController::deleteSelectedElements()
{
    if (!m_elementModel || !m_selectionManager) return;
    
    QList<Element*> selectedElements = m_selectionManager->selectedElements();
    
    // Clear selection first
    m_selectionManager->clearSelection();
    
    // Delete elements
    for (Element *element : selectedElements) {
        m_elementModel->removeElement(element->getId());
    }
}

void CanvasController::createNode(qreal x, qreal y, const QString &title, const QString &color)
{
    if (!m_elementModel) return;
    
    // Only create nodes for script canvas
    if (m_canvasType != "script") {
        qDebug() << "Cannot create nodes on" << m_canvasType << "canvas";
        return;
    }
    
    QString id = m_elementModel->generateId();
    Node *node = new Node(id);
    
    if (node) {
        node->setX(x);
        node->setY(y);
        node->setNodeTitle(title);
        if (!color.isEmpty()) {
            node->setNodeColor(QColor(color));
        }
        // If no color specified, it will use the default from Config
        
        qDebug() << "Created node with ID:" << node->getId() << "title:" << title;
        
        m_elementModel->addElement(node);
        emit elementCreated(node);
    }
}

void CanvasController::createEdge(const QString &sourceNodeId, const QString &targetNodeId, 
                                  const QString &sourceHandleType, const QString &targetHandleType,
                                  int sourcePortIndex, int targetPortIndex)
{
    if (!m_elementModel) return;
    
    // Only create edges for script canvas
    if (m_canvasType != "script") {
        qDebug() << "Cannot create edges on" << m_canvasType << "canvas";
        return;
    }
    
    // Find the source and target nodes
    Element *sourceNode = m_elementModel->getElementById(sourceNodeId);
    Element *targetNode = m_elementModel->getElementById(targetNodeId);
    
    if (!sourceNode || !targetNode) {
        qDebug() << "Cannot create edge: source or target node not found";
        return;
    }
    
    QString id = m_elementModel->generateId();
    Edge *edge = new Edge(id);
    
    if (edge) {
        qDebug() << "Creating edge with ID:" << id;
        
        // Set connections
        edge->setSourceNodeId(sourceNodeId);
        edge->setTargetNodeId(targetNodeId);
        edge->setSourceHandleType(sourceHandleType);
        edge->setTargetHandleType(targetHandleType);
        edge->setSourcePortIndex(sourcePortIndex);
        edge->setTargetPortIndex(targetPortIndex);
        
        // Calculate connection points based on port indices
        qreal sourceX, sourceY, targetX, targetY;
        
        // Source point calculation
        if (sourceHandleType == "right") {
            sourceX = sourceNode->x() + sourceNode->width();
        } else {
            sourceX = sourceNode->x();
        }
        // Calculate Y based on port index (title height + row offset)
        sourceY = sourceNode->y() + 60 + 15 + (sourcePortIndex * 40);
        
        // Target point calculation  
        if (targetHandleType == "left") {
            targetX = targetNode->x();
        } else {
            targetX = targetNode->x() + targetNode->width();
        }
        // Calculate Y based on port index
        targetY = targetNode->y() + 60 + 15 + (targetPortIndex * 40);
        
        edge->setSourcePoint(QPointF(sourceX, sourceY));
        edge->setTargetPoint(QPointF(targetX, targetY));
        
        qDebug() << "Edge points - Source:" << QPointF(sourceX, sourceY) 
                 << "Target:" << QPointF(targetX, targetY);
        qDebug() << "Edge bounds - Position:" << edge->x() << "," << edge->y()
                 << "Size:" << edge->width() << "x" << edge->height();
        
        m_elementModel->addElement(edge);
        emit elementCreated(edge);
        qDebug() << "Created edge from node" << sourceNodeId << "port" << sourcePortIndex
                 << "to node" << targetNodeId << "port" << targetPortIndex;
    }
}