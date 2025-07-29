#include "ChangeParentCommand.h"
#include "../Element.h"
#include "../CanvasElement.h"
#include "../DesignElement.h"
#include "../Frame.h"
#include "../Application.h"
#include "../Project.h"
#include "../ProjectApiClient.h"
#include "../ElementModel.h"
#include "../PlatformConfig.h"
#include <QDebug>

ChangeParentCommand::ChangeParentCommand(Element* element, const QString& newParentId, QObject *parent)
    : Command(parent)
    , m_element(element)
    , m_designElement(nullptr)
    , m_newParentId(newParentId)
    , m_usePositionBasedParenting(false)
    , m_newParent(nullptr)
{
    if (m_element) {
        m_oldParentId = m_element->getParentElementId();
        
        // Check if element is currently in globalElements
        ElementModel* currentModel = qobject_cast<ElementModel*>(m_element->parent());
        if (currentModel) {
            // Check if this is a globalElements model by checking its parent
            PlatformConfig* platform = qobject_cast<PlatformConfig*>(currentModel->parent());
            if (platform) {
                m_wasInGlobalElements = true;
                m_originalPlatformName = platform->name();
            }
        }
        
        QString description;
        if (newParentId.isEmpty()) {
            description = QString("Unparent %1").arg(m_element->getName());
        } else {
            description = QString("Parent %1").arg(m_element->getName());
        }
        setDescription(description);
    }
}

ChangeParentCommand::ChangeParentCommand(DesignElement* element, CanvasElement* newParent, const QPointF& relativePosition, QObject *parent)
    : Command(parent)
    , m_element(element)
    , m_designElement(element)
    , m_usePositionBasedParenting(true)
    , m_newParent(newParent)
    , m_relativePosition(relativePosition)
{
    if (m_element) {
        m_oldParentId = m_element->getParentElementId();
        m_newParentId = newParent ? newParent->getId() : QString();
        
        // Store old absolute position
        if (m_designElement) {
            m_oldPosition = QPointF(m_designElement->x(), m_designElement->y());
        }
        
        // Check if element is currently in globalElements
        ElementModel* currentModel = qobject_cast<ElementModel*>(m_element->parent());
        if (currentModel) {
            // Check if this is a globalElements model by checking its parent
            PlatformConfig* platform = qobject_cast<PlatformConfig*>(currentModel->parent());
            if (platform) {
                m_wasInGlobalElements = true;
                m_originalPlatformName = platform->name();
            }
        }
        
        QString description;
        if (m_newParentId.isEmpty()) {
            description = QString("Unparent %1").arg(m_element->getName());
        } else {
            description = QString("Parent %1").arg(m_element->getName());
        }
        setDescription(description);
    }
}

ChangeParentCommand::~ChangeParentCommand()
{
}

