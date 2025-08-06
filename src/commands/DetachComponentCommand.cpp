#include "DetachComponentCommand.h"
#include "../DesignElement.h"
#include "../SelectionManager.h"
#include <QDebug>

DetachComponentCommand::DetachComponentCommand(DesignElement* element, SelectionManager* selectionManager)
    : m_element(element)
    , m_selectionManager(selectionManager)
    , m_originalInstanceOf("")
    , m_originalName("")
    , m_newName("")
{
    if (m_element) {
        m_originalInstanceOf = m_element->instanceOf();
        m_originalName = m_element->getName();
        
        // Calculate the new name by replacing "Instance" with "Copy"
        m_newName = m_originalName;
        if (m_newName.contains("Instance")) {
            m_newName = m_newName.replace("Instance", "Copy");
        }
        
        setDescription(QString("Detach component %1").arg(m_originalName));
    }
}

void DetachComponentCommand::execute()
{
    if (!m_element) {
        qWarning() << "DetachComponentCommand::execute - Invalid element";
        return;
    }
    
    if (m_originalInstanceOf.isEmpty()) {
        qWarning() << "DetachComponentCommand::execute - Element is not a component instance";
        return;
    }
    
    // Update the name to replace "Instance" with "Copy"
    if (m_originalName != m_newName) {
        m_element->setName(m_newName);
    }
    
    // Clear the instanceOf property to detach from the source component
    // This will automatically disconnect from the source element (see DesignElement::setInstanceOf)
    m_element->setInstanceOf("");
    
    // Select the detached element
    if (m_selectionManager) {
        m_selectionManager->clearSelection();
        m_selectionManager->selectElement(m_element);
    }}

void DetachComponentCommand::undo()
{
    if (!m_element) {
        qWarning() << "DetachComponentCommand::undo - Invalid element";
        return;
    }
    
    // Restore the original instanceOf to re-attach to the source component
    m_element->setInstanceOf(m_originalInstanceOf);
    
    // Restore the original name
    m_element->setName(m_originalName);}