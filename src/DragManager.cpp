#include "DragManager.h"
#include "Element.h"
#include "CanvasElement.h"
#include "SelectionManager.h"
#include "ElementModel.h"
#include <QtMath>
#include <QDebug>

DragManager::DragManager(QObject *parent)
    : QObject(parent)
{
}

bool DragManager::startDrag(Element* element, const QPointF& startPos)
{
    if (!element || !m_selectionManager) return false;
    
    m_dragElement = element;
    m_isDragging = true;
    m_dragStartPos = startPos;
    m_totalDelta = QPointF(0, 0);
    m_hasDraggedMinDistance = false;
    
    // Clear previous drag states
    m_draggedElements.clear();
    
    // Get all selected elements
    QList<Element*> selectedElements = m_selectionManager->selectedElements();
    
    // If the clicked element is not selected, we'll only drag it alone
    // Otherwise, drag all selected elements
    if (m_dragElement->isSelected()) {
        // Store initial positions of all selected elements
        for (Element *elem : selectedElements) {
            // Only visual elements can be dragged
            if (elem->isVisual()) {
                CanvasElement* canvasElement = qobject_cast<CanvasElement*>(elem);
                if (canvasElement) {
                    ElementDragState state;
                    state.element = elem;
                    state.originalPosition = QPointF(canvasElement->x(), canvasElement->y());
                    m_draggedElements.append(state);
                }
            }
        }
    } else {
        // Only drag the clicked element (it's not selected)
        if (m_dragElement->isVisual()) {
            CanvasElement* canvasElement = qobject_cast<CanvasElement*>(m_dragElement);
            if (canvasElement) {
                ElementDragState state;
                state.element = m_dragElement;
                state.originalPosition = QPointF(canvasElement->x(), canvasElement->y());
                m_draggedElements.append(state);
            }
        }
    }
    
    emit isDraggingChanged();
    emit dragStarted();
    return true;
}

void DragManager::updateDrag(const QPointF& currentPos)
{
    if (!m_isDragging || m_draggedElements.isEmpty()) return;
    
    // Calculate the delta from the drag start position
    qreal deltaX = currentPos.x() - m_dragStartPos.x();
    qreal deltaY = currentPos.y() - m_dragStartPos.y();
    m_totalDelta = QPointF(deltaX, deltaY);
    
    // Check if we've moved enough to consider this a drag
    if (!m_hasDraggedMinDistance) {
        qreal distance = qSqrt(deltaX * deltaX + deltaY * deltaY);
        if (distance > DRAG_THRESHOLD) {
            m_hasDraggedMinDistance = true;
        }
    }
    
    // Only move elements if we've established this is a drag
    if (m_hasDraggedMinDistance) {
        // Move all selected elements by the same delta
        for (const ElementDragState &state : m_draggedElements) {
            if (state.element->isVisual()) {
                CanvasElement* canvasElement = qobject_cast<CanvasElement*>(state.element);
                if (canvasElement) {
                    canvasElement->setX(state.originalPosition.x() + deltaX);
                    canvasElement->setY(state.originalPosition.y() + deltaY);
                }
            }
        }
        emit dragUpdated();
    }
}

void DragManager::endDrag()
{
    // Handle click vs drag selection before clearing state
    if (m_isDragging && !m_hasDraggedMinDistance && m_dragElement) {
        handleClickSelection(m_dragElement);
    }
    
    m_isDragging = false;
    m_dragElement = nullptr;
    m_draggedElements.clear();
    m_hasDraggedMinDistance = false;
    
    emit isDraggingChanged();
    emit dragEnded();
}

void DragManager::handleClickSelection(Element* clickedElement)
{
    if (!m_selectionManager || !clickedElement) return;
    
    if (m_selectionManager->selectionCount() > 1 && clickedElement->isSelected()) {
        // Multiple elements were selected and user clicked on one of them
        // Select only the clicked element
        m_selectionManager->selectOnly(clickedElement);
    } else if (!clickedElement->isSelected()) {
        // Clicked on an unselected element
        m_selectionManager->selectOnly(clickedElement);
    }
    // If single element is already selected, do nothing (keep it selected)
}

QList<Element*> DragManager::draggedElements() const
{
    QList<Element*> elements;
    for (const ElementDragState& state : m_draggedElements) {
        elements.append(state.element);
    }
    return elements;
}