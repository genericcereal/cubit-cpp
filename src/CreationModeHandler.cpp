#include "CreationModeHandler.h"
#include "ElementModel.h"
#include "SelectionManager.h"
#include "CommandHistory.h"
#include "CanvasElement.h"
#include "CanvasController.h"
#include "Config.h"
#include "commands/CreateDesignElementCommand.h"
#include <QRectF>
#include <memory>
#include <QDebug>

CreationModeHandler::CreationModeHandler(Config cfg,
                                         ElementModel* model,
                                         SelectionManager* selection,
                                         CommandHistory* history,
                                         std::function<void(CanvasController::Mode)> setMode)
    : m_cfg(cfg)
    , m_elementModel(model)
    , m_selectionManager(selection)
    , m_commandHistory(history)
    , m_setModeFunc(setMode)
{
}

void CreationModeHandler::onPress(qreal x, qreal y)
{
    // Handle creation mode press
    
    if (!m_commandHistory || !m_elementModel || !m_selectionManager) {
        qWarning() << "CreationModeHandler::onPress - Missing dependencies:" 
                   << "commandHistory:" << (m_commandHistory ? "ok" : "null")
                   << "elementModel:" << (m_elementModel ? "ok" : "null")
                   << "selectionManager:" << (m_selectionManager ? "ok" : "null");
        return;
    }
    
    // Store the start position for drag operation
    m_startPos = QPointF(x, y);
    m_isDragging = true;
    
    // Create element immediately at 1x1 size
    QRectF rect(x, y, 1, 1);
    // Creating command
    auto command = std::make_unique<CreateDesignElementCommand>(
        m_elementModel, m_selectionManager, 
        m_cfg.elementType, rect, m_cfg.defaultPayload);
    
    // Store raw pointer before moving ownership
    m_currentCommand = command.get();
    m_commandHistory->execute(std::move(command));
    
    // The element is now created and selected, ready for resize
}

void CreationModeHandler::onMove(qreal x, qreal y)
{
    if (!m_isDragging || !m_selectionManager) return;
    
    CanvasElement* element = currentElement();
    if (!element) return;
    
    // Behave like bottom-right resize joint:
    // - Top-left corner stays fixed at start position
    // - Bottom-right corner follows the mouse
    qreal width = x - m_startPos.x();
    qreal height = y - m_startPos.y();
    
    // Allow negative sizes for flipping, just like standard resize
    // The controls will handle the flipping behavior
    
    // Update element position and size to handle flipping
    if (width < 0) {
        element->setX(x);
        element->setWidth(qMax(-width, 1.0));
    } else {
        element->setX(m_startPos.x());
        element->setWidth(qMax(width, 1.0));
    }
    
    if (height < 0) {
        element->setY(y);
        element->setHeight(qMax(-height, 1.0));
    } else {
        element->setY(m_startPos.y());
        element->setHeight(qMax(height, 1.0));
    }
}

void CreationModeHandler::onRelease(qreal x, qreal y)
{
    Q_UNUSED(x)
    Q_UNUSED(y)
    
    if (!m_isDragging) return;
    m_isDragging = false;
    
    // If the element is too small, resize it to default size
    CanvasElement* element = currentElement();
    if (element && (element->width() < 10 || element->height() < 10)) {
        element->setWidth(qMax(element->width(), qreal(::Config::DEFAULT_ELEMENT_WIDTH)));
        element->setHeight(qMax(element->height(), qreal(::Config::DEFAULT_ELEMENT_HEIGHT)));
    }
    
    // Notify the command that creation is complete with final size
    if (m_currentCommand) {
        m_currentCommand->creationCompleted();
        m_currentCommand = nullptr; // Clear the reference
    }
    
    // Switch back to select mode
    if (m_setModeFunc) {
        m_setModeFunc(CanvasController::Mode::Select);
    }
}

CanvasElement* CreationModeHandler::currentElement() const
{
    if (!m_selectionManager) return nullptr;
    
    auto selectedElements = m_selectionManager->selectedElements();
    if (selectedElements.isEmpty()) return nullptr;
    
    return qobject_cast<CanvasElement*>(selectedElements.first());
}