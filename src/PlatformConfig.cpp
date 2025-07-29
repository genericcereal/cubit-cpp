#include "PlatformConfig.h"
#include "Scripts.h"
#include "Node.h"
#include "UniqueIdGenerator.h"
#include "ElementModel.h"
#include "Frame.h"
#include "Element.h"
#include "Text.h"
#include "Shape.h"
#include "platforms/web/WebTextInput.h"
#include "ComponentInstanceTemplate.h"
#include "Project.h"
#include "CanvasElement.h"
#include <QDebug>
#include <QPointer>

PlatformConfig::PlatformConfig(Type type, QObject *parent)
    : QObject(parent), m_type(type)
{
    initializeFromType(type);

    // Create platform-specific scripts
    m_scripts = std::make_unique<Scripts>(this);

    // Create global elements model
    m_globalElements = std::make_unique<ElementModel>(this);

    // Create platform-specific onLoad node
    createPlatformOnLoadNode();

    // Create initial global elements
    createInitialGlobalElements();
    
    // Connect to element added signal to update existing frames
    connect(m_globalElements.get(), &ElementModel::elementAdded,
            this, [this](Element* element) {
                
                // Also connect to parentIdChanged for this element
                connect(element, &Element::parentIdChanged,
                        this, [this, element]() {
                    
                    // Check if we have a parent project with a main model
                    Project* project = nullptr;
                    QObject* p = this->parent();
                    while (p && !project) {
                        project = qobject_cast<Project*>(p);
                        p = p->parent();
                    }
                    
                    if (project && project->elementModel()) {
                        updateAllFramesWithNewGlobalElement(element, project->elementModel());
                    }
                });
                
                // Check if it already has the global frame as parent
                Project* project = nullptr;
                QObject* p = this->parent();
                while (p && !project) {
                    project = qobject_cast<Project*>(p);
                    p = p->parent();
                }
                
                if (project && project->elementModel()) {
                    updateAllFramesWithNewGlobalElement(element, project->elementModel());
                    // Connect property synchronization
                    connectGlobalElementPropertySync(element, project->elementModel());
                }
            });
}

PlatformConfig::~PlatformConfig() {
    // Disconnect all property sync connections before destruction
    if (m_globalElements) {
        QList<Element*> elements = m_globalElements->getAllElements();
        for (Element* element : elements) {
            if (element) {
                // Disconnect all connections from this element to prevent crashes
                disconnect(element, nullptr, this, nullptr);
            }
        }
    }
    
    // Clear the tracking set
    m_connectedGlobalElements.clear();
}

QString PlatformConfig::name() const
{
    return m_name;
}

Scripts *PlatformConfig::scripts() const
{
    return m_scripts.get();
}

QString PlatformConfig::displayName() const
{
    return m_displayName;
}

PlatformConfig::Type PlatformConfig::type() const
{
    return m_type;
}

ElementModel *PlatformConfig::globalElements() const
{
    return m_globalElements.get();
}

PlatformConfig *PlatformConfig::create(const QString &platformName, QObject *parent)
{
    Type type;
    QString lowerName = platformName.toLower();

    if (lowerName == "web")
    {
        type = Web;
    }
    else if (lowerName == "ios")
    {
        type = iOS;
    }
    else if (lowerName == "android")
    {
        type = Android;
    }
    else
    {
        qWarning() << "Unknown platform type:" << platformName << "- defaulting to Web";
        type = Web;
    }

    return new PlatformConfig(type, parent);
}

void PlatformConfig::initializeFromType(Type type)
{
    switch (type)
    {
    case Web:
        m_name = "web";
        m_displayName = "Web";
        break;
    case iOS:
        m_name = "iOS";
        m_displayName = "iOS";
        break;
    case Android:
        m_name = "android";
        m_displayName = "Android";
        break;
    }
}

