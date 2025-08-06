#include "CanvasController.h"
#include "CreationManager.h"
#include "HitTestService.h"
#include "JsonImporter.h"
#include "ElementModel.h"
#include "SelectionManager.h"
#include "CanvasElement.h"
#include "Config.h"
#include "CommandHistory.h"
#include <vector>
#include "commands/DeleteElementsCommand.h"
#include "commands/MoveElementsCommand.h"
#include "commands/ResizeElementCommand.h"
#include "commands/SetPropertyCommand.h"
#include "commands/ChangeParentCommand.h"
#include "commands/CreateComponentCommand.h"
#include "commands/CreateInstanceCommand.h"
#include "commands/DetachComponentCommand.h"
#include "commands/CreateScriptElementCommand.h"
#include "commands/CreateVariableCommand.h"
#include "commands/CompileScriptsCommand.h"
#include "commands/AssignVariableCommand.h"
#include "IModeHandler.h"
#include "SelectModeHandler.h"
#include "CreationModeHandler.h"
#include "PenModeHandler.h"
#include "Frame.h"
#include "Text.h"
#include "Shape.h"
#include "Node.h"
#include "Edge.h"
#include "Project.h"
#include "DesignElement.h"
#include "Scripts.h"
#include "ShapeControlsController.h"
#include "ConsoleMessageRepository.h"
#include "UniqueIdGenerator.h"
#include "HandleType.h"
#include <QDebug>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>

CanvasController::CanvasController(ElementModel& model,
                                   SelectionManager& sel,
                                   QObject *parent)
    : QObject(parent)
    , m_elementModel(model)
    , m_selectionManager(sel)
    , m_mode(Mode::Select)
    , m_canvasType(CanvasType::Design)
{
    // Create subcontrollers
    m_creationManager = std::make_unique<CreationManager>(this);
    m_hitTestService = std::make_unique<HitTestService>(this);
    m_jsonImporter = std::make_unique<JsonImporter>(this);
    m_commandHistory = std::make_unique<CommandHistory>(this);
    
    // Set element model and selection manager on subcontrollers
    m_creationManager->setElementModel(&m_elementModel);
    m_creationManager->setSelectionManager(&m_selectionManager);
    m_hitTestService->setElementModel(&m_elementModel);
    m_jsonImporter->setElementModel(&m_elementModel);
    
    // If parent is a Project, set it on CreationManager
    if (Project* project = qobject_cast<Project*>(parent)) {
        m_creationManager->setProject(project);
    }
    
    // Connect creation manager signals
    connect(m_creationManager.get(), &CreationManager::elementCreated,
            this, &CanvasController::elementCreated);
    
    // Connect command history signals
    connect(m_commandHistory.get(), &CommandHistory::canUndoChanged,
            this, &CanvasController::canUndoChanged);
    connect(m_commandHistory.get(), &CommandHistory::canRedoChanged,
            this, &CanvasController::canRedoChanged);
    
    // Set up canvas type
    updateSubcontrollersCanvasType();
    
    initializeModeHandlers();
}

CanvasController::~CanvasController() = default;


