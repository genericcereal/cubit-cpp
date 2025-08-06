#include "DeleteElementsCommand.h"
#include "../Element.h"
#include "../ElementModel.h"
#include "../SelectionManager.h"
#include "../Project.h"
#include "../PlatformConfig.h"
#include "../Scripts.h"
#include "../Node.h"
#include "../Edge.h"
#include "../CanvasElement.h"
#include "../DesignElement.h"
#include "../Variable.h"
#include "../Application.h"
#include "../ProjectApiClient.h"
#include <QDebug>
#include <QCoreApplication>
#include <QJsonArray>
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
            info.parent = nullptr;
            info.index = m_elementModel->getAllElements().indexOf(element);
            m_deletedElements.append(info);
            
            // Find all child elements recursively
            QList<Element*> allElements = m_elementModel->getAllElements();
            findChildElements(element->getId(), allElements);
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

    // Store element IDs before deletion for API sync
    m_deletedElementIds.clear();
    for (const ElementInfo& info : m_deletedElements) {
        m_deletedElementIds.append(info.element->getId());
    }
    // Also add child element IDs
    for (const ElementInfo& info : m_deletedChildren) {
        m_deletedElementIds.append(info.element->getId());
    }

    // Clear selection first
    if (m_selectionManager) {
        m_selectionManager->clearSelection();
    }
    
    // Add associated Variable elements to the deletion list
    Project* project = qobject_cast<Project*>(m_elementModel->parent());
    if (project) {
        QList<ElementInfo> variablesToDelete;
        for (const ElementInfo& info : m_deletedElements) {
            Element* element = info.element;
            CanvasElement* canvasElement = qobject_cast<CanvasElement*>(element);
            if (canvasElement && canvasElement->isDesignElement()) {
                    // Find the Variable element with linkedElementId matching our element
                    QList<Element*> allElements = m_elementModel->getAllElements();
                    for (Element* elem : allElements) {
                        if (Variable* var = qobject_cast<Variable*>(elem)) {
                            if (var->linkedElementId() == element->getId()) {
                                ElementInfo varInfo;
                                varInfo.element = var;
                                varInfo.parent = nullptr;
                                varInfo.index = m_elementModel->getAllElements().indexOf(var);
                                variablesToDelete.append(varInfo);
                                break;
                            }
                        }
                    }
            }
        }
        // Add variables to the deletion list
        m_deletedElements.append(variablesToDelete);
    }

    // Set flag to prevent Project from trying to remove nodes/edges again during model removal
    bool isScriptMode = project && project->viewMode() == "script";
    if (isScriptMode) {
        m_elementModel->setProperty("_deleteCommandHandling", true);
        
        // Remove nodes/edges from Scripts BEFORE removing from model to avoid use-after-free
        Scripts* scripts = project->activeScripts();
        if (scripts) {
            // First collect all edges that will be deleted due to node deletion
            QList<Edge*> edgesToDelete;
            for (const ElementInfo& info : m_deletedElements) {
                if (Node* node = qobject_cast<Node*>(info.element)) {
                    // Get edges connected to this node
                    QString nodeId = node->getId();
                    QList<Edge*> connectedEdges = scripts->getEdgesForNode(nodeId);
                    for (Edge* edge : connectedEdges) {
                        if (edge && !edgesToDelete.contains(edge)) {
                            edgesToDelete.append(edge);
                            //          << "connected to node" << nodeId;
                        }
                    }
                }
            }
            
            // Add connected edges to the list of elements to delete from ElementModel
            for (Edge* edge : edgesToDelete) {
                // Check if edge is not already in the deletion list
                bool alreadyDeleting = false;
                for (const ElementInfo& info : m_deletedElements) {
                    if (info.element == edge) {
                        alreadyDeleting = true;
                        break;
                    }
                }
                
                if (!alreadyDeleting) {
                    ElementInfo edgeInfo;
                    edgeInfo.element = edge;
                    edgeInfo.parent = nullptr;
                    edgeInfo.index = m_elementModel->getAllElements().indexOf(edge);
                    m_deletedElements.append(edgeInfo);
                }
            }
            
            // Remove nodes from Scripts (this will also remove connected edges)
            for (const ElementInfo& info : m_deletedElements) {
                if (Node* node = qobject_cast<Node*>(info.element)) {
                    scripts->removeNode(node);
                } else if (Edge* edge = qobject_cast<Edge*>(info.element)) {
                    // Only remove edges that aren't already removed by removeNode
                    if (scripts->getEdge(edge->getId())) {
                        scripts->removeEdge(edge);
                    }
                }
            }
            
            // Don't process events during deletion - this can cause crashes
            // QML will be updated automatically when the model changes
        }
    }

    // Remove elements from model
    for (const ElementInfo& info : m_deletedElements) {
        m_elementModel->removeElement(info.element->getId());
        
        // Also remove from variant array if in variant mode
        // if (isVariantMode && editingComponent) {
        //     editingComponent->removeVariant(info.element);
        // }
    }

    // Also remove any children
    for (const ElementInfo& info : m_deletedChildren) {
        m_elementModel->removeElement(info.element->getId());
        
    }

    // Clear the flag after all removals are done
    if (isScriptMode) {
        m_elementModel->setProperty("_deleteCommandHandling", false);
    }
    
    // Sync with API after all deletions are complete
    syncWithAPI();
}

