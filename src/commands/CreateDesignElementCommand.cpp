#include "CreateDesignElementCommand.h"
#include "DesignElement.h"
#include "Text.h"
#include "Shape.h"
#include "ElementModel.h"
#include "SelectionManager.h"
#include "Application.h"
#include "Project.h"
#include "Component.h"
#include "ElementTypeRegistry.h"
#include "ProjectApiClient.h"
#include <QDebug>

CreateDesignElementCommand::CreateDesignElementCommand(ElementModel* model, SelectionManager* selectionManager,
                                                     const QString& elementType, const QRectF& rect,
                                                     const QVariant& initialPayload, QObject *parent)
    : Command(parent)
    , m_elementModel(model)
    , m_selectionManager(selectionManager)
    , m_elementType(elementType)
    , m_rect(rect)
    , m_initialPayload(initialPayload)
    , m_element(nullptr)
{
    // Get display name from registry if available
    ElementTypeRegistry& registry = ElementTypeRegistry::instance();
    QString displayName = elementType;
    if (registry.hasType(elementType)) {
        displayName = registry.getTypeInfo(elementType).displayName;
    }
    
    setDescription(QString("Create %1 at (%2, %3)")
                   .arg(displayName)
                   .arg(rect.x())
                   .arg(rect.y()));
}

CreateDesignElementCommand::~CreateDesignElementCommand()
{
    // QPointer will automatically be null if the objects were deleted
    // Only delete elements if they exist and the model is still valid
    if (m_elementModel && m_element) {
        // Check if element is not in the model (undone state)
        auto allElements = m_elementModel->getAllElements();
        
        if (!allElements.contains(m_element)) {
            delete m_element;
        }
    }
}

void CreateDesignElementCommand::execute()
{
    if (!m_elementModel) return;

    // Get the Project from the ElementModel's parent
    Project* project = qobject_cast<Project*>(m_elementModel->parent());
    if (!project) {
        qWarning() << "CreateDesignElementCommand: ElementModel has no Project parent";
        return;
    }

    // Check if we're in variant mode
    bool isVariantMode = project->viewMode() == "variant";
    Component* editingComponent = nullptr;
    
    if (isVariantMode) {
        QObject* editingElement = project->editingElement();
        editingComponent = qobject_cast<Component*>(editingElement);
        if (!editingComponent) {
            qWarning() << "CreateDesignElementCommand: In variant mode but editing element is not a Component";
        }
    }

    // Create element on first execution
    if (!m_element) {
        m_elementId = m_elementModel->generateId();
        
        // Use ElementTypeRegistry to create the element
        ElementTypeRegistry& registry = ElementTypeRegistry::instance();
        m_element = registry.createElement(m_elementType, m_elementId);
        
        if (!m_element) {
            qWarning() << "Failed to create element of type:" << m_elementType;
            return;
        }
        
        // Set position and size
        m_element->setRect(m_rect);
        
        // Apply initial payload based on element type
        if (m_elementType == "text" && m_initialPayload.isValid() && m_initialPayload.canConvert<QString>()) {
            if (Text* textElement = qobject_cast<Text*>(m_element)) {
                textElement->setContent(m_initialPayload.toString());
            }
        } else if (m_elementType == "webtextinput" && m_initialPayload.isValid() && m_initialPayload.canConvert<QString>()) {
            m_element->setProperty("placeholder", m_initialPayload.toString());
        } else if (m_elementType == "shape" && m_initialPayload.isValid() && m_initialPayload.canConvert<int>()) {
            if (Shape* shapeElement = qobject_cast<Shape*>(m_element)) {
                shapeElement->setShapeType(static_cast<Shape::ShapeType>(m_initialPayload.toInt()));
            }
        }
    }

    // Add element to appropriate container
    if (isVariantMode && editingComponent) {
        // In variant mode, add to the Component's variants array
        editingComponent->addVariant(m_element);
        // Still add to model for visibility
        m_elementModel->addElement(m_element);
    } else {
        // Normal mode, just add to model
        m_elementModel->addElement(m_element);
    }

    // Select the newly created element
    if (m_selectionManager && m_element) {
        m_selectionManager->selectOnly(m_element);
    }
    
    // Log to console
    qDebug() << QString("Created %1 element (ID: %2) at position (%3, %4)")
                .arg(m_elementType)
                .arg(m_element ? m_element->getId() : "unknown")
                .arg(m_rect.x())
                .arg(m_rect.y());
    
    // Don't sync with API here - it will be synced after resize is complete
}

void CreateDesignElementCommand::undo()
{
    if (!m_elementModel) return;

    // Get the Project from the ElementModel's parent
    Project* project = qobject_cast<Project*>(m_elementModel->parent());
    if (!project) {
        qWarning() << "CreateDesignElementCommand: ElementModel has no Project parent";
        return;
    }

    // Check if we're in variant mode
    bool isVariantMode = project->viewMode() == "variant";
    Component* editingComponent = nullptr;
    
    if (isVariantMode) {
        QObject* editingElement = project->editingElement();
        editingComponent = qobject_cast<Component*>(editingElement);
    }

    // Clear selection if the created element is selected
    if (m_selectionManager && m_selectionManager->hasSelection() && m_element) {
        auto selectedElements = m_selectionManager->selectedElements();
        if (selectedElements.contains(m_element)) {
            m_selectionManager->clearSelection();
        }
    }

    // Remove element from model
    if (m_element) {
        m_elementModel->removeElement(m_element->getId());
        if (isVariantMode && editingComponent) {
            editingComponent->removeVariant(m_element);
        }
    }
}

void CreateDesignElementCommand::creationCompleted()
{
    // Sync with API now that the element has its final size
    syncWithAPI();
}

void CreateDesignElementCommand::syncWithAPI()
{
    if (!m_elementModel || !m_element) {
        return;
    }

    // Get the Project from the ElementModel's parent
    Project* project = qobject_cast<Project*>(m_elementModel->parent());
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

    // Get the project's API ID (assuming it's the same as the project ID for now)
    QString apiProjectId = project->id();
    
    // Serialize the element data
    QJsonObject elementData = app->serializeElement(m_element);
    
    // Sync with API
    apiClient->syncCreateElement(apiProjectId, elementData);
    
    qDebug() << "CreateDesignElementCommand: Syncing element creation with API for project" << apiProjectId;
}