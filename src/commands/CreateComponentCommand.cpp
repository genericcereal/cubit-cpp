#include "CreateComponentCommand.h"
#include "CreateInstanceCommand.h"
#include "../ElementModel.h"
#include "../SelectionManager.h"
#include "../DesignElement.h"
#include "../Frame.h"
#include "../Text.h"
#include "../Shape.h"
#include "../platforms/web/WebTextInput.h"
#include "../Component.h"
#include "../Serializer.h"
#include "../Application.h"
#include <QDebug>
#include <QJsonObject>

CreateComponentCommand::CreateComponentCommand(ElementModel* elementModel, SelectionManager* selectionManager,
                                             DesignElement* sourceElement, QObject *parent)
    : Command(parent)
    , m_elementModel(elementModel)
    , m_selectionManager(selectionManager)
    , m_sourceElement(sourceElement)
    , m_createdInstance(nullptr)
    , m_createdComponent(nullptr)
    , m_sourceElementId(sourceElement ? sourceElement->getId() : QString())
    , m_sourceParentId(sourceElement ? sourceElement->getParentElementId() : QString())
{
    if (sourceElement) {
        setDescription("Create component from " + sourceElement->getName());
    }
}

CreateComponentCommand::~CreateComponentCommand()
{
}

void CreateComponentCommand::execute()
{
    if (!m_elementModel || !m_sourceElement) {
        qWarning() << "CreateComponentCommand: Invalid element model or source element";
        return;
    }
    
    // Step 1: Create a new Component
    if (m_componentId.isEmpty()) {
        m_componentId = m_elementModel->generateId();
    }
    
    m_createdComponent = new ComponentElement(m_componentId, m_elementModel);
    m_createdComponent->setName(m_sourceElement->getName() + " Component");
    
    // Step 2: Add the selected element AND all its children to the component's elements array
    addElementAndChildrenToComponent(m_sourceElement);
    
    // Note: We keep the source element in the model but it will be filtered out
    // from the main canvas display since it's part of a component
    
    // Add the component to the model
    m_elementModel->addElement(m_createdComponent);
    
    // Step 3: Create an instance of the selected element
    if (m_instanceId.isEmpty()) {
        m_instanceId = m_elementModel->generateId();
    }
    
    DesignElement* instance = nullptr;
    
    if (Frame* sourceFrame = qobject_cast<Frame*>(m_sourceElement)) {
        Frame* frameInstance = new Frame(m_instanceId, m_elementModel);
        frameInstance->setRect(QRectF(sourceFrame->x(), sourceFrame->y(), 
                                     sourceFrame->width(), sourceFrame->height()));
        instance = frameInstance;
    } else if (Text* sourceText = qobject_cast<Text*>(m_sourceElement)) {
        Text* textInstance = new Text(m_instanceId, m_elementModel);
        textInstance->setRect(QRectF(sourceText->x(), sourceText->y(), 
                                    sourceText->width(), sourceText->height()));
        instance = textInstance;
    } else if (Shape* sourceShape = qobject_cast<Shape*>(m_sourceElement)) {
        Shape* shapeInstance = new Shape(m_instanceId, m_elementModel);
        shapeInstance->setRect(QRectF(sourceShape->x(), sourceShape->y(), 
                                     sourceShape->width(), sourceShape->height()));
        instance = shapeInstance;
    } else if (WebTextInput* sourceWebText = qobject_cast<WebTextInput*>(m_sourceElement)) {
        WebTextInput* webTextInstance = new WebTextInput(m_instanceId, m_elementModel);
        webTextInstance->setRect(QRectF(sourceWebText->x(), sourceWebText->y(), 
                                       sourceWebText->width(), sourceWebText->height()));
        instance = webTextInstance;
    } else {
        qWarning() << "CreateComponentCommand: Unsupported element type" << m_sourceElement->getTypeName();
        return;
    }
    
    if (instance) {
        // Set the instanceOf property to reference the source element ID
        // This allows the sync mechanism to work properly when the source element changes
        instance->setInstanceOf(m_sourceElement->getId());
        
        // Also set the componentId to track which component this instance belongs to
        instance->setComponentId(m_componentId);
        
        instance->setName(m_sourceElement->getName() + " Instance");
        
        // Set parent if the source had one
        if (!m_sourceParentId.isEmpty()) {
            instance->setParentElementId(m_sourceParentId);
        }
        
        // Add the instance to the model
        m_elementModel->addElement(instance);
        m_createdInstance = instance;
        
        // Create instances for any existing children of the source element
        for (Element* el : m_createdComponent->elements()) {
        }
        createChildInstances(m_sourceElement, instance);
        
        // Select the newly created instance
        if (m_selectionManager) {
            m_selectionManager->clearSelection();
            m_selectionManager->selectElement(instance);
        }    }
}

