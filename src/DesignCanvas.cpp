#include "DesignCanvas.h"
#include "Element.h"
#include "DesignElement.h"
#include "Frame.h"
#include "Text.h"
#include "Shape.h"
#include "platforms/web/WebTextInput.h"
#include "SelectionManager.h"
#include "ElementModel.h"
#include "UniqueIdGenerator.h"
#include "Project.h"
#include "PlatformConfig.h"
#include <QDebug>

DesignCanvas::DesignCanvas(ElementModel& model,
                           SelectionManager& sel,
                           QObject *parent)
    : CanvasController(model, sel, parent)
{
    qDebug() << "DesignCanvas::DesignCanvas - Constructor called";
    
    // Connect to selection changes to clear hover when needed
    connect(&sel, &SelectionManager::selectionChanged,
            this, &DesignCanvas::onSelectionChanged,
            Qt::UniqueConnection);
            
    // Connect to mode changes to clear hover
    connect(this, &CanvasController::modeChanged,
            this, &DesignCanvas::onModeChanged,
            Qt::UniqueConnection);
            
    // Connect to element added to handle global frame instances
    connect(&model, &ElementModel::elementAdded,
            this, &DesignCanvas::onElementAdded,
            Qt::UniqueConnection);
    
    // Check for existing frames and add global instances to them
    QList<Element*> existingElements = model.getAllElements();
    qDebug() << "DesignCanvas::DesignCanvas - Found" << existingElements.size() << "existing elements";
    for (Element* elem : existingElements) {
        if (elem && elem->getType() == Element::FrameType) {
            Frame* frame = qobject_cast<Frame*>(elem);
            if (frame && frame->role() != Frame::appContainer) {
                qDebug() << "DesignCanvas::DesignCanvas - Adding global instances to existing frame" << frame->getId();
                addGlobalElementInstancesToFrame(frame);
            }
        }
    }
}

void DesignCanvas::setHoveredElement(QObject* element)
{
    if (m_hoveredElement != element) {
        // qDebug() << "DesignCanvas::setHoveredElement: changing from" << m_hoveredElement << "to" << element;
        m_hoveredElement = element;
        emit hoveredElementChanged();
    }
}

void DesignCanvas::setIsAnimating(bool animating)
{
    if (m_isAnimating != animating) {
        m_isAnimating = animating;
        emit isAnimatingChanged();
    }
}

void DesignCanvas::setIsDesignControlsResizingDisabled(bool disabled)
{
    if (m_isDesignControlsResizingDisabled != disabled) {
        m_isDesignControlsResizingDisabled = disabled;
        emit isDesignControlsResizingDisabledChanged();
    }
}

void DesignCanvas::setIsDesignControlsMovementDisabled(bool disabled)
{
    if (m_isDesignControlsMovementDisabled != disabled) {
        m_isDesignControlsMovementDisabled = disabled;
        emit isDesignControlsMovementDisabledChanged();
    }
}

void DesignCanvas::setIsEditingShape(bool editing)
{
    if (m_isEditingShape != editing) {
        m_isEditingShape = editing;
        emit isEditingShapeChanged();
    }
}

void DesignCanvas::updateHover(qreal x, qreal y)
{
    // Update hover in select mode
    if (mode() == Mode::Select) {
        Element* element = hitTestForHover(x, y);
        setHoveredElement(element);
    } else {
        // Clear hover in creation modes
        setHoveredElement(nullptr);
    }
}

bool DesignCanvas::isDescendantOf(QObject* elementA, QObject* elementB) const
{
    if (!elementA || !elementB) return false;
    
    Element* elemA = qobject_cast<Element*>(elementA);
    Element* elemB = qobject_cast<Element*>(elementB);
    if (!elemA || !elemB) return false;
    
    Element* current = elemA;
    int maxDepth = 100; // Prevent infinite loops
    int depth = 0;
    
    while (current && !current->getParentElementId().isEmpty() && depth < maxDepth) {
        if (current->getParentElementId() == elemB->getId()) {
            return true;
        }
        // Find the parent element
        current = m_elementModel.getElementById(current->getParentElementId());
        depth++;
    }
    
    return false;
}

bool DesignCanvas::isChildOfSelected(QObject* element) const
{
    if (!element) return false;
    
    const QList<Element*>& selectedElements = m_selectionManager.selectedElements();
    for (Element* selected : selectedElements) {
        if (isDescendantOf(element, selected)) {
            return true;
        }
    }
    return false;
}

