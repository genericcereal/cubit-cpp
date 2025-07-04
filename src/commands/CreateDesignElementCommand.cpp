#include "CreateDesignElementCommand.h"
#include "../Frame.h"
#include "../Text.h"
#include "../Html.h"
#include "../ElementModel.h"
#include "../SelectionManager.h"
#include "../Application.h"
#include "../Project.h"
#include "../Component.h"
#include <QDebug>

CreateDesignElementCommand::CreateDesignElementCommand(ElementModel* model, SelectionManager* selectionManager,
                                                     ElementType type, const QRectF& rect,
                                                     const QVariant& initialPayload, QObject *parent)
    : Command(parent)
    , m_elementModel(model)
    , m_selectionManager(selectionManager)
    , m_elementType(type)
    , m_rect(rect)
    , m_initialPayload(initialPayload)
    , m_frame(nullptr)
    , m_textElement(nullptr)
    , m_htmlElement(nullptr)
{
    QString typeStr;
    switch (type) {
    case FrameElement:
        typeStr = "Frame";
        break;
    case TextElement:
        typeStr = "Text";
        break;
    case HtmlElement:
        typeStr = "HTML";
        break;
    }
    
    setDescription(QString("Create %1 at (%2, %3)")
                   .arg(typeStr)
                   .arg(rect.x())
                   .arg(rect.y()));
}

CreateDesignElementCommand::~CreateDesignElementCommand()
{
    // QPointer will automatically be null if the objects were deleted
    // Only delete elements if they exist and the model is still valid
    if (m_elementModel) {
        // Check if elements are not in the model (undone state)
        auto allElements = m_elementModel->getAllElements();
        
        if (m_frame && !allElements.contains(m_frame)) {
            delete m_frame;
        }
        if (m_textElement && !allElements.contains(m_textElement)) {
            delete m_textElement;
        }
        if (m_htmlElement && !allElements.contains(m_htmlElement)) {
            delete m_htmlElement;
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

    // Create elements on first execution
    if (!m_frame) {
        m_frameId = m_elementModel->generateId();
        
        switch (m_elementType) {
        case FrameElement:
            // Just create a frame
            m_frame = new Frame(m_frameId);
            m_frame->setRect(m_rect);
            break;
            
        case TextElement:
            // Create text element without parent frame
            m_childElementId = m_elementModel->generateId();
            m_textElement = new Text(m_childElementId);
            m_textElement->setRect(m_rect);
            
            // Set initial text if provided
            if (m_initialPayload.isValid() && m_initialPayload.canConvert<QString>()) {
                m_textElement->setText(m_initialPayload.toString());
            } else {
                m_textElement->setText("Text");
            }
            break;
            
        case HtmlElement:
            // Create HTML element without parent frame
            m_childElementId = m_elementModel->generateId();
            m_htmlElement = new Html(m_childElementId);
            m_htmlElement->setRect(m_rect);
            
            // Set initial HTML content or URL if provided
            if (m_initialPayload.isValid()) {
                if (m_initialPayload.canConvert<QString>()) {
                    QString content = m_initialPayload.toString();
                    // Simple heuristic: if it starts with http, treat as URL
                    if (content.startsWith("http://") || content.startsWith("https://")) {
                        m_htmlElement->setUrl(content);
                    } else {
                        m_htmlElement->setHtml(content);
                    }
                }
            }
            break;
        }
    }

    // Add elements to appropriate container
    if (isVariantMode && editingComponent) {
        // In variant mode, add to the Component's variants array
        if (m_frame) {
            editingComponent->addVariant(m_frame);
            // Still add to model for visibility
            m_elementModel->addElement(m_frame);
        }
        if (m_textElement) {
            editingComponent->addVariant(m_textElement);
            // Still add to model for visibility
            m_elementModel->addElement(m_textElement);
        }
        if (m_htmlElement) {
            editingComponent->addVariant(m_htmlElement);
            // Still add to model for visibility
            m_elementModel->addElement(m_htmlElement);
        }
    } else {
        // Normal mode, just add to model
        if (m_frame) {
            m_elementModel->addElement(m_frame);
        }
        if (m_textElement) {
            m_elementModel->addElement(m_textElement);
        }
        if (m_htmlElement) {
            m_elementModel->addElement(m_htmlElement);
        }
    }

    // Select the newly created element
    if (m_selectionManager) {
        if (m_frame) {
            m_selectionManager->selectOnly(m_frame);
        } else if (m_textElement) {
            m_selectionManager->selectOnly(m_textElement);
        } else if (m_htmlElement) {
            m_selectionManager->selectOnly(m_htmlElement);
        }
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

    // Clear selection if any created element is selected
    if (m_selectionManager && m_selectionManager->hasSelection()) {
        auto selectedElements = m_selectionManager->selectedElements();
        if ((m_frame && selectedElements.contains(m_frame)) ||
            (m_textElement && selectedElements.contains(m_textElement)) ||
            (m_htmlElement && selectedElements.contains(m_htmlElement))) {
            m_selectionManager->clearSelection();
        }
    }

    // Remove elements from model in reverse order (children first)
    if (m_textElement) {
        m_elementModel->removeElement(m_textElement->getId());
        if (isVariantMode && editingComponent) {
            editingComponent->removeVariant(m_textElement);
        }
    }
    if (m_htmlElement) {
        m_elementModel->removeElement(m_htmlElement->getId());
        if (isVariantMode && editingComponent) {
            editingComponent->removeVariant(m_htmlElement);
        }
    }
    if (m_frame) {
        m_elementModel->removeElement(m_frame->getId());
        if (isVariantMode && editingComponent) {
            editingComponent->removeVariant(m_frame);
        }
    }
}