void CanvasController::initializeModeHandlers()
{
    // Create mode handlers
    auto setModeFunc = [this](Mode mode) { this->setMode(mode); };
    
    m_modeHandlers[Mode::Select] = std::make_unique<SelectModeHandler>(
        m_hitTestService.get(), &m_selectionManager);
    
    // Frame mode
    m_modeHandlers[Mode::Frame] = std::make_unique<CreationModeHandler>(
        CreationModeHandler::Config{
            "frame",
            QVariant()  // No default payload for frames
        },
        &m_elementModel, &m_selectionManager,
        m_commandHistory.get(), m_hitTestService.get(), setModeFunc);
    
    // Text mode
    m_modeHandlers[Mode::Text] = std::make_unique<CreationModeHandler>(
        CreationModeHandler::Config{
            "text",
            QVariant("Text")  // Default text content
        },
        &m_elementModel, &m_selectionManager,
        m_commandHistory.get(), m_hitTestService.get(), setModeFunc);
    
    // WebTextInput mode
    m_modeHandlers[Mode::WebTextInput] = std::make_unique<CreationModeHandler>(
        CreationModeHandler::Config{
            "webtextinput",
            QVariant("Enter text...")  // Default placeholder
        },
        &m_elementModel, &m_selectionManager,
        m_commandHistory.get(), m_hitTestService.get(), setModeFunc);
    
    // Shape Square mode
    m_modeHandlers[Mode::ShapeSquare] = std::make_unique<CreationModeHandler>(
        CreationModeHandler::Config{
            "shape",
            QVariant::fromValue(static_cast<int>(Shape::Square))
        },
        &m_elementModel, &m_selectionManager,
        m_commandHistory.get(), m_hitTestService.get(), setModeFunc);
    
    // Shape Triangle mode
    m_modeHandlers[Mode::ShapeTriangle] = std::make_unique<CreationModeHandler>(
        CreationModeHandler::Config{
            "shape",
            QVariant::fromValue(static_cast<int>(Shape::Triangle))
        },
        &m_elementModel, &m_selectionManager,
        m_commandHistory.get(), m_hitTestService.get(), setModeFunc);
    
    // Shape Pen mode - use specialized PenModeHandler
    m_modeHandlers[Mode::ShapePen] = std::make_unique<PenModeHandler>(
        &m_elementModel, &m_selectionManager,
        m_commandHistory.get(), setModeFunc, m_shapeControlsController);
    
    
    // Set initial handler
    m_currentHandler = m_modeHandlers[m_mode].get();
}

void CanvasController::updateSubcontrollersCanvasType()
{
    CreationManager::CanvasType creationType;
    HitTestService::CanvasType hitTestType;
    
    switch (m_canvasType) {
        case CanvasType::Design:
            creationType = CreationManager::CanvasType::Design;
            hitTestType = HitTestService::CanvasType::Design;
            break;
        case CanvasType::Script:
            creationType = CreationManager::CanvasType::Script;
            hitTestType = HitTestService::CanvasType::Script;
            break;
    }
    
    m_creationManager->setCanvasType(creationType);
    m_hitTestService->setCanvasType(hitTestType);
}

void CanvasController::setMode(Mode mode)
{
    if (m_mode != mode) {
        Mode oldMode = m_mode;
        m_mode = mode;
        
        // Clear selection when switching from Select mode to any creation mode
        if (oldMode == Mode::Select && mode != Mode::Select) {
            m_selectionManager.clearSelection();
        }
        
        // Update current handler
        auto it = m_modeHandlers.find(m_mode);
        if (it != m_modeHandlers.end()) {
            m_currentHandler = it->second.get();
        } else {
            m_currentHandler = nullptr;
            qWarning() << "No handler found for mode:" << static_cast<int>(mode);
        }
        
        emit modeChanged();
    }
}

void CanvasController::setCanvasType(CanvasType type)
{
    if (m_canvasType != type) {
        m_canvasType = type;
        updateSubcontrollersCanvasType();
        emit canvasTypeChanged();
    }
}

void CanvasController::setEditingElement(QObject* editingElement)
{
    if (m_hitTestService) {
        m_hitTestService->setEditingElement(editingElement);
    }
}



Element* CanvasController::hitTest(qreal x, qreal y)
{
    return m_hitTestService->hitTest(x, y);
}

Element* CanvasController::hitTestForHover(qreal x, qreal y)
{
    return m_hitTestService->hitTestForHover(x, y);
}

void CanvasController::handleMousePress(qreal x, qreal y)
{
    // Handle mouse press
    if (m_currentHandler) {
        m_currentHandler->onPress(x, y);
    } else {
        qWarning() << "CanvasController::handleMousePress - No handler for mode" << static_cast<int>(m_mode);
    }
}

void CanvasController::handleMouseMove(qreal x, qreal y)
{
    if (m_currentHandler) {
        m_currentHandler->onMove(x, y);
    }
}

void CanvasController::handleMouseRelease(qreal x, qreal y)
{
    if (m_currentHandler) {
        m_currentHandler->onRelease(x, y);
    }
}