void PlatformConfig::createPlatformOnLoadNode()
{
    if (!m_scripts)
        return;


    // Clear any default nodes first
    m_scripts->clear();

    // Create the platform-specific onLoad node
    QString nodeId = UniqueIdGenerator::generate16DigitId();
    Node *onLoadNode = new Node(nodeId, m_scripts.get());

    // Configure the onLoad node based on platform type (matching NodeCatalog.qml)
    QString nodeTitle;
    switch (m_type)
    {
    case Web:
        nodeTitle = "On Web Load";
        break;
    case iOS:
        nodeTitle = "On iOS Load";
        break;
    case Android:
        nodeTitle = "On Android Load";
        break;
    }

    onLoadNode->setNodeTitle(nodeTitle);
    onLoadNode->setNodeType("Event");

    // Set position (centered in a reasonable default location)
    onLoadNode->setX(-100);
    onLoadNode->setY(0);
    onLoadNode->setWidth(150);
    onLoadNode->setHeight(80);

    // Configure the node's ports based on the catalog
    // Platform onLoad nodes have one source port: "done" (Flow type)
    Node::RowConfig rowConfig;
    rowConfig.hasSource = true;
    rowConfig.sourceLabel = "Done";
    rowConfig.sourceType = "Flow";
    rowConfig.sourcePortIndex = 0;
    onLoadNode->addRow(rowConfig);

    // Add output port
    onLoadNode->addOutputPort("done");
    onLoadNode->setOutputPortType(0, "Flow");

    // Add the node to scripts
    m_scripts->addNode(onLoadNode);
}

void PlatformConfig::createInitialGlobalElements()
{
    if (!m_globalElements)
        return;


    // Create a default frame
    QString frameId = UniqueIdGenerator::generate16DigitId();
    Frame *frame = new Frame(frameId, m_globalElements.get());

    // Set frame properties
    frame->setName("Global Frame");
    frame->setX(100);
    frame->setY(100);

    // Set the platform for this frame
    frame->setPlatform(m_name);

    // Set role to appContainer AFTER platform - this is exclusive to global elements
    // (setPlatform would set it to container by default, so we override it here)
    frame->setRole(Frame::appContainer);

    // Set size based on platform type
    if (m_type == iOS)
    {
        // iPhone viewport size
        frame->setWidth(375);
        frame->setHeight(812);
    }
    else
    {
        // Default size for web and Android (for now)
        frame->setWidth(400);
        frame->setHeight(400);
    }

    frame->setFill(QColor("#4287f5"));        // Blue background
    frame->setBorderColor(QColor("#2563eb")); // Darker blue border
    frame->setBorderWidth(1);
    frame->setBorderRadius(4);

    // Add the frame to the global elements model
    m_globalElements->addElement(frame);
}

Frame *PlatformConfig::findGlobalFrame() const
{
    // Check the global elements model for the appContainer frame
    if (!m_globalElements)
        return nullptr;

    QList<Element *> elements = m_globalElements->getAllElements();
    for (Element *elem : elements)
    {
        Frame *frame = qobject_cast<Frame *>(elem);
        if (frame && frame->role() == Frame::appContainer)
        {
            return frame;
        }
    }

    return nullptr;
}

void PlatformConfig::addGlobalElementInstancesToFrame(Frame *targetFrame, ElementModel *targetModel)
{
    if (!targetFrame || !targetModel || !m_globalElements)
        return;


    // Set flag to prevent recursive instance creation
    m_isAddingInstances = true;

    Frame *globalFrame = findGlobalFrame();
    if (!globalFrame)
    {
        m_isAddingInstances = false;
        return;
    }

    QList<Element *> globalElements = m_globalElements->getAllElements();

    // Find all elements parented to the global frame
    for (Element *elem : globalElements)
    {
        if (elem != globalFrame && elem->getParentElementId() == globalFrame->getId())
        {

            // Create an instance of this element in the target frame
            Element *instance = nullptr;

            // Create the appropriate type of element
            QString instanceId = UniqueIdGenerator::generate16DigitId();
            switch (elem->getType())
            {
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
                continue; // Skip unsupported types
            }

            if (instance)
            {
                // Copy properties from source element
                QStringList propNames = elem->propertyNames();
                for (const QString &propName : propNames)
                {
                    // Don't copy elementId, parentId, or selected state
                    if (propName != "elementId" && propName != "parentId" && propName != "selected")
                    {
                        instance->setProperty(propName, elem->getProperty(propName));
                    }
                }
                
                // Explicitly set selected to false for new instances
                instance->setSelected(false);

                // Set the parent to the target frame with relative positioning
                if (DesignElement* designInstance = qobject_cast<DesignElement*>(instance)) {
                    // Get the position of the element relative to its parent (global frame)
                    CanvasElement* canvasElem = qobject_cast<CanvasElement*>(elem);
                    if (canvasElem && globalFrame) {
                        // Calculate relative position
                        qreal relativeX = canvasElem->x() - globalFrame->x();
                        qreal relativeY = canvasElem->y() - globalFrame->y();
                        
                        // Set parent with relative position
                        designInstance->setParentElement(targetFrame, relativeX, relativeY);
                    } else {
                        // Fallback if no position info
                        designInstance->setParentElement(targetFrame, 0, 0);
                    }
                } else {
                    // Fallback for non-DesignElements
                    instance->setParentElementId(targetFrame->getId());
                }


                // Set isFrozen to true for global element instances
                if (DesignElement* designInstance = qobject_cast<DesignElement*>(instance)) {
                    designInstance->setIsFrozen(true);
                    // Set the source element ID for tracking
                    designInstance->setGlobalElementSourceId(elem->getId());
                }

                // Mark as a global instance to prevent recursive parenting
                // Check if this is a ComponentInstance type
                if (auto frameInstance = qobject_cast<FrameComponentInstance *>(instance))
                {
                    frameInstance->setIsGlobalInstance(true);
                }
                else if (auto textInstance = qobject_cast<TextComponentInstance *>(instance))
                {
                    textInstance->setIsGlobalInstance(true);
                }
                else if (auto shapeInstance = qobject_cast<ShapeComponentInstance *>(instance))
                {
                    shapeInstance->setIsGlobalInstance(true);
                }
                else if (auto webTextInstance = qobject_cast<WebTextInputComponentInstance *>(instance))
                {
                    webTextInstance->setIsGlobalInstance(true);
                }

                // Add to the model
                targetModel->addElement(instance);
                
                // Connect property synchronization for this global element
                Project* project = nullptr;
                QObject* p = this->parent();
                while (p && !project) {
                    project = qobject_cast<Project*>(p);
                    p = p->parent();
                }
                
                if (project && project->elementModel()) {
                    connectGlobalElementPropertySync(elem, project->elementModel());
                }
            }
        }
    }


    // Clear flag after we're done
    m_isAddingInstances = false;
}

