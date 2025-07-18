#include "CreateDesignElementCommand.h"
#include "DesignElement.h"
#include "Text.h"
#include "ElementModel.h"
#include "SelectionManager.h"
#include "Application.h"
#include "Project.h"
#include "Component.h"
#include "ElementTypeRegistry.h"
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


    // Check if we're in variant mode
    Application* app = Application::instance();
    Project* activeCanvas = app ? app->activeCanvas() : nullptr;
    bool isVariantMode = activeCanvas && activeCanvas->viewMode() == "variant";
    Component* editingComponent = nullptr;
    
    if (isVariantMode && activeCanvas) {
        QObject* editingElement = activeCanvas->editingElement();
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
}

void CreateDesignElementCommand::undo()
{
    if (!m_elementModel) return;

    // Check if we're in variant mode
    Application* app = Application::instance();
    Project* activeCanvas = app ? app->activeCanvas() : nullptr;
    bool isVariantMode = activeCanvas && activeCanvas->viewMode() == "variant";
    Component* editingComponent = nullptr;
    
    if (isVariantMode && activeCanvas) {
        QObject* editingElement = activeCanvas->editingElement();
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