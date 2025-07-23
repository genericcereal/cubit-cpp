#include "SelectionManager.h"
#include "Element.h"
#include "CanvasElement.h"
#include "Component.h"
#include "ElementModel.h"
#include "Application.h"
#include "Project.h"
#include <limits>
#include <QDebug>

SelectionManager::SelectionManager(QObject *parent)
    : QObject(parent)
{
}

bool SelectionManager::isSelected(Element *element) const
{
    return element && m_selectedElements.contains(element);  // O(1) with QSet
}

bool SelectionManager::hasVisualSelection() const
{
    // Check if any selected element is visual
    for (Element* element : m_selectedElements) {
        if (element && element->isVisual()) {
            return true;
        }
    }
    return false;
}

void SelectionManager::selectElement(Element *element)
{
    if (!element || isSelected(element)) return;
    
    m_selectedElements.insert(element);  // O(1) with QSet
    updateElementSelection(element, true);
    expandBoundingBox(element);  // Incremental update
    emit elementSelected(element);
    checkComponentVariantMembership();
    emit selectionChanged();
}

void SelectionManager::deselectElement(Element *element)
{
    if (!element || !isSelected(element)) return;
    
    m_selectedElements.remove(element);  // O(1) with QSet
    updateElementSelection(element, false);
    
    // When removing an element, we may need to recalculate if it was on the boundary
    shrinkBoundingBox();
    
    emit elementDeselected(element);
    checkComponentVariantMembership();
    emit selectionChanged();
}

void SelectionManager::toggleSelection(Element *element)
{
    if (!element) return;
    
    if (isSelected(element)) {
        deselectElement(element);
    } else {
        selectElement(element);
    }
}

void SelectionManager::selectOnly(Element *element)
{
    if (!element) {
        clearSelection();
        return;
    }
    
    // Clear current selection
    for (Element *e : m_selectedElements) {
        updateElementSelection(e, false);
    }
    m_selectedElements.clear();
    
    // Select only this element
    m_selectedElements.insert(element);  // O(1) with QSet
    updateElementSelection(element, true);
    
    // Reset bounding box to this single element
    if (element->isVisual()) {
        CanvasElement* canvasElement = qobject_cast<CanvasElement*>(element);
        if (canvasElement) {
            m_boundingX = canvasElement->x();
            m_boundingY = canvasElement->y();
            m_boundingWidth = canvasElement->width();
            m_boundingHeight = canvasElement->height();
        }
    } else {
        // Non-visual elements don't affect bounding box
        m_boundingX = 0;
        m_boundingY = 0;
        m_boundingWidth = 0;
        m_boundingHeight = 0;
    }
    
    emit elementSelected(element);
    checkComponentVariantMembership();
    emit selectionChanged();
}

void SelectionManager::selectAll(const std::vector<Element*> &elements)
{
    clearSelection();
    
    if (elements.empty()) return;
    
    // Initialize bounding box with first valid element
    bool firstElement = true;
    
    for (Element *element : elements) {
        if (element) {
            m_selectedElements.insert(element);  // O(1) with QSet
            updateElementSelection(element, true);
            
            if (element->isVisual()) {
                if (firstElement) {
                    // Initialize bounding box
                    CanvasElement* canvasElement = qobject_cast<CanvasElement*>(element);
                    if (canvasElement) {
                        m_boundingX = canvasElement->x();
                        m_boundingY = canvasElement->y();
                        m_boundingWidth = canvasElement->width();
                        m_boundingHeight = canvasElement->height();
                        firstElement = false;
                    }
                } else {
                    // Expand bounding box incrementally
                    expandBoundingBox(element);
                }
            }
            
            emit elementSelected(element);
        }
    }
    
    if (!m_selectedElements.isEmpty()) {
        checkComponentVariantMembership();
        emit selectionChanged();
    }
}

void SelectionManager::clearSelection()
{
    if (m_selectedElements.isEmpty()) return;
    
    for (Element *element : m_selectedElements) {
        updateElementSelection(element, false);
        emit elementDeselected(element);
    }
    
    m_selectedElements.clear();
    
    // Reset bounding box
    m_boundingX = 0;
    m_boundingY = 0;
    m_boundingWidth = 0;
    m_boundingHeight = 0;
    
    checkComponentVariantMembership();
    emit selectionChanged();
}

