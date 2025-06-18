#include "FrameModeHandler.h"
#include "CreationManager.h"
#include "ElementModel.h"
#include "SelectionManager.h"
#include "CommandHistory.h"
#include "CanvasElement.h"
#include "commands/CreateFrameCommand.h"
#include <QRectF>
#include <memory>

FrameModeHandler::FrameModeHandler(CreationManager* creationManager,
                                   ElementModel* elementModel,
                                   SelectionManager* selectionManager,
                                   CommandHistory* commandHistory,
                                   std::function<void(const QString&)> setModeFunc)
    : m_creationManager(creationManager)
    , m_elementModel(elementModel)
    , m_selectionManager(selectionManager)
    , m_commandHistory(commandHistory)
    , m_setModeFunc(setModeFunc)
{
}

void FrameModeHandler::onPress(qreal x, qreal y)
{
    if (m_creationManager) {
        m_creationManager->startDragCreation("frame", QPointF(x, y));
    }
}

void FrameModeHandler::onMove(qreal x, qreal y)
{
    if (m_creationManager) {
        m_creationManager->updateDragCreation(QPointF(x, y));
    }
}

void FrameModeHandler::onRelease(qreal x, qreal y)
{
    Q_UNUSED(x)
    Q_UNUSED(y)
    
    if (!m_creationManager || !m_elementModel || !m_commandHistory) return;
    
    // Finish drag creation
    Element* createdElement = m_creationManager->finishDragCreation();
    
    if (createdElement) {
        // For frames, we need to create a command for undo/redo support
        // First remove the element created by CreationManager
        m_elementModel->removeElement(createdElement->getId());
        
        // Then create it again using a command
        CanvasElement* canvasElement = qobject_cast<CanvasElement*>(createdElement);
        if (canvasElement) {
            QRectF rect(canvasElement->x(), canvasElement->y(), 
                       canvasElement->width(), canvasElement->height());
            auto command = std::make_unique<CreateFrameCommand>(
                m_elementModel, m_selectionManager, rect);
            m_commandHistory->execute(std::move(command));
        }
    }
    
    // Switch back to select mode
    if (m_setModeFunc) {
        m_setModeFunc("select");
    }
}