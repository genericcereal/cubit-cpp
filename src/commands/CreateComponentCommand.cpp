#include "CreateComponentCommand.h"
#include "CreateInstanceCommand.h"
#include "../ElementModel.h"
#include "../DesignElement.h"
#include "../Component.h"
#include "../ComponentVariant.h"
#include "../ComponentVariantTemplate.h"
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
{
    if (sourceElement) {
        setDescription(QString("Create component from %1").arg(sourceElement->getName()));
    } else {
        setDescription("Create component");
    }
}

CreateComponentCommand::~CreateComponentCommand()
{
    // QPointer will automatically be null if objects were deleted
}

void CreateComponentCommand::execute()
{
    if (!m_elementModel || !m_sourceElement) {
        qWarning() << "CreateComponentCommand: Invalid element model or source element";
        return;
    }

    // Generate component ID if not already created
    if (m_componentId.isEmpty()) {
        m_componentId = m_elementModel->generateId();
    }

    // Create the component
    m_createdComponent = new Component(m_componentId, m_elementModel);
    m_createdComponent->setName(m_sourceElement->getName() + " Component");
    m_createdComponent->setComponentType(m_sourceElement->getTypeName());

    // Generate variant ID
    if (m_variantId.isEmpty()) {
        m_variantId = m_elementModel->generateId();
    }

    // Create a ComponentVariant based on the source element type
    Element* variantElement = nullptr;
    
    // First serialize the source element to get all its properties
    Serializer* serializer = Application::instance()->serializer();
    QJsonObject elementData = serializer->serializeElement(m_sourceElement);
    
    // Update the element data for the variant
    elementData["elementId"] = m_variantId;
    elementData["name"] = "Default Variant";
    
    // Create the appropriate ComponentVariant type based on source type
    if (qobject_cast<Frame*>(m_sourceElement)) {
        FrameComponentVariant* frameVariant = new FrameComponentVariant(m_variantId, m_elementModel);
        variantElement = frameVariant;
    } else if (qobject_cast<Text*>(m_sourceElement)) {
        TextComponentVariant* textVariant = new TextComponentVariant(m_variantId, m_elementModel);
        variantElement = textVariant;
    } else if (qobject_cast<Shape*>(m_sourceElement)) {
        ShapeComponentVariant* shapeVariant = new ShapeComponentVariant(m_variantId, m_elementModel);
        variantElement = shapeVariant;
    } else if (qobject_cast<WebTextInput*>(m_sourceElement)) {
        WebTextInputComponentVariant* webTextVariant = new WebTextInputComponentVariant(m_variantId, m_elementModel);
        variantElement = webTextVariant;
    } else {
        qWarning() << "CreateComponentCommand: Unsupported element type" << m_sourceElement->getTypeName();
        delete m_createdComponent;
        m_createdComponent = nullptr;
        return;
    }
    
    if (!variantElement) {
        qWarning() << "CreateComponentCommand: Failed to create variant element";
        delete m_createdComponent;
        m_createdComponent = nullptr;
        return;
    }
    
    // Copy all properties from the serialized data
    QStringList propNames = m_sourceElement->propertyNames();
    for (const QString& propName : propNames) {
        // Skip properties that shouldn't be copied
        if (propName != "elementId" && propName != "parentId" && propName != "selected") {
            QVariant value = m_sourceElement->getProperty(propName);
            variantElement->setProperty(propName, value);
        }
    }
    
    // Set the name explicitly
    variantElement->setName("Default Variant");

    // Add the variant element to the element model
    m_elementModel->addElement(variantElement);
    
    // Add the variant to the component
    m_createdComponent->addVariant(variantElement);

    // Add the component to the element model
    m_elementModel->addElement(m_createdComponent);

    // Replace the original element with an instance of the component
    // First, get the parent of the source element
    QString parentId = m_sourceElement->getParentElementId();
    
    // Create an instance command to replace the source element
    m_createInstanceCommand = std::make_unique<CreateInstanceCommand>(
        m_elementModel, variantElement, parentId, this);
    m_createInstanceCommand->execute();

    // Get the created instance
    DesignElement* instance = m_createInstanceCommand->getCreatedInstance();
    if (instance) {
        // Copy the position and size from the original element
        instance->setX(m_sourceElement->x());
        instance->setY(m_sourceElement->y());
        instance->setWidth(m_sourceElement->width());
        instance->setHeight(m_sourceElement->height());
        
        // Remove the original element (without deleting it, so we can restore it on undo)
        m_elementModel->removeElementWithoutDelete(m_sourceElement);
    }
}

void CreateComponentCommand::undo()
{
    if (!m_elementModel) {
        qWarning() << "CreateComponentCommand: Cannot undo - element model is null";
        return;
    }

    // Undo the instance creation first
    if (m_createInstanceCommand) {
        m_createInstanceCommand->undo();
    }

    // Restore the original element if it was removed
    if (m_sourceElement && !m_elementModel->getElementById(m_sourceElement->getId())) {
        m_elementModel->addElement(m_sourceElement);
    }

    // Remove the variant element from the model
    if (m_createdComponent) {
        // Get the variant from the component before removing
        const QList<Element*>& variants = m_createdComponent->variants();
        for (Element* variant : variants) {
            if (variant && variant->getId() == m_variantId) {
                m_elementModel->removeElement(variant);
                break;
            }
        }
    }

    // Remove the component
    if (m_createdComponent) {
        m_elementModel->removeElement(m_createdComponent);
        m_createdComponent = nullptr;
    }
}