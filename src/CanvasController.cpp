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
#include <QDebug>

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
            CreateDesignElementCommand::FrameElement,
            QVariant()  // No default payload for frames
        },
        &m_elementModel, &m_selectionManager,
        m_commandHistory.get(), setModeFunc);
    
    // Text mode
    m_modeHandlers[Mode::Text] = std::make_unique<CreationModeHandler>(
        CreationModeHandler::Config{
            CreateDesignElementCommand::TextElement,
            QVariant("Text")  // Default text content
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
        m_mode = mode;
        
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
    if (m_currentHandler) {
        m_currentHandler->onPress(x, y);
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


