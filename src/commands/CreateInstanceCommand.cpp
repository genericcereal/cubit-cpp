#include "CreateInstanceCommand.h"
#include "../ElementModel.h"
#include "../Element.h"
#include "../DesignElement.h"
#include "../Frame.h"
// #include "../ComponentInstance.h" // Component system removed
// #include "../ComponentVariant.h" // Component system removed
#include "../Serializer.h"
#include "../Application.h"
#include "../Project.h"
#include "../Variable.h"
#include "../UniqueIdGenerator.h"
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
    
    // Mark as an instance by setting the instanceOf property
    m_createdInstance->setInstanceOf(m_sourceElement->getId());
    
    // Add to element model
    m_elementModel->addElement(m_createdInstance);
    
    // Create a Variable element for this component instance
    // Get the Project from the ElementModel's parent
    if (Project* project = qobject_cast<Project*>(m_elementModel->parent())) {
        // Create a Variable element that represents this instance
        QString variableId = UniqueIdGenerator::generate16DigitId();
        Variable* variable = new Variable(variableId, m_elementModel);
        
        // Configure the variable as an element variable
        variable->setName(m_createdInstance->getName());
        variable->setValue(m_instanceId);  // Store the instance ID as the value
        variable->setVariableType("string");
        variable->setVariableScope("element");  // Mark as element variable
        variable->setLinkedElementId(m_instanceId);  // Link to the instance element
        
        // Add the variable to the element model
        m_elementModel->addElement(variable);
        
        // Connect element's nameChanged signal to update the Variable
        connect(m_createdInstance, &Element::nameChanged, variable, [variable, instance = m_createdInstance]() {
            variable->setName(instance->getName());
        });
    }
}

void CreateInstanceCommand::undo()
{
    if (!m_elementModel || !m_createdInstance) {
        qWarning() << "CreateInstanceCommand: Cannot undo - invalid state";
        return;
    }

    // First, remove the associated Variable element if it exists
    if (Project* project = qobject_cast<Project*>(m_elementModel->parent())) {
        // Find and remove the Variable element with linkedElementId matching our instance
        QList<Element*> allElements = m_elementModel->getAllElements();
        for (Element* elem : allElements) {
            if (Variable* var = qobject_cast<Variable*>(elem)) {
                if (var->linkedElementId() == m_instanceId) {
                    m_elementModel->removeElement(var->getId());
                    break;
                }
            }
        }
    }
    
    // Remove the instance from the element model
    m_elementModel->removeElement(m_createdInstance);
    
    // The element model takes ownership, so it will delete the instance
    m_createdInstance = nullptr;
}