void CreateComponentCommand::undo()
{
    // Remove created child instances first
    for (DesignElement* childInstance : m_createdChildInstances) {
        if (childInstance && m_elementModel) {
            m_elementModel->removeElement(childInstance->getId());
        }
    }
    m_createdChildInstances.clear();
    
    if (m_createdInstance && m_elementModel) {
        // Remove the instance
        m_elementModel->removeElement(m_createdInstance->getId());
        m_createdInstance = nullptr;
    }
    
    if (m_createdComponent && m_elementModel) {
        // Remove all elements from the component's list
        for (DesignElement* element : m_addedToComponent) {
            m_createdComponent->removeElement(element);
            
            // Only reset the isVariant property for the outermost element (source element)
            if (element == m_sourceElement && element) {
                element->setProperty("isVariant", false);
            }
        }
        m_addedToComponent.clear();
        
        // Remove the component
        m_elementModel->removeElement(m_createdComponent->getId());
        m_createdComponent = nullptr;
    }
}

void CreateComponentCommand::redo()
{
    execute();
}

void CreateComponentCommand::createChildInstances(DesignElement* sourceElement, DesignElement* parentInstance)
{
    if (!sourceElement || !parentInstance || !m_elementModel || !m_createdComponent) {
        return;
    }
    
    // Get children from the component's elements array instead of the main model
    // This ensures we create instances for all elements that are part of the component
    QList<Element*> componentElements = m_createdComponent->elements();
    
    for (Element* element : componentElements) {
        if (element->getParentElementId() == sourceElement->getId()) {
            // Create an instance of this child element
            DesignElement* sourceChild = qobject_cast<DesignElement*>(element);
            if (!sourceChild) {
                continue;
            }
            
            QString childInstanceId = m_elementModel->generateId();
            DesignElement* childInstance = nullptr;
            
            // Create the appropriate type of instance
            if (Frame* sourceFrame = qobject_cast<Frame*>(sourceChild)) {
                Frame* frameInstance = new Frame(childInstanceId, m_elementModel);
                frameInstance->setRect(QRectF(sourceFrame->x(), sourceFrame->y(), 
                                             sourceFrame->width(), sourceFrame->height()));
                childInstance = frameInstance;
            } else if (Text* sourceText = qobject_cast<Text*>(sourceChild)) {
                Text* textInstance = new Text(childInstanceId, m_elementModel);
                textInstance->setRect(QRectF(sourceText->x(), sourceText->y(), 
                                            sourceText->width(), sourceText->height()));
                childInstance = textInstance;
            } else if (Shape* sourceShape = qobject_cast<Shape*>(sourceChild)) {
                Shape* shapeInstance = new Shape(childInstanceId, m_elementModel);
                shapeInstance->setRect(QRectF(sourceShape->x(), sourceShape->y(), 
                                             sourceShape->width(), sourceShape->height()));
                childInstance = shapeInstance;
            } else if (WebTextInput* sourceWebText = qobject_cast<WebTextInput*>(sourceChild)) {
                WebTextInput* webTextInstance = new WebTextInput(childInstanceId, m_elementModel);
                webTextInstance->setRect(QRectF(sourceWebText->x(), sourceWebText->y(), 
                                               sourceWebText->width(), sourceWebText->height()));
                childInstance = webTextInstance;
            }
            
            if (childInstance) {
                // Set the instanceOf property to reference the source child element ID
                childInstance->setInstanceOf(sourceChild->getId());
                // Set the componentId to track which component this instance belongs to
                childInstance->setComponentId(m_componentId);
                childInstance->setName(sourceChild->getName() + " Instance");
                
                // Set parent to the parent instance
                childInstance->setParentElementId(parentInstance->getId());
                
                // Add the child instance to the model
                m_elementModel->addElement(childInstance);
                m_createdChildInstances.append(childInstance);
                
                // Recursively create instances for children of this child
                createChildInstances(sourceChild, childInstance);
            }
        }
    }
}

void CreateComponentCommand::addElementAndChildrenToComponent(DesignElement* element)
{
    if (!element || !m_createdComponent) {
        return;
    }
    
    // Add this element to the component
    m_createdComponent->addElement(element);
    m_addedToComponent.append(element);
    
    // Only mark the outermost element (the source element) as a variant
    // Children should not have isVariant set to true
    if (element == m_sourceElement) {
        if (element->hasProperty("isVariant")) {
            element->setProperty("isVariant", true);
        } else {
            qWarning() << "Element does not have isVariant property:" << element->getId() << element->getTypeName();
        }
    }
    
    // Recursively add all children
    QList<Element*> allElements = m_elementModel->getAllElements();
    for (Element* child : allElements) {
        if (child->getParentElementId() == element->getId()) {
            DesignElement* designChild = qobject_cast<DesignElement*>(child);
            if (designChild) {
                addElementAndChildrenToComponent(designChild);
            }
        }
    }
}