#include "DeleteElementsCommand.h"
#include "../Element.h"
#include "../ElementModel.h"
#include "../SelectionManager.h"
#include <QDebug>

DeleteElementsCommand::DeleteElementsCommand(ElementModel* model, SelectionManager* selectionManager,
                                             const QList<Element*>& elements, QObject *parent)
    : Command(parent)
    , m_elementModel(model)
    , m_selectionManager(selectionManager)
{
    // Store information about elements to delete
    for (Element* element : elements) {
        if (element) {
            ElementInfo info;
            info.element = element;
            info.parent = nullptr; // TODO: Handle parent-child relationships when implemented
            info.index = m_elementModel->getAllElements().indexOf(element);
            m_deletedElements.append(info);
        }
    }

    setDescription(QString("Delete %1 element%2")
                   .arg(elements.size())
                   .arg(elements.size() == 1 ? "" : "s"));
}

DeleteElementsCommand::~DeleteElementsCommand()
{
    // Elements are owned by the model or by this command when undone
    // Don't delete them here as they might be re-added on redo
}

void DeleteElementsCommand::execute()
{
    if (!m_elementModel) return;

    // Clear selection first
    if (m_selectionManager) {
        m_selectionManager->clearSelection();
    }

    // Remove elements from model
    for (const ElementInfo& info : m_deletedElements) {
        m_elementModel->removeElement(info.element->getId());
    }

    // Also remove any children
    for (const ElementInfo& info : m_deletedChildren) {
        m_elementModel->removeElement(info.element->getId());
    }

}

void DeleteElementsCommand::undo()
{
    if (!m_elementModel) return;

    // Restore elements in reverse order to maintain proper indices
    for (int i = m_deletedElements.size() - 1; i >= 0; --i) {
        const ElementInfo& info = m_deletedElements[i];
        m_elementModel->addElement(info.element);
    }

    // Restore children
    for (int i = m_deletedChildren.size() - 1; i >= 0; --i) {
        const ElementInfo& info = m_deletedChildren[i];
        m_elementModel->addElement(info.element);
    }

    // Restore selection
    if (m_selectionManager && !m_deletedElements.isEmpty()) {
        QList<Element*> elementsToSelect;
        for (const ElementInfo& info : m_deletedElements) {
            elementsToSelect.append(info.element);
        }
        m_selectionManager->selectAll(elementsToSelect);
    }

}