void PlatformConfig::updateAllFramesWithNewGlobalElement(Element* globalElement, ElementModel* mainModel)
{
    if (!globalElement || !mainModel || !m_globalElements || m_isAddingInstances)
        return;
        
    // Check if this element is parented to the global frame
    Frame* globalFrame = findGlobalFrame();
    if (!globalFrame || globalElement->getParentElementId() != globalFrame->getId())
        return;
        
    
    // Set flag to prevent recursive instance creation
    m_isAddingInstances = true;
    
    // Find all top-level frames in the main model (frames without parents)
    QList<Element*> allElements = mainModel->getAllElements();
    
    for (Element* elem : allElements) {
        if (elem && elem->getType() == Element::FrameType && elem->getParentElementId().isEmpty()) {
            Frame* frame = qobject_cast<Frame*>(elem);
            if (frame && frame->role() != Frame::appContainer) {
                
                // Create an instance of the new global element
                Element* instance = nullptr;
                QString instanceId = UniqueIdGenerator::generate16DigitId();
                
                switch (globalElement->getType()) {
                case Element::FrameType:
                    instance = new Frame(instanceId, mainModel);
                    break;
                case Element::TextType:
                    instance = new Text(instanceId, mainModel);
                    break;
                case Element::ShapeType:
                    instance = new Shape(instanceId, mainModel);
                    break;
                case Element::WebTextInputType:
                    instance = new WebTextInput(instanceId, mainModel);
                    break;
                default:
                    continue; // Skip unsupported types
                }
                
                if (instance) {
                    // Copy properties from source element
                    QStringList propNames = globalElement->propertyNames();
                    for (const QString &propName : propNames) {
                        // Don't copy elementId, parentId, or selected state
                        if (propName != "elementId" && propName != "parentId" && propName != "selected") {
                            instance->setProperty(propName, globalElement->getProperty(propName));
                        }
                    }
                    
                    // Explicitly set selected to false for new instances
                    instance->setSelected(false);
                    
                    // Set the parent to the target frame with relative positioning
                    if (DesignElement* designInstance = qobject_cast<DesignElement*>(instance)) {
                        // Get the position of the global element relative to its parent (global frame)
                        CanvasElement* globalCanvasElement = qobject_cast<CanvasElement*>(globalElement);
                        if (globalCanvasElement) {
                            Frame* globalFrame = findGlobalFrame();
                            if (globalFrame) {
                                // Calculate relative position
                                qreal relativeX = globalCanvasElement->x() - globalFrame->x();
                                qreal relativeY = globalCanvasElement->y() - globalFrame->y();
                                
                                // Set parent with relative position
                                designInstance->setParentElement(frame, relativeX, relativeY);
                            } else {
                                // Fallback if no global frame found
                                designInstance->setParentElement(frame, 0, 0);
                            }
                        }
                    } else {
                        // Fallback for non-DesignElements
                        instance->setParentElementId(frame->getId());
                    }
                    
                    
                    // Set isFrozen to true for global element instances
                    if (DesignElement* designInstance = qobject_cast<DesignElement*>(instance)) {
                        designInstance->setIsFrozen(true);
                        // Set the source element ID for tracking
                        designInstance->setGlobalElementSourceId(globalElement->getId());
                    }
                    
                    // Add to the model
                    mainModel->addElement(instance);
                    
                }
            }
        }
    }
    
    
    // Clear flag after we're done
    m_isAddingInstances = false;
}

