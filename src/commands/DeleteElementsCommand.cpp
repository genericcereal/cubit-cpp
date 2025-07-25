#include "DeleteElementsCommand.h"
#include "../Element.h"
#include "../ElementModel.h"
#include "../SelectionManager.h"
#include "../Project.h"
#include "../Component.h"
#include "../PlatformConfig.h"
#include <QDebug>
#include <vector>

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

    qDebug() << "DeleteElementsCommand::execute() - Deleting" << m_deletedElements.size() << "elements";
    for (const ElementInfo& info : m_deletedElements) {
        qDebug() << "  - Deleting element:" << info.element->getId() << "type:" << info.element->metaObject()->className();
    }

    // Get the Project from the ElementModel's parent
    Project* project = qobject_cast<Project*>(m_elementModel->parent());
    if (!project) {
        qWarning() << "DeleteElementsCommand: ElementModel has no Project parent";
    }

    // Check if we're in variant mode or globalElements mode
    bool isVariantMode = project && project->viewMode() == "variant";
    bool isGlobalElementsMode = project && project->viewMode() == "globalElements";
    Component* editingComponent = nullptr;
    PlatformConfig* editingPlatform = nullptr;
    
    if (isVariantMode && project) {
        QObject* editingElement = project->editingElement();
        editingComponent = qobject_cast<Component*>(editingElement);
    } else if (isGlobalElementsMode && project) {
        QObject* editingElement = project->editingElement();
        editingPlatform = qobject_cast<PlatformConfig*>(editingElement);
    }

    // Clear selection first
    if (m_selectionManager) {
        m_selectionManager->clearSelection();
    }

    // Remove elements from model
    for (const ElementInfo& info : m_deletedElements) {
        m_elementModel->removeElement(info.element->getId());
        
        // Also remove from variant array if in variant mode
        if (isVariantMode && editingComponent) {
            editingComponent->removeVariant(info.element);
        }
        // Also remove from globalElements if in globalElements mode
        else if (isGlobalElementsMode && editingPlatform) {
            ElementModel* globalElements = editingPlatform->globalElements();
            if (globalElements) {
                globalElements->removeElement(info.element->getId());
            }
        }
    }

    // Also remove any children
    for (const ElementInfo& info : m_deletedChildren) {
        m_elementModel->removeElement(info.element->getId());
        
        if (isVariantMode && editingComponent) {
            editingComponent->removeVariant(info.element);
        } else if (isGlobalElementsMode && editingPlatform) {
            ElementModel* globalElements = editingPlatform->globalElements();
            if (globalElements) {
                globalElements->removeElement(info.element->getId());
            }
        }
    }

}

void DeleteElementsCommand::undo()
{
    if (!m_elementModel) return;

    // Get the Project from the ElementModel's parent
    Project* project = qobject_cast<Project*>(m_elementModel->parent());
    
    // Check if we're in variant mode or globalElements mode
    bool isVariantMode = project && project->viewMode() == "variant";
    bool isGlobalElementsMode = project && project->viewMode() == "globalElements";
    Component* editingComponent = nullptr;
    PlatformConfig* editingPlatform = nullptr;
    
    if (isVariantMode && project) {
        QObject* editingElement = project->editingElement();
        editingComponent = qobject_cast<Component*>(editingElement);
    } else if (isGlobalElementsMode && project) {
        QObject* editingElement = project->editingElement();
        editingPlatform = qobject_cast<PlatformConfig*>(editingElement);
    }

    // Restore elements in reverse order to maintain proper indices
    for (int i = m_deletedElements.size() - 1; i >= 0; --i) {
        const ElementInfo& info = m_deletedElements[i];
        m_elementModel->addElement(info.element);
        
        // Also restore to variant array if in variant mode
        if (isVariantMode && editingComponent) {
            editingComponent->addVariant(info.element);
        }
        // Also restore to globalElements if in globalElements mode
        else if (isGlobalElementsMode && editingPlatform) {
            ElementModel* globalElements = editingPlatform->globalElements();
            if (globalElements) {
                globalElements->addElement(info.element);
            }
        }
    }

    // Restore children
    for (int i = m_deletedChildren.size() - 1; i >= 0; --i) {
        const ElementInfo& info = m_deletedChildren[i];
        m_elementModel->addElement(info.element);
        
        if (isVariantMode && editingComponent) {
            editingComponent->addVariant(info.element);
        } else if (isGlobalElementsMode && editingPlatform) {
            ElementModel* globalElements = editingPlatform->globalElements();
            if (globalElements) {
                globalElements->addElement(info.element);
            }
        }
    }

    // Restore selection
    if (m_selectionManager && !m_deletedElements.isEmpty()) {
        std::vector<Element*> elementsToSelect;
        elementsToSelect.reserve(m_deletedElements.size());
        for (const ElementInfo& info : m_deletedElements) {
            elementsToSelect.push_back(info.element);
        }
        m_selectionManager->selectAll(elementsToSelect);
    }

}