void CanvasController::handleEscapeKey()
{
    // Handle escape key - no longer used for line creation mode
    // Left for other escape functionality if needed
}

void CanvasController::handleEnterKey()
{
    // Handle enter key - currently only used for pen creation mode
    if (m_mode == Mode::ShapePen) {
        PenModeHandler* penHandler = static_cast<PenModeHandler*>(m_currentHandler);
        if (penHandler) {
            penHandler->onEnterPressed();
        }
    }
}

void CanvasController::createElement(const QString &type, qreal x, qreal y, qreal width, qreal height)
{
    m_creationManager->createElement(type, x, y, width, height);
}

void CanvasController::createVariable()
{
    // Use command pattern for undo/redo support
    auto command = std::make_unique<CreateVariableCommand>(&m_elementModel, &m_selectionManager);
    m_commandHistory->execute(std::move(command));
}

void CanvasController::createNode(qreal x, qreal y, const QString &title, const QString &color)
{
    // Get the Project from the ElementModel's parent
    Project* project = qobject_cast<Project*>(m_elementModel.parent());
    if (!project) {
        qWarning() << "CanvasController::createNode - ElementModel has no Project parent";
        return;
    }
    
    Scripts* scripts = project->activeScripts();
    if (!scripts) {
        qWarning() << "CanvasController::createNode - No active scripts";
        return;
    }
    
    // Create the node using command pattern
    QVariantMap payload;
    payload["nodeTitle"] = title;
    if (!color.isEmpty()) {
        payload["nodeColor"] = color;
    }
    // TODO: Set nodeType based on the color or other parameters
    
    auto command = std::make_unique<CreateScriptElementCommand>(
        &m_elementModel, &m_selectionManager, scripts, "node",
        QRectF(x, y, 200, 100), payload);
    
    m_commandHistory->execute(std::move(command));
}

void CanvasController::createEdge(const QString &sourceNodeId, const QString &targetNodeId, 
                                  const QString &sourceHandleType, const QString &targetHandleType,
                                  int sourcePortIndex, int targetPortIndex)
{
    Q_UNUSED(sourceHandleType)
    Q_UNUSED(targetHandleType)
    // Get the Project from the ElementModel's parent
    Project* project = qobject_cast<Project*>(m_elementModel.parent());
    if (!project) {
        qWarning() << "CanvasController::createEdge - ElementModel has no Project parent";
        return;
    }
    
    Scripts* scripts = project->activeScripts();
    if (!scripts) {
        qWarning() << "CanvasController::createEdge - No active scripts";
        return;
    }
    
    // Validate that nodes exist and ports are compatible
    Element *sourceElement = m_elementModel.getElementById(sourceNodeId);
    Element *targetElement = m_elementModel.getElementById(targetNodeId);
    
    if (!sourceElement || !targetElement) {
        return;
    }
    
    // Check if nodes are the correct type and get port types
    Node *srcNode = qobject_cast<Node*>(sourceElement);
    Node *tgtNode = qobject_cast<Node*>(targetElement);
    
    if (!srcNode || !tgtNode) {
        return;
    }
    
    // Get port types
    QString sourcePortType = srcNode->getOutputPortType(sourcePortIndex);
    QString targetPortType = tgtNode->getInputPortType(targetPortIndex);
    
    // Validate that port types can connect
    if (!PortType::canConnect(sourcePortType, targetPortType)) {
        qWarning() << "Port types cannot connect"
                   << "source:" << sourcePortType 
                   << "target:" << targetPortType;
        return;
    }
    
    // Create the edge using command pattern
    QVariantMap payload;
    payload["sourceNodeId"] = sourceNodeId;
    payload["targetNodeId"] = targetNodeId;
    payload["sourcePortIndex"] = sourcePortIndex;
    payload["targetPortIndex"] = targetPortIndex;
    payload["sourcePortType"] = sourcePortType;
    payload["targetPortType"] = targetPortType;
    
    auto command = std::make_unique<CreateScriptElementCommand>(
        &m_elementModel, &m_selectionManager, scripts, "edge",
        QRectF(), payload);
    
    m_commandHistory->execute(std::move(command));
}

