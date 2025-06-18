#include "SelectModeHandler.h"
#include "DragManager.h"
#include "HitTestService.h"
#include "SelectionManager.h"
#include "Element.h"
#include <QDebug>

SelectModeHandler::SelectModeHandler(DragManager* dragManager, 
                                     HitTestService* hitTestService, 
                                     SelectionManager* selectionManager)
    : m_dragManager(dragManager)
    , m_hitTestService(hitTestService)
    , m_selectionManager(selectionManager)
{
}

void SelectModeHandler::onPress(qreal x, qreal y)
{
    if (!m_hitTestService || !m_selectionManager || !m_dragManager) return;
    
    Element *element = m_hitTestService->hitTest(x, y);
    if (element) {
        // Don't change selection on press - wait to see if it's a click or drag
        // Just prepare for potential drag
        bool started = m_dragManager->startDrag(element, QPointF(x, y));
        if (!started) {
            qDebug() << "WARNING: DragManager::startDrag returned false for" << element->getName();
        }
    } else {
        m_selectionManager->clearSelection();
    }
}

void SelectModeHandler::onMove(qreal x, qreal y)
{
    if (m_dragManager && m_dragManager->isDragging()) {
        m_dragManager->updateDrag(QPointF(x, y));
    }
}

void SelectModeHandler::onRelease(qreal x, qreal y)
{
    Q_UNUSED(x)
    Q_UNUSED(y)
    
    if (m_dragManager && m_dragManager->isDragging()) {
        m_dragManager->endDrag();
    }
}