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
#include "PlatformConfig.h"
#include "Variable.h"
#include "UniqueIdGenerator.h"
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

    // Check if we're in variant mode or globalElements mode
    bool isVariantMode = project->viewMode() == "variant";
    bool isGlobalElementsMode = project->viewMode() == "globalElements";
    Component* editingComponent = nullptr;
    PlatformConfig* editingPlatform = nullptr;
    
    if (isVariantMode) {
        QObject* editingElement = project->editingElement();
        editingComponent = qobject_cast<Component*>(editingElement);
        if (!editingComponent) {
            qWarning() << "CreateDesignElementCommand: In variant mode but editing element is not a Component";
        }
    } else if (isGlobalElementsMode) {
        QObject* editingElement = project->editingElement();
        editingPlatform = qobject_cast<PlatformConfig*>(editingElement);
        if (!editingPlatform) {
            qWarning() << "CreateDesignElementCommand: In globalElements mode but editing element is not a PlatformConfig";
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
        
        // Apply initial payload BEFORE setting geometry to avoid wrong joint creation
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
        
        // Set position and size AFTER shape type is set
        m_element->setRect(m_rect);
    }

    // Add element to appropriate container
    if (isVariantMode && editingComponent) {
        // In variant mode, add to the Component's variants array
        editingComponent->addVariant(m_element);
        // Still add to model for visibility
        m_elementModel->addElement(m_element);
    } else if (isGlobalElementsMode && editingPlatform) {
        // In globalElements mode, add to the PlatformConfig's globalElements
        ElementModel* globalElements = editingPlatform->globalElements();
        if (globalElements) {
            globalElements->addElement(m_element);
            // Also add to main model for visibility (it will be filtered by ElementFilterProxy)
            m_elementModel->addElement(m_element);
        } else {
            qWarning() << "CreateDesignElementCommand: Platform has no globalElements model";
            m_elementModel->addElement(m_element);
        }
    } else {
        // Normal mode, just add to model
        m_elementModel->addElement(m_element);
    }

    // Select the newly created element
    if (m_selectionManager && m_element) {
        m_selectionManager->selectOnly(m_element);
    }
    
    // Create a Variable element for this design element
    // Only for design elements (not variants or components themselves)
    if (!isVariantMode && !isGlobalElementsMode && 
        m_element && project) {
        
        // Check if it's a design element (Frame, Text, Shape) or component instance
        bool shouldCreateVariable = false;
        
        if (m_elementType == "frame" || m_elementType == "text" || 
            m_elementType == "shape" || m_elementType == "webtextinput") {
            shouldCreateVariable = true;
        } else if (m_element->isComponentInstance()) {
            shouldCreateVariable = true;
        }
        
        if (shouldCreateVariable) {
            // Create a Variable element that represents this design element
            QString variableId = UniqueIdGenerator::generate16DigitId();
            Variable* variable = new Variable(variableId, m_elementModel);
            
            // Configure the variable as an element variable
            variable->setName(m_element->getName());
            variable->setValue(m_elementId);  // Store the element ID as the value
            variable->setVariableType("string");
            variable->setVariableScope("element");  // Mark as element variable
            variable->setLinkedElementId(m_elementId);  // Link to the source element
            
            // Add the variable to the element model
            m_elementModel->addElement(variable);
            
            // Connect element's nameChanged signal to update the Variable
            connect(m_element, &Element::nameChanged, variable, [variable, element = m_element]() {
                variable->setName(element->getName());
            });
            
            qDebug() << "Created Variable element for design element:" << m_element->getName() 
                     << "with ID:" << variableId;
        }
    }
    
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

    // Check if we're in variant mode or globalElements mode
    bool isVariantMode = project->viewMode() == "variant";
    bool isGlobalElementsMode = project->viewMode() == "globalElements";
    Component* editingComponent = nullptr;
    PlatformConfig* editingPlatform = nullptr;
    
    if (isVariantMode) {
        QObject* editingElement = project->editingElement();
        editingComponent = qobject_cast<Component*>(editingElement);
    } else if (isGlobalElementsMode) {
        QObject* editingElement = project->editingElement();
        editingPlatform = qobject_cast<PlatformConfig*>(editingElement);
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
        // First, remove the associated Variable element if it exists
        if (!isVariantMode && !isGlobalElementsMode && project) {
            // Find and remove the Variable element with linkedElementId matching our element
            QList<Element*> allElements = m_elementModel->getAllElements();
            for (Element* elem : allElements) {
                if (Variable* var = qobject_cast<Variable*>(elem)) {
                    if (var->linkedElementId() == m_element->getId()) {
                        m_elementModel->removeElement(var->getId());
                        qDebug() << "Removed Variable element for design element:" << m_element->getName();
                        break;
                    }
                }
            }
        }
        
        m_elementModel->removeElement(m_element->getId());
        if (isVariantMode && editingComponent) {
            editingComponent->removeVariant(m_element);
        } else if (isGlobalElementsMode && editingPlatform) {
            ElementModel* globalElements = editingPlatform->globalElements();
            if (globalElements) {
                globalElements->removeElement(m_element->getId());
            }
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
    
    
    if (m_elementType == "shape") {
        if (Shape* shape = qobject_cast<Shape*>(m_element)) {
        }
    }
    
    // Sync with API
    apiClient->syncCreateElement(apiProjectId, elementData);
    
}