void CanvasController::createEdgeByPortId(const QString &sourceNodeId, const QString &targetNodeId,
                                          const QString &sourcePortId, const QString &targetPortId)
{
    // Get the Project from the ElementModel's parent
    Project* project = qobject_cast<Project*>(m_elementModel.parent());
    if (!project) {
        qWarning() << "CanvasController::createEdgeByPortId - ElementModel has no Project parent";
        return;
    }
    
    Scripts* scripts = project->activeScripts();
    if (!scripts) {
        qWarning() << "CanvasController::createEdgeByPortId - No active scripts";
        return;
    }
    
    // Find the source and target nodes
    Element *sourceElement = m_elementModel.getElementById(sourceNodeId);
    Element *targetElement = m_elementModel.getElementById(targetNodeId);
    
    if (!sourceElement || !targetElement) {
        return;
    }
    
    // Cast to Node to get port information
    Node *srcNode = qobject_cast<Node*>(sourceElement);
    Node *tgtNode = qobject_cast<Node*>(targetElement);
    
    if (!srcNode || !tgtNode) {
        return;
    }
    
    // Find port indices from port IDs
    int sourcePortIndex = srcNode->getOutputPortIndex(sourcePortId);
    int targetPortIndex = tgtNode->getInputPortIndex(targetPortId);
    
    if (sourcePortIndex == -1 || targetPortIndex == -1) {
        qWarning() << "Invalid port indices"
                   << "sourcePortId:" << sourcePortId << "index:" << sourcePortIndex
                   << "targetPortId:" << targetPortId << "index:" << targetPortIndex;
        return;
    }
    
    // Call the main createEdge method with the resolved indices
    createEdge(sourceNodeId, targetNodeId, "right", "left", sourcePortIndex, targetPortIndex);
}

QString CanvasController::createNodeFromJson(const QString &jsonData)
{
    // Ensure JsonImporter has the CreationManager set
    m_jsonImporter->setCreationManager(m_creationManager.get());
    return m_jsonImporter->createNodeFromJson(jsonData);
}

void CanvasController::createNodesFromJson(const QString &jsonData)
{
    m_jsonImporter->setCreationManager(m_creationManager.get());
    m_jsonImporter->createNodesFromJson(jsonData);
}

void CanvasController::createGraphFromJson(const QString &jsonData)
{
    m_jsonImporter->setCreationManager(m_creationManager.get());
    m_jsonImporter->createGraphFromJson(jsonData);
}

void CanvasController::createComponent(DesignElement* sourceElement)
{
    if (!sourceElement || !m_commandHistory) {
        qWarning() << "CanvasController::createComponent - Invalid source element or command history";
        return;
    }
    
    // Create and execute the command
    auto command = std::make_unique<CreateComponentCommand>(&m_elementModel, &m_selectionManager, sourceElement);
    m_commandHistory->execute(std::move(command));
}

void CanvasController::detachComponent(DesignElement* instanceElement)
{
    if (!instanceElement || !m_commandHistory) {
        qWarning() << "CanvasController::detachComponent - Invalid instance element or command history";
        return;
    }
    
    // Check if the element has an instanceOf property set
    if (instanceElement->instanceOf().isEmpty()) {
        qWarning() << "CanvasController::detachComponent - Element is not a component instance";
        return;
    }
    
    // Create and execute the detach command
    auto command = std::make_unique<DetachComponentCommand>(instanceElement, &m_selectionManager);
    m_commandHistory->execute(std::move(command));
}


void CanvasController::selectElementsInRect(const QRectF &rect)
{
    
    std::vector<Element*> elementsInRect = m_hitTestService->elementsInRect(rect);
    
    if (!elementsInRect.empty()) {
        m_selectionManager.selectAll(elementsInRect);
    } else {
        // Clear selection if no elements are in the rect
        m_selectionManager.clearSelection();
    }
}

void CanvasController::selectAll()
{
    QList<Element*> allElements = m_elementModel.getAllElements();
    std::vector<Element*> elementsVec(allElements.begin(), allElements.end());
    m_selectionManager.selectAll(elementsVec);
}

