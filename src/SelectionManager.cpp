#include "SelectionManager.h"
#include "Element.h"
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

void SelectionManager::selectElement(Element *element)
{
    if (!element || isSelected(element)) return;
    
    m_selectedElements.insert(element);  // O(1) with QSet
    updateElementSelection(element, true);
    expandBoundingBox(element);  // Incremental update
    emit elementSelected(element);
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
    m_boundingX = element->x();
    m_boundingY = element->y();
    m_boundingWidth = element->width();
    m_boundingHeight = element->height();
    
    emit elementSelected(element);
    emit selectionChanged();
}

void SelectionManager::selectAll(const QList<Element*> &elements)
{
    clearSelection();
    
    if (elements.isEmpty()) return;
    
    // Initialize bounding box with first valid element
    bool firstElement = true;
    
    for (Element *element : elements) {
        if (element) {
            m_selectedElements.insert(element);  // O(1) with QSet
            updateElementSelection(element, true);
            
            if (firstElement) {
                // Initialize bounding box
                m_boundingX = element->x();
                m_boundingY = element->y();
                m_boundingWidth = element->width();
                m_boundingHeight = element->height();
                firstElement = false;
            } else {
                // Expand bounding box incrementally
                expandBoundingBox(element);
            }
            
            emit elementSelected(element);
        }
    }
    
    if (!m_selectedElements.isEmpty()) {
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
    
    emit selectionChanged();
}

void SelectionManager::updateElementSelection(Element *element, bool selected)
{
    if (element) {
        element->setSelected(selected);
        
        if (selected) {
            // Connect to geometry changes when selected - single connection with UniqueConnection flag
            connect(element, &Element::geometryChanged, 
                    this, &SelectionManager::onElementGeometryChanged,
                    Qt::UniqueConnection);
        } else {
            // Disconnect when deselected
            disconnect(element, &Element::geometryChanged, 
                       this, &SelectionManager::onElementGeometryChanged);
        }
    }
}

void SelectionManager::expandBoundingBox(Element *element)
{
    if (!element) return;
    
    if (m_selectedElements.size() == 1) {
        // First element, initialize bounding box
        m_boundingX = element->x();
        m_boundingY = element->y();
        m_boundingWidth = element->width();
        m_boundingHeight = element->height();
    } else {
        // Expand existing bounding box
        qreal elementRight = element->x() + element->width();
        qreal elementBottom = element->y() + element->height();
        qreal currentRight = m_boundingX + m_boundingWidth;
        qreal currentBottom = m_boundingY + m_boundingHeight;
        
        qreal newLeft = qMin(m_boundingX, element->x());
        qreal newTop = qMin(m_boundingY, element->y());
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
        if (!element) continue;
        
        minX = qMin(minX, element->x());
        minY = qMin(minY, element->y());
        maxX = qMax(maxX, element->x() + element->width());
        maxY = qMax(maxY, element->y() + element->height());
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