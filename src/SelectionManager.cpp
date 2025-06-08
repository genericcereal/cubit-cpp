#include "SelectionManager.h"
#include "Element.h"

SelectionManager::SelectionManager(QObject *parent)
    : QObject(parent)
{
}

bool SelectionManager::isSelected(Element *element) const
{
    return element && m_selectedElements.contains(element);
}

void SelectionManager::selectElement(Element *element)
{
    if (!element || isSelected(element)) return;
    
    m_selectedElements.append(element);
    updateElementSelection(element, true);
    emit elementSelected(element);
    emit selectionChanged();
}

void SelectionManager::deselectElement(Element *element)
{
    if (!element || !isSelected(element)) return;
    
    m_selectedElements.removeOne(element);
    updateElementSelection(element, false);
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
    selectElement(element);
}

void SelectionManager::selectAll(const QList<Element*> &elements)
{
    clearSelection();
    
    for (Element *element : elements) {
        if (element && !isSelected(element)) {
            m_selectedElements.append(element);
            updateElementSelection(element, true);
            emit elementSelected(element);
        }
    }
    
    if (!elements.isEmpty()) {
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
    emit selectionChanged();
}

void SelectionManager::updateElementSelection(Element *element, bool selected)
{
    if (element) {
        element->setSelected(selected);
    }
}