void DeleteElementsCommand::undo()
{
    if (!m_elementModel) return;

    // Restore elements in reverse order to maintain proper indices
    for (int i = m_deletedElements.size() - 1; i >= 0; --i) {
        const ElementInfo& info = m_deletedElements[i];
        m_elementModel->addElement(info.element);
        
        // Also restore to variant array if in variant mode
        // if (isVariantMode && editingComponent) {
        //     editingComponent->addVariant(info.element);
        // }
    }

    // Restore children
    for (int i = m_deletedChildren.size() - 1; i >= 0; --i) {
        const ElementInfo& info = m_deletedChildren[i];
        m_elementModel->addElement(info.element);
        
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

void DeleteElementsCommand::syncWithAPI()
{
    // Get the project from the element model
    if (!m_elementModel) return;
    
    Project* project = qobject_cast<Project*>(m_elementModel->parent());
    if (!project) {
        return;
    }
    
    // Get the Application and API client
    Application* app = Application::instance();
    if (!app) {
        return;
    }
    
    ProjectApiClient* apiClient = app->projectApiClient();
    if (!apiClient) {
        return;
    }
    
    // Get the project's API ID
    QString apiProjectId = project->id();
    
    // Use pre-stored element IDs (elements might already be deleted)
    QJsonArray elementIds;
    for (const QString& elementId : m_deletedElementIds) {
        elementIds.append(elementId);
    }
    
    // Sync with API
    apiClient->syncDeleteElements(apiProjectId, elementIds);
}

void DeleteElementsCommand::findChildElements(const QString& parentId, const QList<Element*>& allElements)
{
    for (Element* element : allElements) {
        if (element && element->getParentElementId() == parentId) {
            // Check if this element is not already in the deletion list
            bool alreadyDeleting = false;
            for (const ElementInfo& info : m_deletedElements) {
                if (info.element == element) {
                    alreadyDeleting = true;
                    break;
                }
            }
            for (const ElementInfo& info : m_deletedChildren) {
                if (info.element == element) {
                    alreadyDeleting = true;
                    break;
                }
            }
            
            if (!alreadyDeleting) {
                ElementInfo childInfo;
                childInfo.element = element;
                childInfo.parent = nullptr;
                childInfo.index = m_elementModel->getAllElements().indexOf(element);
                m_deletedChildren.append(childInfo);
                
                // Don't store IDs here - will be done in execute()
                
                // Recursively find children of this child
                findChildElements(element->getId(), allElements);
            }
        }
    }
}