void DesignCanvas::updateParentingDuringDrag()
{
    if (m_selectionManager.selectionCount() == 0) {
        return;
    }
    
    const QList<Element*>& selectedElements = m_selectionManager.selectedElements();
    
    // If no element is hovered, unparent the selected elements
    if (!m_hoveredElement) {
        for (Element* element : selectedElements) {
            if (!element) continue;
            
            // Guard: Don't change parentId of children of selected elements
            if (isChildOfSelected(element)) {
                continue;
            }
            
            // Guard: ComponentVariants shouldn't have parents
            if (element->getTypeName() == "FrameComponentVariant") {
                continue;
            }
            
            // Unparent the element by setting parentId to empty string
            if (!element->getParentElementId().isEmpty()) {
                element->setParentElementId("");
            }
        }
        return;
    }
    
    Element* hovered = qobject_cast<Element*>(m_hoveredElement);
    if (!hovered) {
        return;
    }
    
    for (Element* element : selectedElements) {
        if (!element) continue;
        
        // Guard: Don't change parentId of children of selected elements
        if (isChildOfSelected(element)) {
            continue;
        }
        
        // Guard: ComponentVariants shouldn't have parents
        if (element->getTypeName() == "FrameComponentVariant") {
            continue;
        }
        
        // Guard: Don't set an element as its own parent
        if (hovered->getId() == element->getId()) {
            continue;
        }
        
        // Guard: Don't create circular dependencies
        if (isDescendantOf(hovered, element)) {
            continue;
        }
        
        // Guard: Don't parent to children of selected elements
        if (isChildOfSelected(hovered)) {
            continue;
        }
        
        // Guard: Check if the hovered element accepts children
        Frame* frameHovered = qobject_cast<Frame*>(hovered);
        if (frameHovered && !frameHovered->acceptsChildren()) {
            continue;
        }
        
        // Guard: Don't parent relatively positioned elements to other relatively positioned elements
        Frame* frameElement = qobject_cast<Frame*>(element);
        if (frameElement && frameHovered) {
            if (frameElement->position() == Frame::Relative && 
                frameHovered->position() == Frame::Relative) {
                continue;
            }
        }
        
        // Update parentId if it's different
        if (element->getParentElementId() != hovered->getId()) {
            element->setParentElementId(hovered->getId());
        }
    }
}

void DesignCanvas::onSelectionChanged()
{
    clearHoverIfSelected();
    
    // Don't automatically enter shape editing mode on selection
    // Shape editing mode should be activated explicitly (e.g., double-click)
    // For now, always disable shape editing when selection changes
    setIsEditingShape(false);
}

void DesignCanvas::onModeChanged()
{
    setHoveredElement(nullptr);
}

void DesignCanvas::clearHoverIfSelected()
{
    if (m_hoveredElement) {
        Element* elem = qobject_cast<Element*>(m_hoveredElement);
        if (elem && m_selectionManager.isSelected(elem)) {
            setHoveredElement(nullptr);
        }
    }
}

void DesignCanvas::onElementAdded(Element* element)
{
    qDebug() << "DesignCanvas::onElementAdded - Element added:" << (element ? element->getId() : "null") 
             << "Type:" << (element ? element->getTypeName() : "null");
    
    // Skip if we're already adding instances to prevent infinite recursion
    if (m_isAddingInstances) {
        qDebug() << "DesignCanvas::onElementAdded - Skipping due to m_isAddingInstances flag";
        return;
    }
    
    // Check if a new frame was added (not a global frame)
    if (element && element->getType() == Element::FrameType) {
        Frame* newFrame = qobject_cast<Frame*>(element);
        qDebug() << "DesignCanvas::onElementAdded - Frame detected, role:" << (newFrame ? newFrame->role() : -1);
        if (newFrame && newFrame->role() != Frame::appContainer) {
            qDebug() << "DesignCanvas::onElementAdded - Adding global instances to frame" << newFrame->getId();
            // Add instances of all elements parented to the global frame
            addGlobalElementInstancesToFrame(newFrame);
        }
    }
}

