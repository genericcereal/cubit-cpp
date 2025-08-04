#include "CreateComponentCommand.h"
#include "CreateInstanceCommand.h"
#include "../ElementModel.h"
#include "../DesignElement.h"
#include "../Frame.h"
#include "../Text.h"
#include "../Shape.h"
#include "../platforms/web/WebTextInput.h"
#include "../Serializer.h"
#include "../Application.h"
#include <QDebug>
#include <QJsonObject>

CreateComponentCommand::CreateComponentCommand(ElementModel* elementModel, DesignElement* sourceElement,
                                             QObject *parent)
    : Command(parent)
    , m_elementModel(elementModel)
    , m_sourceElement(sourceElement)
    , m_sourceElementId(sourceElement ? sourceElement->getId() : QString())
    , m_sourceParentId(sourceElement ? sourceElement->getParentElementId() : QString())
    , m_createdInstance(nullptr)
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

    // In the new pattern, the source element itself becomes the component definition
    // We just need to create an instance that references it
    
    // Generate instance ID if not already created
    if (m_instanceId.isEmpty()) {
        m_instanceId = m_elementModel->generateId();
    }
    
    // Create an instance of the source element
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
        // Set the instanceOf property to reference the source element
        instance->setInstanceOf(m_sourceElement->getId());
        instance->setName(m_sourceElement->getName() + " Instance");
        
        // Set parent if the source had one
        if (!m_sourceParentId.isEmpty()) {
            instance->setParentElementId(m_sourceParentId);
        }
        
        // Add the instance to the model
        m_elementModel->addElement(instance);
        m_createdInstance = instance;
        
                 << "of" << m_sourceElement->getId();
    }
}

void CreateComponentCommand::undo()
{
    if (m_createdInstance && m_elementModel) {
        // Remove the instance
        m_elementModel->removeElement(m_createdInstance->getId());
        m_createdInstance = nullptr;
    }
}

void CreateComponentCommand::redo()
{
    execute();
}