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
#include "IModeHandler.h"
#include "SelectModeHandler.h"
#include "CreationModeHandler.h"
#include "FrameComponentVariant.h"
#include "TextComponentVariant.h"
#include "WebTextInputComponentVariant.h"
#include "Frame.h"
#include "Text.h"
#include "Shape.h"
#include "Node.h"
#include "Edge.h"
#include "Component.h"
#include "Project.h"
#include "DesignElement.h"
#include "UniqueIdGenerator.h"
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
        m_commandHistory.get(), setModeFunc);
    
    // Text mode
    m_modeHandlers[Mode::Text] = std::make_unique<CreationModeHandler>(
        CreationModeHandler::Config{
            "text",
            QVariant("Text")  // Default text content
        },
        &m_elementModel, &m_selectionManager,
        m_commandHistory.get(), setModeFunc);
    
    // WebTextInput mode
    m_modeHandlers[Mode::WebTextInput] = std::make_unique<CreationModeHandler>(
        CreationModeHandler::Config{
            "webtextinput",
            QVariant("Enter text...")  // Default placeholder
        },
        &m_elementModel, &m_selectionManager,
        m_commandHistory.get(), setModeFunc);
    
    // Shape Square mode
    m_modeHandlers[Mode::ShapeSquare] = std::make_unique<CreationModeHandler>(
        CreationModeHandler::Config{
            "shape",
            QVariant::fromValue(static_cast<int>(Shape::Square))
        },
        &m_elementModel, &m_selectionManager,
        m_commandHistory.get(), setModeFunc);
    
    // Shape Triangle mode
    m_modeHandlers[Mode::ShapeTriangle] = std::make_unique<CreationModeHandler>(
        CreationModeHandler::Config{
            "shape",
            QVariant::fromValue(static_cast<int>(Shape::Triangle))
        },
        &m_elementModel, &m_selectionManager,
        m_commandHistory.get(), setModeFunc);
    
    // Shape Line mode
    m_modeHandlers[Mode::ShapeLine] = std::make_unique<CreationModeHandler>(
        CreationModeHandler::Config{
            "shape",
            QVariant::fromValue(static_cast<int>(Shape::Line))
        },
        &m_elementModel, &m_selectionManager,
        m_commandHistory.get(), setModeFunc);
    
    
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
        case CanvasType::Variant:
            creationType = CreationManager::CanvasType::Design;  // Variant uses design creation
            hitTestType = HitTestService::CanvasType::Variant;
            break;
    }
    
    m_creationManager->setCanvasType(creationType);
    m_hitTestService->setCanvasType(hitTestType);
}

void CanvasController::setMode(Mode mode)
{
    if (m_mode != mode) {
        // Mode changing
        m_mode = mode;
        
        // Update current handler
        auto it = m_modeHandlers.find(m_mode);
        if (it != m_modeHandlers.end()) {
            m_currentHandler = it->second.get();
            // Handler found for mode
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

void CanvasController::createElement(const QString &type, qreal x, qreal y, qreal width, qreal height)
{
    m_creationManager->createElement(type, x, y, width, height);
}

void CanvasController::createVariable()
{
    // Variables are non-visual, but createElement still requires position/size parameters
    // The CreationManager will ignore these for non-visual elements
    Element* variable = m_creationManager->createElement("variable", 0, 0, 0, 0);
    
    if (variable) {
        // Select the newly created variable
        m_selectionManager.selectOnly(variable);
    }
}

void CanvasController::createNode(qreal x, qreal y, const QString &title, const QString &color)
{
    m_creationManager->createNode(x, y, title, color);
}

void CanvasController::createEdge(const QString &sourceNodeId, const QString &targetNodeId, 
                                  const QString &sourceHandleType, const QString &targetHandleType,
                                  int sourcePortIndex, int targetPortIndex)
{
    m_creationManager->createEdge(sourceNodeId, targetNodeId, sourceHandleType, targetHandleType,
                                  sourcePortIndex, targetPortIndex);
}

void CanvasController::createEdgeByPortId(const QString &sourceNodeId, const QString &targetNodeId,
                                          const QString &sourcePortId, const QString &targetPortId)
{
    m_creationManager->createEdgeByPortId(sourceNodeId, targetNodeId, sourcePortId, targetPortId);
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

void CanvasController::duplicateVariant(const QString &variantId)
{
    // Find the source variant
    Element* sourceElement = m_elementModel.getElementById(variantId);
    if (!sourceElement) {
        qWarning() << "duplicateVariant: Could not find variant with id" << variantId;
        return;
    }
    
    // Check if it's a DesignElement with isComponentVariant
    DesignElement* designElement = qobject_cast<DesignElement*>(sourceElement);
    if (!designElement || !designElement->isComponentVariant()) {
        qWarning() << "duplicateVariant: Element is not a ComponentVariant";
        return;
    }
    
    // Try to get the ComponentVariant interface
    ComponentVariant* sourceVariant = dynamic_cast<ComponentVariant*>(sourceElement);
    if (!sourceVariant) {
        qWarning() << "duplicateVariant: Element does not implement ComponentVariant interface";
        return;
    }
    
    // Find the parent component by searching through all components
    Component* component = nullptr;
    QList<Element*> allElements = m_elementModel.getAllElements();
    for (Element* elem : allElements) {
        if (Component* comp = qobject_cast<Component*>(elem)) {
            for (Element* variantElem : comp->variants()) {
                if (variantElem && variantElem->getId() == variantId) {
                    component = comp;
                    break;
                }
            }
            if (component) break;
        }
    }
    
    if (!component) {
        qWarning() << "duplicateVariant: Could not find parent component for variant";
        return;
    }
    
    // Generate new ID for the duplicate
    QString newId = UniqueIdGenerator::generate16DigitId();
    
    // Clone the variant using the virtual clone method
    ComponentVariant* clonedVariant = sourceVariant->clone(newId);
    Element* newElement = dynamic_cast<Element*>(clonedVariant);
    
    if (!clonedVariant || !newElement) {
        qWarning() << "duplicateVariant: Failed to clone variant";
        return;
    }
    
    // Generate a unique name
    int variantCount = component->variants().size();
    QString variantName = QString("Variant%1").arg(variantCount + 1);
    clonedVariant->setVariantName(variantName);
    
    // Position the new variant next to the source
    if (CanvasElement* canvasElement = qobject_cast<CanvasElement*>(newElement)) {
        canvasElement->setX(designElement->x() + designElement->width() + 50);
        canvasElement->setY(designElement->y());
    }
    
    // Add to component
    component->addVariant(newElement);
    
    // Add to element model
    m_elementModel.addElement(newElement);
    
    // Select the new variant
    m_selectionManager.clearSelection();
    m_selectionManager.selectElement(newElement);
    
    qDebug() << "Created duplicate variant:" << clonedVariant->variantName() << "with ID:" << newId;
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
    qDebug() << "CanvasController::undo() called";
    if (m_commandHistory) {
        qDebug() << "CanvasController::undo() - calling m_commandHistory->undo()";
        m_commandHistory->undo();
    } else {
        qDebug() << "CanvasController::undo() - m_commandHistory is null!";
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
        // Node ports are QStringList, so we use the port names directly
        QString sourcePort = sourceNode->outputPorts().first();
        QString targetPort = targetNode->inputPorts().first();
        
        // Create edge using port indices (0 for first port)
        m_creationManager->createEdge(sourceNodeId, targetNodeId, "output", "input", 0, 0);
    }
}