void ChangeParentCommand::execute()
{
    if (!m_element) return;
    
    // Check if we're parenting to an appContainer
    bool isParentingToAppContainer = false;
    Frame* targetFrame = nullptr;
    ElementModel* currentModel = qobject_cast<ElementModel*>(m_element->parent());
    
    if (!m_newParentId.isEmpty() && currentModel) {
        Element* parentElement = currentModel->getElementById(m_newParentId);
        targetFrame = qobject_cast<Frame*>(parentElement);
        
        // Check if it's not found in current model - might be in globalElements
        if (!targetFrame) {
            // Try to find in platform globalElements
            Project* project = qobject_cast<Project*>(currentModel->parent());
            if (project) {
                QList<PlatformConfig*> platforms = project->getAllPlatforms();
                for (PlatformConfig* platform : platforms) {
                    if (platform && platform->globalElements()) {
                        parentElement = platform->globalElements()->getElementById(m_newParentId);
                        targetFrame = qobject_cast<Frame*>(parentElement);
                        if (targetFrame) break;
                    }
                }
            }
        }
        
        isParentingToAppContainer = targetFrame && targetFrame->role() == Frame::appContainer;
    }
    
    // Check if we're unparenting from an appContainer  
    bool isUnparentingFromAppContainer = false;
    if (m_newParentId.isEmpty() && !m_oldParentId.isEmpty()) {
        // Check if the old parent was an appContainer
        Element* oldParentElement = nullptr;
        
        // First check in current model
        oldParentElement = currentModel->getElementById(m_oldParentId);
        
        // If not found, check in globalElements
        if (!oldParentElement) {
            Project* project = qobject_cast<Project*>(currentModel->parent());
            if (!project) {
                // Current model might be globalElements
                PlatformConfig* platform = qobject_cast<PlatformConfig*>(currentModel->parent());
                if (platform) {
                    project = qobject_cast<Project*>(platform->parent());
                }
            }
            
            if (project) {
                QList<PlatformConfig*> platforms = project->getAllPlatforms();
                for (PlatformConfig* platform : platforms) {
                    if (platform && platform->globalElements()) {
                        oldParentElement = platform->globalElements()->getElementById(m_oldParentId);
                        if (oldParentElement) break;
                    }
                }
            }
        }
        
        Frame* oldFrame = qobject_cast<Frame*>(oldParentElement);
        isUnparentingFromAppContainer = oldFrame && oldFrame->role() == Frame::appContainer;
    }
    
    // If parenting to appContainer, we need to move the element to globalElements
    if (isParentingToAppContainer && targetFrame) {
        Project* project = qobject_cast<Project*>(currentModel->parent());
        if (project) {
            // Find the platform that owns this appContainer
            QString platformName = targetFrame->platform();
            PlatformConfig* platform = project->getPlatform(platformName);
            
            if (platform && platform->globalElements()) {
                qDebug() << "Moving element" << m_element->getId() << "to globalElements of platform" << platformName;
                
                // Remove from current model WITHOUT deleting the element
                currentModel->removeElementWithoutDelete(m_element);
                
                // Add to globalElements
                platform->globalElements()->addElement(m_element);
                
                // Now set the parent
                if (m_usePositionBasedParenting && m_designElement) {
                    m_designElement->setParentElement(m_newParent, m_relativePosition.x(), m_relativePosition.y());
                } else {
                    m_element->setParentElementId(m_newParentId);
                }
            }
        }
    } else if (isUnparentingFromAppContainer) {
        // Move element from globalElements back to main model
        Project* project = nullptr;
        
        // Find project
        if (currentModel->parent()) {
            project = qobject_cast<Project*>(currentModel->parent());
            if (!project) {
                PlatformConfig* platform = qobject_cast<PlatformConfig*>(currentModel->parent());
                if (platform) {
                    project = qobject_cast<Project*>(platform->parent());
                }
            }
        }
        
        if (project && project->elementModel()) {
            qDebug() << "Moving element" << m_element->getId() << "from globalElements to main model";
            
            // Remove from current model (globalElements) WITHOUT deleting the element
            currentModel->removeElementWithoutDelete(m_element);
            
            // Add to main model
            project->elementModel()->addElement(m_element);
            
            // Now unparent
            if (m_usePositionBasedParenting && m_designElement) {
                QPointF absolutePos(m_designElement->x(), m_designElement->y());
                m_designElement->setParentElement(nullptr);
                m_designElement->setX(absolutePos.x());
                m_designElement->setY(absolutePos.y());
            } else {
                m_element->setParentElementId(m_newParentId);
            }
        }
    } else {
        // Normal parenting logic
        if (m_usePositionBasedParenting && m_designElement) {
            if (m_newParent) {
                // Use the position-based setParentElement method
                m_designElement->setParentElement(m_newParent, m_relativePosition.x(), m_relativePosition.y());
            } else {
                // Unparent - first store current absolute position
                QPointF absolutePos(m_designElement->x(), m_designElement->y());
                m_designElement->setParentElement(nullptr);
                // Restore absolute position after unparenting
                m_designElement->setX(absolutePos.x());
                m_designElement->setY(absolutePos.y());
            }
        } else {
            // Handle regular elements or simple parentId changes
            m_element->setParentElementId(m_newParentId);
        }
    }
    
    // Log to console
    if (m_newParentId.isEmpty()) {
        qDebug() << QString("Unparented element %1").arg(m_element->getName());
    } else {
        qDebug() << QString("Parented element %1 to %2").arg(m_element->getName()).arg(m_newParentId);
    }
    
    // Sync with API
    syncWithAPI();
}

