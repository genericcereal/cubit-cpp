#include "Component.h"

ComponentElement::ComponentElement(const QString &id, QObject *parent)
    : Element(ComponentType, id, parent)
{
    setName("Component");
    m_showInElementList = true;  // Components should appear in the element list
}

ComponentElement::~ComponentElement()
{
}

void ComponentElement::addElement(Element* element)
{
    if (!element) {
        qWarning() << "ComponentElement::addElement called with null element";
        return;
    }
    
    if (m_elements.contains(element)) {
        qWarning() << "ComponentElement::addElement - element" << element->getId() << "already in component";
        return;
    }
    
    m_elements.append(element);
    emit elementAdded(element);
    emit elementsChanged();
}

void ComponentElement::removeElement(Element* element)
{
    if (!element || !m_elements.contains(element)) {
        return;
    }
    
    m_elements.removeOne(element);
    emit elementRemoved(element);
    emit elementsChanged();
}