void PlatformConfig::connectGlobalElementPropertySync(Element* globalElement, ElementModel* mainModel)
{
    if (!globalElement || !mainModel) return;
    
    QString elementId = globalElement->getId();
    
    // Check if already connected
    if (m_connectedGlobalElements.contains(elementId)) {
        return;
    }
    
    
    // Mark as connected
    m_connectedGlobalElements.insert(elementId);
    
    // Connect to property change signals
    // For CanvasElement properties (width, height only - position changes handled by move command)
    if (CanvasElement* canvasElement = qobject_cast<CanvasElement*>(globalElement)) {
        QPointer<Element> safeElement = globalElement;
        QPointer<ElementModel> safeModel = mainModel;
        
        // Don't connect to x/y changes - those will be handled by the move command
        // to avoid continuous updates during dragging
        
        connect(canvasElement, &CanvasElement::widthChanged,
                this, [this, safeElement, safeModel]() {
                    if (safeElement && safeModel) {
                        updateGlobalElementInstances(safeElement, safeModel);
                    }
                });
                
        connect(canvasElement, &CanvasElement::heightChanged,
                this, [this, safeElement, safeModel]() {
                    if (safeElement && safeModel) {
                        updateGlobalElementInstances(safeElement, safeModel);
                    }
                });
    }
    
    // For DesignElement anchor properties
    if (DesignElement* designElement = qobject_cast<DesignElement*>(globalElement)) {
        QPointer<Element> safeElement = globalElement;
        QPointer<ElementModel> safeModel = mainModel;
        
        // Don't connect to anchor position changes (left, right, top, bottom) as these
        // change continuously during drag. Position sync is handled by move command.
        
        // Only connect to anchor state changes (enabled/disabled)
        connect(designElement, &DesignElement::leftAnchoredChanged,
                this, [this, safeElement, safeModel]() {
                    if (safeElement && safeModel) {
                        updateGlobalElementInstances(safeElement, safeModel);
                    }
                });
                
        connect(designElement, &DesignElement::rightAnchoredChanged,
                this, [this, safeElement, safeModel]() {
                    if (safeElement && safeModel) {
                        updateGlobalElementInstances(safeElement, safeModel);
                    }
                });
                
        connect(designElement, &DesignElement::topAnchoredChanged,
                this, [this, safeElement, safeModel]() {
                    if (safeElement && safeModel) {
                        updateGlobalElementInstances(safeElement, safeModel);
                    }
                });
                
        connect(designElement, &DesignElement::bottomAnchoredChanged,
                this, [this, safeElement, safeModel]() {
                    if (safeElement && safeModel) {
                        updateGlobalElementInstances(safeElement, safeModel);
                    }
                });
    }
    
    // For Frame-specific properties
    if (Frame* frame = qobject_cast<Frame*>(globalElement)) {
        QPointer<Element> safeElement = globalElement;
        QPointer<ElementModel> safeModel = mainModel;
        
        connect(frame, &Frame::fillChanged,
                this, [this, safeElement, safeModel]() {
                    if (safeElement && safeModel) {
                        updateGlobalElementInstances(safeElement, safeModel);
                    }
                });
                
        connect(frame, &Frame::borderColorChanged,
                this, [this, safeElement, safeModel]() {
                    if (safeElement && safeModel) {
                        updateGlobalElementInstances(safeElement, safeModel);
                    }
                });
                
        connect(frame, &Frame::borderWidthChanged,
                this, [this, safeElement, safeModel]() {
                    if (safeElement && safeModel) {
                        updateGlobalElementInstances(safeElement, safeModel);
                    }
                });
                
        connect(frame, &Frame::borderRadiusChanged,
                this, [this, safeElement, safeModel]() {
                    if (safeElement && safeModel) {
                        updateGlobalElementInstances(safeElement, safeModel);
                    }
                });
    }
    
    // For Text-specific properties
    if (Text* text = qobject_cast<Text*>(globalElement)) {
        QPointer<Element> safeElement = globalElement;
        QPointer<ElementModel> safeModel = mainModel;
        
        connect(text, &Text::contentChanged,
                this, [this, safeElement, safeModel]() {
                    if (safeElement && safeModel) {
                        updateGlobalElementInstances(safeElement, safeModel);
                    }
                });
                
        connect(text, &Text::fontChanged,
                this, [this, safeElement, safeModel]() {
                    if (safeElement && safeModel) {
                        updateGlobalElementInstances(safeElement, safeModel);
                    }
                });
                
        connect(text, &Text::colorChanged,
                this, [this, safeElement, safeModel]() {
                    if (safeElement && safeModel) {
                        updateGlobalElementInstances(safeElement, safeModel);
                    }
                });
    }
}

