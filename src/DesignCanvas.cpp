#include "DesignCanvas.h"
#include "Element.h"
#include "DesignElement.h"
#include "Frame.h"
#include "Shape.h"
#include "SelectionManager.h"
#include "ElementModel.h"
#include <QDebug>

DesignCanvas::DesignCanvas(ElementModel& model,
                           SelectionManager& sel,
                           QObject *parent)
    : CanvasController(model, sel, parent)
{
    // Connect to selection changes to clear hover when needed
    connect(&sel, &SelectionManager::selectionChanged,
            this, &DesignCanvas::onSelectionChanged,
            Qt::UniqueConnection);
            
    // Connect to mode changes to clear hover
    connect(this, &CanvasController::modeChanged,
            this, &DesignCanvas::onModeChanged,
            Qt::UniqueConnection);
}

void DesignCanvas::setHoveredElement(QObject* element)
{
    if (m_hoveredElement != element) {
        // qDebug() << "DesignCanvas::setHoveredElement: changing from" << m_hoveredElement << "to" << element;
        m_hoveredElement = element;
        emit hoveredElementChanged();
    }
}

void DesignCanvas::setIsAnimating(bool animating)
{
    if (m_isAnimating != animating) {
        m_isAnimating = animating;
        emit isAnimatingChanged();
    }
}

void DesignCanvas::setIsDesignControlsResizingDisabled(bool disabled)
{
    if (m_isDesignControlsResizingDisabled != disabled) {
        m_isDesignControlsResizingDisabled = disabled;
        emit isDesignControlsResizingDisabledChanged();
    }
}

void DesignCanvas::setIsDesignControlsMovementDisabled(bool disabled)
{
    if (m_isDesignControlsMovementDisabled != disabled) {
        m_isDesignControlsMovementDisabled = disabled;
        emit isDesignControlsMovementDisabledChanged();
    }
}

void DesignCanvas::setIsEditingShape(bool editing)
{
    if (m_isEditingShape != editing) {
        m_isEditingShape = editing;
        emit isEditingShapeChanged();
    }
}

void DesignCanvas::updateHover(qreal x, qreal y)
{
    // Update hover in select mode
    if (mode() == Mode::Select) {
        Element* element = hitTestForHover(x, y);
        setHoveredElement(element);
    } else {
        // Clear hover in creation modes
        setHoveredElement(nullptr);
    }
}

bool DesignCanvas::isDescendantOf(QObject* elementA, QObject* elementB) const
{
    if (!elementA || !elementB) return false;
    
    Element* elemA = qobject_cast<Element*>(elementA);
    Element* elemB = qobject_cast<Element*>(elementB);
    if (!elemA || !elemB) return false;
    
    Element* current = elemA;
    int maxDepth = 100; // Prevent infinite loops
    int depth = 0;
    
    while (current && !current->getParentElementId().isEmpty() && depth < maxDepth) {
        if (current->getParentElementId() == elemB->getId()) {
            return true;
        }
        // Find the parent element
        current = m_elementModel.getElementById(current->getParentElementId());
        depth++;
    }
    
    return false;
}

bool DesignCanvas::isChildOfSelected(QObject* element) const
{
    if (!element) return false;
    
    const QList<Element*>& selectedElements = m_selectionManager.selectedElements();
    for (Element* selected : selectedElements) {
        if (isDescendantOf(element, selected)) {
            return true;
        }
    }
    return false;
}

void DesignCanvas::updateParentingDuringDrag()
{
    if (m_selectionManager.selectionCount() == 0) {
        return;
    }
    
    const QList<Element*>& selectedElements = m_selectionManager.selectedElements();
    
    // If no element is hovered, unparent the selected elements
    if (!m_hoveredElement) {
        for (Element* element : selectedElements) {
            if (!element) continue;
            
            // Guard: Don't change parentId of children of selected elements
            if (isChildOfSelected(element)) {
                continue;
            }
            
            // Guard: ComponentVariants shouldn't have parents
            if (element->getTypeName() == "FrameComponentVariant") {
                continue;
            }
            
            // Unparent the element by setting parentId to empty string
            if (!element->getParentElementId().isEmpty()) {
                element->setParentElementId("");
            }
        }
        return;
    }
    
    Element* hovered = qobject_cast<Element*>(m_hoveredElement);
    if (!hovered) {
        return;
    }
    
    for (Element* element : selectedElements) {
        if (!element) continue;
        
        // Guard: Don't change parentId of children of selected elements
        if (isChildOfSelected(element)) {
            continue;
        }
        
        // Guard: ComponentVariants shouldn't have parents
        if (element->getTypeName() == "FrameComponentVariant") {
            continue;
        }
        
        // Guard: Don't set an element as its own parent
        if (hovered->getId() == element->getId()) {
            continue;
        }
        
        // Guard: Don't create circular dependencies
        if (isDescendantOf(hovered, element)) {
            continue;
        }
        
        // Guard: Don't parent to children of selected elements
        if (isChildOfSelected(hovered)) {
            continue;
        }
        
        // Guard: Check if the hovered element accepts children
        Frame* frameHovered = qobject_cast<Frame*>(hovered);
        if (frameHovered && !frameHovered->acceptsChildren()) {
            continue;
        }
        
        // Guard: Don't parent relatively positioned elements to other relatively positioned elements
        Frame* frameElement = qobject_cast<Frame*>(element);
        if (frameElement && frameHovered) {
            if (frameElement->position() == Frame::Relative && 
                frameHovered->position() == Frame::Relative) {
                continue;
            }
        }
        
        // Update parentId if it's different
        if (element->getParentElementId() != hovered->getId()) {
            element->setParentElementId(hovered->getId());
        }
    }
}

void DesignCanvas::onSelectionChanged()
{
    clearHoverIfSelected();
    
    // Don't automatically enter shape editing mode on selection
    // Shape editing mode should be activated explicitly (e.g., double-click)
    // For now, always disable shape editing when selection changes
    setIsEditingShape(false);
}

void DesignCanvas::onModeChanged()
{
    setHoveredElement(nullptr);
}

void DesignCanvas::clearHoverIfSelected()
{
    if (m_hoveredElement) {
        Element* elem = qobject_cast<Element*>(m_hoveredElement);
        if (elem && m_selectionManager.isSelected(elem)) {
            setHoveredElement(nullptr);
        }
    }
}