void CanvasController::deleteSelectedElements()
{
    QList<Element*> selectedElements = m_selectionManager.selectedElements();
    if (selectedElements.isEmpty()) return;
    
    // Create and execute delete command
    auto command = std::make_unique<DeleteElementsCommand>(
        &m_elementModel, &m_selectionManager, selectedElements);
    m_commandHistory->execute(std::move(command));
}

bool CanvasController::canUndo() const
{
    return m_commandHistory && m_commandHistory->canUndo();
}

bool CanvasController::canRedo() const
{
    return m_commandHistory && m_commandHistory->canRedo();
}

void CanvasController::undo()
{
    if (m_commandHistory) {
        m_commandHistory->undo();
    } else {
    }
}

void CanvasController::redo()
{
    if (m_commandHistory) {
        m_commandHistory->redo();
    }
}

void CanvasController::setSavedContentX(qreal x)
{
    if (m_savedContentX != x) {
        m_savedContentX = x;
        emit savedContentXChanged();
    }
}

void CanvasController::setSavedContentY(qreal y)
{
    if (m_savedContentY != y) {
        m_savedContentY = y;
        emit savedContentYChanged();
    }
}

void CanvasController::setSavedZoom(qreal zoom)
{
    if (m_savedZoom != zoom) {
        m_savedZoom = zoom;
        emit savedZoomChanged();
    }
}

void CanvasController::setShapeControlsController(QObject* controller)
{
    ShapeControlsController* shapeController = qobject_cast<ShapeControlsController*>(controller);
    if (m_shapeControlsController != shapeController) {
        m_shapeControlsController = shapeController;
        
        // Update existing PenModeHandler if it exists
        auto it = m_modeHandlers.find(Mode::ShapePen);
        if (it != m_modeHandlers.end()) {
            // Recreate the PenModeHandler with the new controller
            auto setModeFunc = [this](Mode newMode) { this->setMode(newMode); };
            m_modeHandlers[Mode::ShapePen] = std::make_unique<PenModeHandler>(
                &m_elementModel, &m_selectionManager,
                m_commandHistory.get(), setModeFunc, m_shapeControlsController);
            
            // Update current handler if we're in pen mode
            if (m_mode == Mode::ShapePen) {
                m_currentHandler = m_modeHandlers[Mode::ShapePen].get();
            }
        }
        
        emit shapeControlsControllerChanged();
    }
}

void CanvasController::createFrame(const QRectF& rect)
{
    // Use the creation manager to create a frame
    m_creationManager->createElement("frame", rect.x(), rect.y(), rect.width(), rect.height());
}

void CanvasController::createTextInFrame(Frame* frame, const QString& text)
{
    if (!frame) return;
    
    // Create a text element inside the frame
    QString textId = UniqueIdGenerator::generate16DigitId();
    Text* textElement = new Text(textId, &m_elementModel);
    textElement->setContent(text);
    textElement->setParentElementId(frame->getId());
    textElement->setX(0);  // Position at top-left of frame
    textElement->setY(0);
    textElement->setWidth(frame->width());
    textElement->setHeight(30);  // Default text height
    
    m_elementModel.addElement(textElement);
    
    // Select the frame (not the text)
    m_selectionManager.clearSelection();
    m_selectionManager.selectElement(frame);
}

void CanvasController::moveElements(const QList<Element*>& elements, const QPointF& delta)
{
    if (elements.isEmpty()) return;
    
    // Create and execute move command
    auto command = std::make_unique<MoveElementsCommand>(elements, delta);
    m_commandHistory->execute(std::move(command));
}

void CanvasController::moveElementsWithOriginalPositions(const QList<Element*>& elements, const QPointF& delta, const QHash<QString, QPointF>& originalPositions)
{
    if (elements.isEmpty()) return;
    
    // Create and execute move command with original positions
    auto command = std::make_unique<MoveElementsCommand>(elements, delta, originalPositions);
    m_commandHistory->execute(std::move(command));
}