Frame* DesignCanvas::findGlobalFrame() const
{
    // First check the current element model
    QList<Element*> elements = m_elementModel.getAllElements();
    for (Element* elem : elements) {
        Frame* frame = qobject_cast<Frame*>(elem);
        if (frame && frame->role() == Frame::appContainer) {
            return frame;
        }
    }
    
    // If not found, check the platform configs for global frames
    // Get the project from the element model's parent
    Project* project = qobject_cast<Project*>(m_elementModel.parent());
    if (project) {
        QList<PlatformConfig*> platforms = project->getAllPlatforms();
        for (PlatformConfig* platform : platforms) {
            if (platform && platform->globalElements()) {
                QList<Element*> globalElements = platform->globalElements()->getAllElements();
                for (Element* elem : globalElements) {
                    Frame* frame = qobject_cast<Frame*>(elem);
                    if (frame && frame->role() == Frame::appContainer) {
                        return frame;
                    }
                }
            }
        }
    }
    
    return nullptr;
}

void DesignCanvas::addGlobalElementInstancesToFrame(Frame* targetFrame)
{
    if (!targetFrame) return;
    
    qDebug() << "DesignCanvas: Adding global element instances to frame" << targetFrame->getId();
    
    // Set flag to prevent recursive instance creation
    m_isAddingInstances = true;
    
    // Get the project from the element model's parent
    Project* project = qobject_cast<Project*>(m_elementModel.parent());
    if (!project) {
        qDebug() << "DesignCanvas: No project found";
        m_isAddingInstances = false;
        return;
    }
    
    // Check all platform configs for global elements
    QList<PlatformConfig*> platforms = project->getAllPlatforms();
    qDebug() << "DesignCanvas: Found" << platforms.size() << "platforms";
    for (PlatformConfig* platform : platforms) {
        if (!platform || !platform->globalElements()) continue;
        
        ElementModel* globalElementModel = platform->globalElements();
        QList<Element*> globalElements = globalElementModel->getAllElements();
        qDebug() << "DesignCanvas: Platform" << platform->name() << "has" << globalElements.size() << "global elements";
        
        // Debug: List all elements in global elements
        for (Element* elem : globalElements) {
            qDebug() << "  - Element ID:" << elem->getId() << "Type:" << elem->getTypeName() << "Name:" << elem->getName() << "ParentId:" << elem->getParentElementId();
        }
        
        // Find the global frame in this platform
        Frame* globalFrame = nullptr;
        for (Element* elem : globalElements) {
            Frame* frame = qobject_cast<Frame*>(elem);
            if (frame) {
                qDebug() << "DesignCanvas: Checking frame" << frame->getId() << "with role" << frame->role() << "(appContainer =" << Frame::appContainer << ")";
                if (frame->role() == Frame::appContainer) {
                    globalFrame = frame;
                    qDebug() << "DesignCanvas: Found global frame" << frame->getId() << "with role appContainer";
                    break;
                }
            }
        }
        
        if (!globalFrame) {
            qDebug() << "DesignCanvas: No global frame found in platform" << platform->name();
            continue;
        }
        
        // Find all elements parented to the global frame
        int instanceCount = 0;
        for (Element* elem : globalElements) {
            if (elem != globalFrame && elem->getParentElementId() == globalFrame->getId()) {
                qDebug() << "DesignCanvas: Found element" << elem->getId() << "type" << elem->getTypeName() << "parented to global frame";
                instanceCount++;
            // Create an instance of this element in the target frame
            Element* instance = nullptr;
            
            // Create the appropriate type of element
            QString instanceId = UniqueIdGenerator::generate16DigitId();
            switch (elem->getType()) {
                case Element::FrameType:
                    instance = new Frame(instanceId, &m_elementModel);
                    break;
                case Element::TextType:
                    instance = new Text(instanceId, &m_elementModel);
                    break;
                case Element::ShapeType:
                    instance = new Shape(instanceId, &m_elementModel);
                    break;
                case Element::WebTextInputType:
                    instance = new WebTextInput(instanceId, &m_elementModel);
                    break;
                default:
                    continue; // Skip unsupported types
            }
            
            if (instance) {
                // Copy properties from source element
                QStringList propNames = elem->propertyNames();
                for (const QString& propName : propNames) {
                    if (propName != "elementId" && propName != "parentId") {
                        instance->setProperty(propName, elem->getProperty(propName));
                    }
                }
                
                // Set the parent to the target frame
                instance->setParentElementId(targetFrame->getId());
                
                // Mark as an instance (not shown in element list)
                instance->setShowInElementList(false);
                
                // Mark as a global instance to prevent recursive parenting
                instance->setIsGlobalInstance(true);
                
                // Add to the model
                m_elementModel.addElement(instance);
            }
        }
    }
        qDebug() << "DesignCanvas: Created" << instanceCount << "instances from platform" << platform->name();
    }
    
    // Clear flag after we're done
    m_isAddingInstances = false;
}