void ChangeParentCommand::undo()
{
    if (!m_element) return;
    
    // Get current model
    ElementModel* currentModel = qobject_cast<ElementModel*>(m_element->parent());
    if (!currentModel) return;
    
    // Find the project
    Project* project = nullptr;
    QObject* p = currentModel->parent();
    while (p && !project) {
        project = qobject_cast<Project*>(p);
        if (!project) {
            // Check if parent is PlatformConfig, then get project from there
            PlatformConfig* platform = qobject_cast<PlatformConfig*>(p);
            if (platform) {
                p = platform->parent();
                continue;
            }
        }
        p = p->parent();
    }
    
    if (!project) return;
    
    // Check if we need to move the element back to its original model
    bool needsModelTransfer = false;
    ElementModel* targetModel = nullptr;
    
    if (m_wasInGlobalElements && !m_originalPlatformName.isEmpty()) {
        // Element was in globalElements, needs to go back
        PlatformConfig* platform = project->getPlatform(m_originalPlatformName);
        if (platform && platform->globalElements()) {
            targetModel = platform->globalElements();
            needsModelTransfer = (currentModel != targetModel);
        }
    } else {
        // Element was in main model
        targetModel = project->elementModel();
        needsModelTransfer = (currentModel != targetModel);
    }
    
    // Transfer element if needed
    if (needsModelTransfer && targetModel) {
        currentModel->removeElementWithoutDelete(m_element);
        targetModel->addElement(m_element);
    }
    
    // Now restore the parent relationship
    if (m_usePositionBasedParenting && m_designElement) {
        // Find the old parent element if it exists
        if (!m_oldParentId.isEmpty()) {
            Element* oldParentElement = nullptr;
            
            // Search in target model first
            if (targetModel) {
                oldParentElement = targetModel->getElementById(m_oldParentId);
            }
            
            // If not found, search in all models
            if (!oldParentElement) {
                oldParentElement = project->elementModel()->getElementById(m_oldParentId);
                if (!oldParentElement) {
                    // Check globalElements
                    QList<PlatformConfig*> platforms = project->getAllPlatforms();
                    for (PlatformConfig* platform : platforms) {
                        if (platform && platform->globalElements()) {
                            oldParentElement = platform->globalElements()->getElementById(m_oldParentId);
                            if (oldParentElement) break;
                        }
                    }
                }
            }
            
            CanvasElement* oldParent = qobject_cast<CanvasElement*>(oldParentElement);
            if (oldParent) {
                // Calculate relative position to old parent
                QPointF relPos(m_oldPosition.x() - oldParent->x(), m_oldPosition.y() - oldParent->y());
                m_designElement->setParentElement(oldParent, relPos.x(), relPos.y());
            } else {
                // Fallback to simple parentId
                m_element->setParentElementId(m_oldParentId);
            }
        } else {
            // Was unparented - restore to unparented state at old position
            m_designElement->setParentElement(nullptr);
            m_designElement->setX(m_oldPosition.x());
            m_designElement->setY(m_oldPosition.y());
        }
    } else {
        // Handle regular elements
        m_element->setParentElementId(m_oldParentId);
    }
    
    qDebug() << QString("Undid parent change for element %1").arg(m_element->getName());
}

void ChangeParentCommand::syncWithAPI()
{
    if (!m_element) {
        return;
    }

    // Navigate up to find the project (through ElementModel)
    ElementModel* model = nullptr;
    QObject* parent = m_element->parent();
    while (parent) {
        model = qobject_cast<ElementModel*>(parent);
        if (model) break;
        parent = parent->parent();
    }

    if (!model) {
        return;
    }

    Project* project = qobject_cast<Project*>(model->parent());
    if (!project) {
        return;
    }

    // Get the Application instance and its API client
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
    
    // Sync with API
    apiClient->syncUpdateElement(apiProjectId, m_element->getId());
    
    qDebug() << "ChangeParentCommand: Syncing parent change with API for project" << apiProjectId 
             << "element:" << m_element->getId() << "new parent:" << m_newParentId;
}