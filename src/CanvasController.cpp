#include "CanvasController.h"
#include "DragManager.h"
#include "CreationManager.h"
#include "HitTestService.h"
#include "JsonImporter.h"
#include "ElementModel.h"
#include "SelectionManager.h"
#include "CanvasElement.h"
#include "Config.h"
#include "CommandHistory.h"
#include "commands/CreateFrameCommand.h"
#include "commands/DeleteElementsCommand.h"
#include "commands/MoveElementsCommand.h"
#include "commands/ResizeElementCommand.h"
#include "commands/SetPropertyCommand.h"
#include "IModeHandler.h"
#include "SelectModeHandler.h"
#include "FrameModeHandler.h"
#include "TextModeHandler.h"
#include "HtmlModeHandler.h"
#include <QDebug>

CanvasController::CanvasController(QObject *parent)
    : QObject(parent)
    , m_mode(Mode::Select)
    , m_canvasType(CanvasType::Design)
    , m_elementModel(nullptr)
    , m_selectionManager(nullptr)
{
    initializeSubcontrollers();
    initializeModeHandlers();
}

CanvasController::~CanvasController() = default;

void CanvasController::initializeSubcontrollers()
{
    // Create subcontrollers
    m_dragManager = std::make_unique<DragManager>(this);
    m_creationManager = std::make_unique<CreationManager>(this);
    m_hitTestService = std::make_unique<HitTestService>(this);
    m_jsonImporter = std::make_unique<JsonImporter>(this);
    m_commandHistory = std::make_unique<CommandHistory>(this);
    
    // Connect drag manager signals
    connect(m_dragManager.get(), &DragManager::isDraggingChanged,
            this, &CanvasController::isDraggingChanged);
    connect(m_dragManager.get(), &DragManager::dragEnded,
            this, [this]() {
                // Create move command if elements were actually moved
                if (m_dragManager->hasDraggedMinDistance() && !m_dragManager->draggedElements().isEmpty()) {
                    auto command = std::make_unique<MoveElementsCommand>(
                        m_dragManager->draggedElements(), 
                        m_dragManager->totalDelta());
                    m_commandHistory->execute(std::move(command));
                }
            });
    
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
}

void CanvasController::initializeModeHandlers()
{
    // Create mode handlers
    auto setModeFunc = [this](const QString& mode) { this->setMode(mode); };
    
    m_modeHandlers[Mode::Select] = std::make_unique<SelectModeHandler>(
        m_dragManager.get(), m_hitTestService.get(), m_selectionManager);
    
    m_modeHandlers[Mode::Frame] = std::make_unique<FrameModeHandler>(
        m_creationManager.get(), m_elementModel, m_selectionManager, 
        m_commandHistory.get(), setModeFunc);
    
    m_modeHandlers[Mode::Text] = std::make_unique<TextModeHandler>(
        m_creationManager.get(), setModeFunc);
    
    m_modeHandlers[Mode::Html] = std::make_unique<HtmlModeHandler>(
        m_creationManager.get(), setModeFunc);
    
    // Set initial handler
    m_currentHandler = m_modeHandlers[m_mode].get();
}

void CanvasController::updateSubcontrollersCanvasType()
{
    auto creationType = (m_canvasType == CanvasType::Design) 
        ? CreationManager::CanvasType::Design 
        : CreationManager::CanvasType::Script;
    
    auto hitTestType = (m_canvasType == CanvasType::Design)
        ? HitTestService::CanvasType::Design
        : HitTestService::CanvasType::Script;
    
    m_creationManager->setCanvasType(creationType);
    m_hitTestService->setCanvasType(hitTestType);
}

void CanvasController::setMode(const QString &mode)
{
    Mode newMode = modeFromString(mode);
    qDebug() << "CanvasController::setMode called - current:" << modeToString(m_mode) << "new:" << mode;
    if (m_mode != newMode) {
        m_mode = newMode;
        
        // Update current handler
        auto it = m_modeHandlers.find(m_mode);
        if (it != m_modeHandlers.end()) {
            m_currentHandler = it->second.get();
        } else {
            m_currentHandler = nullptr;
            qWarning() << "No handler found for mode:" << mode;
        }
        
        qDebug() << "Mode changed, emitting signal";
        emit modeChanged();
    }
}

void CanvasController::setCanvasType(const QString &type)
{
    CanvasType newType = canvasTypeFromString(type);
    if (m_canvasType != newType) {
        m_canvasType = newType;
        updateSubcontrollersCanvasType();
        emit canvasTypeChanged();
    }
}

bool CanvasController::isDragging() const
{
    return m_dragManager->isDragging();
}

void CanvasController::setElementModel(ElementModel *model)
{
    m_elementModel = model;
    m_dragManager->setElementModel(model);
    m_creationManager->setElementModel(model);
    m_hitTestService->setElementModel(model);
    m_jsonImporter->setElementModel(model);
    
    // Reinitialize mode handlers with updated model
    if (m_selectionManager) {
        initializeModeHandlers();
    }
}

void CanvasController::setSelectionManager(SelectionManager *manager)
{
    m_selectionManager = manager;
    m_dragManager->setSelectionManager(manager);
    m_creationManager->setSelectionManager(manager);
    
    // Reinitialize mode handlers with updated selection manager
    if (m_elementModel) {
        initializeModeHandlers();
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
    if (!m_elementModel || !m_selectionManager) return;
    
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
    
    if (variable && m_selectionManager) {
        // Select the newly created variable
        m_selectionManager->selectOnly(variable);
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
    if (!m_selectionManager) return;
    
    QList<Element*> elementsInRect = m_hitTestService->elementsInRect(rect);
    
    if (!elementsInRect.isEmpty()) {
        m_selectionManager->selectAll(elementsInRect);
    } else {
        // Clear selection if no elements are in the rect
        m_selectionManager->clearSelection();
    }
}

void CanvasController::selectAll()
{
    if (!m_elementModel || !m_selectionManager) return;
    m_selectionManager->selectAll(m_elementModel->getAllElements());
}

void CanvasController::deleteSelectedElements()
{
    if (!m_elementModel || !m_selectionManager) return;
    
    QList<Element*> selectedElements = m_selectionManager->selectedElements();
    if (selectedElements.isEmpty()) return;
    
    // Create and execute delete command
    auto command = std::make_unique<DeleteElementsCommand>(
        m_elementModel, m_selectionManager, selectedElements);
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


// Helper methods for enum conversion
CanvasController::Mode CanvasController::modeFromString(const QString &str)
{
    if (str == "select") return Mode::Select;
    if (str == "frame") return Mode::Frame;
    if (str == "text") return Mode::Text;
    if (str == "html") return Mode::Html;
    
    qWarning() << "Unknown mode string:" << str << "defaulting to Select";
    return Mode::Select;
}

QString CanvasController::modeToString(Mode mode)
{
    switch (mode) {
        case Mode::Select: return "select";
        case Mode::Frame: return "frame";
        case Mode::Text: return "text";
        case Mode::Html: return "html";
    }
    return "select"; // Should never reach here
}

CanvasController::CanvasType CanvasController::canvasTypeFromString(const QString &str)
{
    if (str == "design") return CanvasType::Design;
    if (str == "script") return CanvasType::Script;
    
    qWarning() << "Unknown canvas type string:" << str << "defaulting to Design";
    return CanvasType::Design;
}

QString CanvasController::canvasTypeToString(CanvasType type)
{
    switch (type) {
        case CanvasType::Design: return "design";
        case CanvasType::Script: return "script";
    }
    return "design"; // Should never reach here
}