void PlatformConfig::updateGlobalElementInstances(Element* sourceElement, ElementModel* mainModel)
{
    if (!sourceElement || !mainModel || m_isAddingInstances) {
        return;
    }
    
    // Check if the source element is still valid and not being destroyed
    if (!sourceElement->parent()) {
        return;
    }
    
    QString sourceId = sourceElement->getId();
    
    // Find all instances with this source ID
    QList<Element*> allElements = mainModel->getAllElements();
    
    
    for (Element* element : allElements) {
        if (DesignElement* designElement = qobject_cast<DesignElement*>(element)) {
            QString instanceSourceId = designElement->globalElementSourceId();
            if (!instanceSourceId.isEmpty() && instanceSourceId == sourceId) {
                
                // Update properties
                QStringList propNames = sourceElement->propertyNames();
                
                for (const QString& propName : propNames) {
                    // Skip properties that shouldn't be synced
                    if (propName != "elementId" && propName != "parentId" && 
                        propName != "selected" && propName != "x" && propName != "y") {
                        
                        QVariant value = sourceElement->getProperty(propName);
                        QVariant oldValue = element->getProperty(propName);
                        element->setProperty(propName, value);
                    }
                }
                
                // For parented elements, update relative position
                if (CanvasElement* sourceCanvas = qobject_cast<CanvasElement*>(sourceElement)) {
                    if (CanvasElement* instanceCanvas = qobject_cast<CanvasElement*>(element)) {
                        // Update size properties
                        instanceCanvas->setWidth(sourceCanvas->width());
                        instanceCanvas->setHeight(sourceCanvas->height());
                        
                        // Update relative position if both elements are parented
                        if (!sourceElement->getParentElementId().isEmpty() && !element->getParentElementId().isEmpty()) {
                            // Get the global frame to calculate relative position
                            Frame* globalFrame = findGlobalFrame();
                            if (globalFrame) {
                                // Calculate the source element's position relative to the global frame
                                qreal relativeX = sourceCanvas->x() - globalFrame->x();
                                qreal relativeY = sourceCanvas->y() - globalFrame->y();
                                
                                // Get the instance's parent frame
                                Element* instanceParent = mainModel->getElementById(element->getParentElementId());
                                if (CanvasElement* instanceParentCanvas = qobject_cast<CanvasElement*>(instanceParent)) {
                                    // Update the instance's absolute position based on its parent + relative offset
                                    instanceCanvas->setX(instanceParentCanvas->x() + relativeX);
                                    instanceCanvas->setY(instanceParentCanvas->y() + relativeY);
                                }
                            }
                        }
                    }
                }
                
            }
        }
    }
    
}

void PlatformConfig::connectAllGlobalElementsPropertySync(ElementModel* mainModel)
{
    if (!m_globalElements || !mainModel) return;
    
    
    QList<Element*> globalElements = m_globalElements->getAllElements();
    for (Element* element : globalElements) {
        connectGlobalElementPropertySync(element, mainModel);
    }
}

void PlatformConfig::updateGlobalElementsAfterMove(const QList<Element*>& movedElements, ElementModel* mainModel)
{
    if (!m_globalElements || !mainModel || movedElements.isEmpty()) return;
    
    
    // Check each moved element to see if it's a global element or instance
    for (Element* element : movedElements) {
        if (!element) continue;
        
        
        // First check if this element exists in our global elements
        if (m_globalElements->getElementById(element->getId())) {
            updateGlobalElementInstances(element, mainModel);
        } else {
            // Check if this is an instance of a global element (has globalElementSourceId)
            if (DesignElement* designElement = qobject_cast<DesignElement*>(element)) {
                QString sourceId = designElement->globalElementSourceId();
                if (!sourceId.isEmpty()) {
                    // Find the source global element
                    Element* sourceElement = m_globalElements->getElementById(sourceId);
                    if (sourceElement) {
                        updateGlobalElementInstances(sourceElement, mainModel);
                    } else {
                    }
                }
            }
        }
    }
}