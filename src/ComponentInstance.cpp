#include "ComponentInstance.h"
#include "ComponentVariant.h"
#include "Element.h"
#include "Frame.h"
#include "Text.h"
#include "Shape.h"
#include "platforms/web/WebTextInput.h"
#include "ElementModel.h"
#include "Application.h"
#include "Project.h"
#include "SelectionManager.h"
#include "UniqueIdGenerator.h"
#include <QDebug>

ComponentInstance::ComponentInstance(const QString& elementId)
    : m_elementId(elementId)
{
}

QString ComponentInstance::elementId() const
{
    return m_elementId;
}

ComponentVariant* ComponentInstance::masterVariant() const
{
    return m_masterVariant;
}

void ComponentInstance::setMasterVariant(ComponentVariant* variant)
{
    m_masterVariant = variant;
}

void ComponentInstance::handleGlobalFrameParenting(const QString& oldParentId, const QString& newParentId, Element* element)
{
    if (!element) return;
    
    qDebug() << "ComponentInstance::handleGlobalFrameParenting - Called for element" << element->getId() 
             << "old parent:" << oldParentId << "new parent:" << newParentId
             << "isGlobalInstance:" << m_isGlobalInstance << "isVisual:" << element->isVisual();
    
    // Skip if this is already a global instance
    if (m_isGlobalInstance) {
        qDebug() << "  Skipping - element is already a global instance";
        return;
    }
    
    // Only process if this element is visual and we have a model
    if (!element->isVisual()) {
        qDebug() << "  Skipping - element is not visual";
        return;
    }
    
    ElementModel* model = qobject_cast<ElementModel*>(element->parent());
    if (!model) {
        qDebug() << "  Skipping - no element model found";
        return;
    }
    
    // Check if we're being removed from a global frame
    if (!oldParentId.isEmpty()) {
        Element* oldParent = model->getElementById(oldParentId);
        qDebug() << "  Checking old parent" << oldParentId << "- found:" << (oldParent != nullptr)
                 << "is global frame:" << (oldParent ? isGlobalFrame(oldParent) : false);
        if (oldParent && isGlobalFrame(oldParent)) {
            qDebug() << "  Removing instances from all frames";
            removeInstancesFromAllFrames(element);
        }
    }
    
    // Check if we're being added to a global frame
    if (!newParentId.isEmpty()) {
        Element* newParent = model->getElementById(newParentId);
        qDebug() << "  Checking new parent" << newParentId << "- found:" << (newParent != nullptr)
                 << "is global frame:" << (newParent ? isGlobalFrame(newParent) : false);
        if (newParent && isGlobalFrame(newParent)) {
            qDebug() << "  Creating instances in all frames";
            createInstancesInAllFrames(element);
        }
    }
}

bool ComponentInstance::isGlobalFrame(Element* element) const
{
    if (!element) return false;
    
    Frame* frame = qobject_cast<Frame*>(element);
    if (!frame) return false;
    
    return frame->role() == Frame::appContainer;
}

void ComponentInstance::createInstancesInAllFrames(Element* element)
{
    if (!element) return;
    
    qDebug() << "ComponentInstance::createInstancesInAllFrames - Called for element" << element->getId();
    
    // We need to trigger the DesignCanvas to create instances
    // Find all canvases and trigger instance creation
    Application* app = Application::instance();
    if (!app) {
        qDebug() << "  No application instance found";
        return;
    }
    
    ElementModel* sourceModel = qobject_cast<ElementModel*>(element->parent());
    if (!sourceModel) {
        qDebug() << "  No source model found";
        return;
    }
    
    // For each project/canvas in the application
    for (const auto& project : app->canvases()) {
        ElementModel* canvasModel = project->elementModel();
        if (!canvasModel || canvasModel == sourceModel) {
            qDebug() << "  Skipping canvas (same as source or null)";
            continue;
        }
        
        qDebug() << "  Processing canvas with" << canvasModel->rowCount() << "elements";
        
        // Find all frames in this canvas (except global frames)
        QList<Element*> elements = canvasModel->getAllElements();
        for (Element* elem : elements) {
            if (elem && elem->getType() == Element::FrameType) {
                Frame* frame = qobject_cast<Frame*>(elem);
                if (frame && frame->role() != Frame::appContainer) {
                    qDebug() << "    Found target frame" << frame->getId() << "in canvas";
                    // Create instance of this element in the frame
                    createInstanceInFrame(frame, canvasModel, element);
                }
            }
        }
        
        // Clear selection after creating instances to prevent them from appearing selected
        if (project->selectionManager()) {
            project->selectionManager()->clearSelection();
        }
    }
}

void ComponentInstance::removeInstancesFromAllFrames(Element* element)
{
    if (!element) return;
    
    Application* app = Application::instance();
    if (!app) return;
    
    ElementModel* sourceModel = qobject_cast<ElementModel*>(element->parent());
    if (!sourceModel) return;
    
    // For each canvas/project in the application
    for (const auto& project : app->canvases()) {
        ElementModel* canvasModel = project->elementModel();
        if (!canvasModel || canvasModel == sourceModel) continue; // Skip the source canvas
        
        // Find and remove instances of this element
        QList<Element*> elements = canvasModel->getAllElements();
        QList<Element*> toRemove;
        
        for (Element* elem : elements) {
            // Check if this element is an instance by matching properties
            if (elem->getType() == element->getType() && 
                !elem->showInElementList() &&
                elem->getName() == element->getName()) {
                toRemove.append(elem);
            }
        }
        
        // Remove the instances
        for (Element* elem : toRemove) {
            canvasModel->removeElement(elem);
        }
    }
}

void ComponentInstance::createInstanceInFrame(Frame* targetFrame, ElementModel* targetModel, Element* element)
{
    if (!targetFrame || !targetModel || !element) return;
    
    qDebug() << "ComponentInstance::createInstanceInFrame - Creating instance of" << element->getId() 
             << "in frame" << targetFrame->getId();
    
    // Create the appropriate type of element
    Element* instance = nullptr;
    QString instanceId = UniqueIdGenerator::generate16DigitId();
    
    switch (element->getType()) {
        case Element::FrameType:
            instance = new Frame(instanceId, targetModel);
            break;
        case Element::TextType:
            instance = new Text(instanceId, targetModel);
            break;
        case Element::ShapeType:
            instance = new Shape(instanceId, targetModel);
            break;
        case Element::WebTextInputType:
            instance = new WebTextInput(instanceId, targetModel);
            break;
        default:
            return; // Unsupported type
    }
    
    if (instance) {
        // Copy properties from source element
        QStringList propNames = element->propertyNames();
        for (const QString& propName : propNames) {
            // Don't copy elementId, parentId, or selected state
            if (propName != "elementId" && propName != "parentId" && propName != "selected") {
                instance->setProperty(propName, element->getProperty(propName));
            }
        }
        
        // Explicitly set selected to false for new instances
        instance->setSelected(false);
        
        // Set the parent to the target frame
        instance->setParentElementId(targetFrame->getId());
        
        // Mark as an instance (not shown in element list)
        instance->setShowInElementList(false);
        
        // Mark as a global instance to prevent recursive parenting
        // Note: We need to cast to ComponentInstance to set this property
        // This will only work if the instance is actually a ComponentInstance type
        // For now, we'll use the Element's property system
        if (instance->hasProperty("isGlobalInstance")) {
            instance->setProperty("isGlobalInstance", true);
        }
        
        // Add to the model
        targetModel->addElement(instance);
    }
}