#include "CreateInstanceCommand.h"
#include "../ElementModel.h"
#include "../Element.h"
#include "../DesignElement.h"
#include "../Frame.h"
#include "../ComponentInstance.h"
#include "../ComponentVariant.h"
#include "../Serializer.h"
#include "../Application.h"
#include <QDebug>
#include <QJsonObject>

CreateInstanceCommand::CreateInstanceCommand(ElementModel* elementModel, Element* sourceElement,
                                           const QString& parentId, QObject *parent)
    : Command(parent)
    , m_elementModel(elementModel)
    , m_sourceElement(sourceElement)
    , m_parentId(parentId)
{
    if (sourceElement) {
        setDescription(QString("Create instance of %1").arg(sourceElement->getName()));
    } else {
        setDescription("Create instance");
    }
}

CreateInstanceCommand::~CreateInstanceCommand()
{
    // QPointer will automatically be null if objects were deleted
}

void CreateInstanceCommand::execute()
{
    if (!m_elementModel || !m_sourceElement) {
        qWarning() << "CreateInstanceCommand: Invalid element model or source element";
        return;
    }

    // Generate instance ID if not already created
    if (m_instanceId.isEmpty()) {
        m_instanceId = m_elementModel->generateId();
    }

    // Serialize the source element to create a copy
    Serializer* serializer = Application::instance()->serializer();
    QJsonObject elementData = serializer->serializeElement(m_sourceElement);
    
    // Update the element data for the instance
    elementData["elementId"] = m_instanceId;
    elementData["name"] = m_sourceElement->getName() + " Instance";
    
    // Set parent if specified
    if (!m_parentId.isEmpty()) {
        elementData["parentId"] = m_parentId;
    }
    
    // Deserialize to create the new instance
    Element* newElement = serializer->deserializeElement(elementData, m_elementModel);
    if (!newElement) {
        qWarning() << "CreateInstanceCommand: Failed to create instance";
        return;
    }

    // Cast to DesignElement (instances should be DesignElements)
    m_createdInstance = qobject_cast<DesignElement*>(newElement);
    if (!m_createdInstance) {
        qWarning() << "CreateInstanceCommand: Created element is not a DesignElement";
        delete newElement;
        return;
    }

    // TODO: Create ComponentInstance to establish the link
    // auto* componentInstance = new ComponentInstance(m_instanceId);
    // Set the master variant if the source element has one
    // This would require accessing the Component/ComponentVariant system
    
    // Mark as an instance by setting the source ID
    m_createdInstance->setSourceId(m_sourceElement->getId());
    
    // Add to element model
    m_elementModel->addElement(m_createdInstance);
}

void CreateInstanceCommand::undo()
{
    if (!m_elementModel || !m_createdInstance) {
        qWarning() << "CreateInstanceCommand: Cannot undo - invalid state";
        return;
    }

    // Remove the instance from the element model
    m_elementModel->removeElement(m_createdInstance);
    
    // The element model takes ownership, so it will delete the instance
    m_createdInstance = nullptr;
}