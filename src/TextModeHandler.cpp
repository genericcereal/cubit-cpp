#include "TextModeHandler.h"
#include "CreationManager.h"

TextModeHandler::TextModeHandler(CreationManager* creationManager,
                                 std::function<void(CanvasController::Mode)> setModeFunc)
    : m_creationManager(creationManager)
    , m_setModeFunc(setModeFunc)
{
}

void TextModeHandler::onPress(qreal x, qreal y)
{
    if (m_creationManager) {
        m_creationManager->startDragCreation("text", QPointF(x, y));
    }
}

void TextModeHandler::onMove(qreal x, qreal y)
{
    if (m_creationManager) {
        m_creationManager->updateDragCreation(QPointF(x, y));
    }
}

void TextModeHandler::onRelease(qreal x, qreal y)
{
    Q_UNUSED(x)
    Q_UNUSED(y)
    
    if (m_creationManager) {
        // TODO: Handle Text creation with commands
        m_creationManager->finishDragCreation();
    }
    
    // Switch back to select mode
    if (m_setModeFunc) {
        m_setModeFunc(CanvasController::Mode::Select);
    }
}