void SelectionManager::updateElementSelection(Element *element, bool selected)
{
    if (element) {
        element->setSelected(selected);
        
        if (selected) {
            // Connect to geometry changes when selected - single connection with UniqueConnection flag
            if (element->isVisual()) {
                CanvasElement* canvasElement = qobject_cast<CanvasElement*>(element);
                if (canvasElement) {
                    connect(canvasElement, &CanvasElement::geometryChanged, 
                            this, &SelectionManager::onElementGeometryChanged,
                            Qt::UniqueConnection);
                }
            }
        } else {
            // Disconnect when deselected
            if (element->isVisual()) {
                CanvasElement* canvasElement = qobject_cast<CanvasElement*>(element);
                if (canvasElement) {
                    disconnect(canvasElement, &CanvasElement::geometryChanged, 
                               this, &SelectionManager::onElementGeometryChanged);
                }
            }
        }
    }
}

void SelectionManager::expandBoundingBox(Element *element)
{
    if (!element || !element->isVisual()) return;
    
    CanvasElement* canvasElement = qobject_cast<CanvasElement*>(element);
    if (!canvasElement) return;
    
    if (m_selectedElements.size() == 1) {
        // First element, initialize bounding box
        m_boundingX = canvasElement->x();
        m_boundingY = canvasElement->y();
        m_boundingWidth = canvasElement->width();
        m_boundingHeight = canvasElement->height();
    } else {
        // Expand existing bounding box
        qreal elementRight = canvasElement->x() + canvasElement->width();
        qreal elementBottom = canvasElement->y() + canvasElement->height();
        qreal currentRight = m_boundingX + m_boundingWidth;
        qreal currentBottom = m_boundingY + m_boundingHeight;
        
        qreal newLeft = qMin(m_boundingX, canvasElement->x());
        qreal newTop = qMin(m_boundingY, canvasElement->y());
        qreal newRight = qMax(currentRight, elementRight);
        qreal newBottom = qMax(currentBottom, elementBottom);
        
        m_boundingX = newLeft;
        m_boundingY = newTop;
        m_boundingWidth = newRight - newLeft;
        m_boundingHeight = newBottom - newTop;
    }
}

void SelectionManager::shrinkBoundingBox()
{
    // When an element is removed, we need to check if it was on the boundary
    // If so, we need to recalculate. This is still O(N) but only when necessary
    if (m_selectedElements.isEmpty()) {
        m_boundingX = 0;
        m_boundingY = 0;
        m_boundingWidth = 0;
        m_boundingHeight = 0;
    } else {
        recalculateBoundingBox();
    }
}

void SelectionManager::recalculateBoundingBox()
{
    if (m_selectedElements.isEmpty()) {
        m_boundingX = 0;
        m_boundingY = 0;
        m_boundingWidth = 0;
        m_boundingHeight = 0;
        return;
    }
    
    qreal minX = std::numeric_limits<qreal>::max();
    qreal minY = std::numeric_limits<qreal>::max();
    qreal maxX = std::numeric_limits<qreal>::lowest();
    qreal maxY = std::numeric_limits<qreal>::lowest();
    
    for (Element *element : m_selectedElements) {
        if (!element || !element->isVisual()) continue;
        
        CanvasElement* canvasElement = qobject_cast<CanvasElement*>(element);
        if (canvasElement) {
            minX = qMin(minX, canvasElement->x());
            minY = qMin(minY, canvasElement->y());
            maxX = qMax(maxX, canvasElement->x() + canvasElement->width());
            maxY = qMax(maxY, canvasElement->y() + canvasElement->height());
        }
    }
    
    m_boundingX = minX;
    m_boundingY = minY;
    m_boundingWidth = maxX - minX;
    m_boundingHeight = maxY - minY;
}

void SelectionManager::onElementGeometryChanged()
{
    // When selected elements move/resize, we need to recalculate
    recalculateBoundingBox();
    emit selectionChanged();
}

void SelectionManager::checkComponentVariantMembership()
{
    // Get the Project from parent
    Project* project = qobject_cast<Project*>(parent());
    if (!project) return;
    
    ElementModel* elementModel = project->elementModel();
    if (!elementModel) return;
    
    // Check each selected element
    for (Element* selectedElement : m_selectedElements) {
        if (!selectedElement) continue;
        
        // Get all elements from the model
        QList<Element*> allElements = elementModel->getAllElements();
        
        // Check each component in the model
        for (Element* element : allElements) {
            Component* component = qobject_cast<Component*>(element);
            if (component) {
                // Check if the selected element is in this component's variants
                QList<Element*> variants = component->variants();
                if (variants.contains(selectedElement)) {
                    break;
                }
            }
        }
        
    }
}