#include "HtmlModeHandler.h"
#include "CreationManager.h"

HtmlModeHandler::HtmlModeHandler(CreationManager* creationManager,
                                 std::function<void(CanvasController::Mode)> setModeFunc)
    : m_creationManager(creationManager)
    , m_setModeFunc(setModeFunc)
{
}

void HtmlModeHandler::onPress(qreal x, qreal y)
{
    if (m_creationManager) {
        m_creationManager->startDragCreation("html", QPointF(x, y));
    }
}

void HtmlModeHandler::onMove(qreal x, qreal y)
{
    if (m_creationManager) {
        m_creationManager->updateDragCreation(QPointF(x, y));
    }
}

void HtmlModeHandler::onRelease(qreal x, qreal y)
{
    Q_UNUSED(x)
    Q_UNUSED(y)
    
    if (m_creationManager) {
        // TODO: Handle Html creation with commands
        m_creationManager->finishDragCreation();
    }
    
    // Switch back to select mode
    if (m_setModeFunc) {
        m_setModeFunc(CanvasController::Mode::Select);
    }
}