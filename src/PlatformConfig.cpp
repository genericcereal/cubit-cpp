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
#include <QDebug>

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
                qDebug() << "PlatformConfig: Element added to globalElements:" << element->getId();
                
                // Also connect to parentIdChanged for this element
                connect(element, &Element::parentIdChanged,
                        this, [this, element]() {
                    qDebug() << "PlatformConfig: Element parent changed:" << element->getId() 
                             << "new parent:" << element->getParentElementId();
                    
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
                }
            });
}

PlatformConfig::~PlatformConfig() = default;

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

    qDebug() << "Creating platform onLoad node for" << m_name;

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

    qDebug() << "Creating initial global elements for" << m_name;

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

    qDebug() << "PlatformConfig: Adding global element instances to frame" << targetFrame->getId();

    // Set flag to prevent recursive instance creation
    m_isAddingInstances = true;

    Frame *globalFrame = findGlobalFrame();
    if (!globalFrame)
    {
        qDebug() << "PlatformConfig: No global frame found";
        m_isAddingInstances = false;
        return;
    }

    QList<Element *> globalElements = m_globalElements->getAllElements();
    qDebug() << "PlatformConfig: Found" << globalElements.size() << "global elements";

    // Find all elements parented to the global frame
    int instanceCount = 0;
    for (Element *elem : globalElements)
    {
        if (elem != globalFrame && elem->getParentElementId() == globalFrame->getId())
        {
            qDebug() << "PlatformConfig: Found element" << elem->getId() << "type" << elem->getTypeName() << "parented to global frame";

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

                // Ensure instance is shown in element list
                instance->setShowInElementList(true);

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
                instanceCount++;
            }
        }
    }

    qDebug() << "PlatformConfig: Created" << instanceCount << "instances";

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
        
    qDebug() << "PlatformConfig: New global element added:" << globalElement->getId() 
             << "type:" << globalElement->getTypeName();
    
    // Set flag to prevent recursive instance creation
    m_isAddingInstances = true;
    
    // Find all top-level frames in the main model (frames without parents)
    QList<Element*> allElements = mainModel->getAllElements();
    int updatedFrameCount = 0;
    
    for (Element* elem : allElements) {
        if (elem && elem->getType() == Element::FrameType && elem->getParentElementId().isEmpty()) {
            Frame* frame = qobject_cast<Frame*>(elem);
            if (frame && frame->role() != Frame::appContainer) {
                qDebug() << "PlatformConfig: Adding instance to frame" << frame->getId();
                
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
                    
                    // Ensure instance is shown in element list
                    instance->setShowInElementList(true);
                    
                    // Add to the model
                    mainModel->addElement(instance);
                    
                    qDebug() << "PlatformConfig: Created instance" << instance->getId() 
                             << "with parent" << frame->getId()
                             << "showInElementList:" << instance->showInElementList();
                    
                    updatedFrameCount++;
                }
            }
        }
    }
    
    qDebug() << "PlatformConfig: Updated" << updatedFrameCount << "frames with new global element";
    
    // Clear flag after we're done
    m_isAddingInstances = false;
}