void CanvasController::resizeElement(CanvasElement* element, const QRectF& oldRect, const QRectF& newRect)
{
    if (!element) return;
    
    // Create and execute resize command with full rectangles (position + size)
    auto command = std::make_unique<ResizeElementCommand>(element, oldRect, newRect);
    m_commandHistory->execute(std::move(command));
}

void CanvasController::setElementProperty(Element* element, const QString& property, const QVariant& value)
{
    if (!element) return;
    
    // Get current value
    QVariant oldValue = element->property(property.toUtf8().constData());
    
    // Create and execute set property command
    auto command = std::make_unique<SetPropertyCommand>(element, property, oldValue, value);
    m_commandHistory->execute(std::move(command));
}

void CanvasController::createNode(const QPointF& position, const QString& nodeType, const QString& nodeTitle)
{
    // Use existing createNode method with appropriate parameters
    createNode(position.x(), position.y(), nodeTitle, nodeType);
}

void CanvasController::createEdge(const QString& sourceNodeId, const QString& targetNodeId)
{
    // Find nodes
    Element* sourceElement = m_elementModel.getElementById(sourceNodeId);
    Element* targetElement = m_elementModel.getElementById(targetNodeId);
    
    Node* sourceNode = qobject_cast<Node*>(sourceElement);
    Node* targetNode = qobject_cast<Node*>(targetElement);
    
    if (!sourceNode || !targetNode) return;
    
    // Create edge between first output port of source and first input port of target
    if (!sourceNode->outputPorts().isEmpty() && !targetNode->inputPorts().isEmpty()) {
        // Create edge using port indices (0 for first port)
        createEdge(sourceNodeId, targetNodeId, "output", "input", 0, 0);
    }
}

void CanvasController::setElementParent(Element* element, const QString& newParentId)
{
    if (!element) return;
    
    // Create and execute change parent command
    auto command = std::make_unique<ChangeParentCommand>(element, newParentId);
    m_commandHistory->execute(std::move(command));
}

void CanvasController::setElementParentWithPosition(DesignElement* element, CanvasElement* newParent, qreal relX, qreal relY)
{
    if (!element) return;
    
    // Create and execute change parent command with position
    QPointF relativePosition(relX, relY);
    auto command = std::make_unique<ChangeParentCommand>(element, newParent, relativePosition);
    m_commandHistory->execute(std::move(command));
}

void CanvasController::compileScripts()
{
    // Get the Scripts object from the active project context (same pattern as other methods)
    Project* project = qobject_cast<Project*>(m_elementModel.parent());
    if (!project) {
        qWarning() << "Cannot compile scripts: No project found";
        return;
    }
    
    Scripts* scripts = project->activeScripts();
    if (!scripts) {
        qWarning() << "Cannot compile scripts: No active scripts found";
        return;
    }
    
    // Get console message repository for logging
    ConsoleMessageRepository* console = project->console();
    
    // Create and execute compile command with all required dependencies
    auto command = std::make_unique<CompileScriptsCommand>(scripts, &m_elementModel, console, project);
    m_commandHistory->execute(std::move(command));
}

void CanvasController::assignVariable(const QString& variableId, const QString& elementId, const QString& propertyName)
{
    // Get the project from the element model's parent
    Project* project = qobject_cast<Project*>(m_elementModel.parent());
    if (!project) {
        qWarning() << "Cannot assign variable: No project found";
        return;
    }
    
    // Create and execute assign variable command
    auto command = std::make_unique<AssignVariableCommand>(project, variableId, elementId, propertyName);
    m_commandHistory->execute(std::move(command));
}

void CanvasController::removeVariableBinding(const QString& elementId, const QString& propertyName)
{
    // Get the project from the element model's parent
    Project* project = qobject_cast<Project*>(m_elementModel.parent());
    if (!project) {
        qWarning() << "Cannot remove variable binding: No project found";
        return;
    }
    
    // Create and execute assign variable command for removal
    auto command = std::make_unique<AssignVariableCommand>(project, elementId, propertyName);
    m_commandHistory->execute(std::move(command));
}


