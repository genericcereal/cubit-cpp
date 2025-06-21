#include "SelectModeHandler.h"
#include "HitTestService.h"
#include "SelectionManager.h"
#include "Element.h"
#include <QDebug>

SelectModeHandler::SelectModeHandler(HitTestService* hitTestService, 
                                     SelectionManager* selectionManager)
    : m_hitTestService(hitTestService)
    , m_selectionManager(selectionManager)
{
}

void SelectModeHandler::onPress(qreal x, qreal y)
{
    if (!m_hitTestService || !m_selectionManager) return;
    
    Element *element = m_hitTestService->hitTest(x, y);
    if (element) {
        // Use selectOnly to clear existing selection and select only this element
        m_selectionManager->selectOnly(element);
    } else {
        m_selectionManager->clearSelection();
    }
}

void SelectModeHandler::onMove(qreal x, qreal y)
{
    Q_UNUSED(x)
    Q_UNUSED(y)
    // Mouse move handling is now done in QML
}

void SelectModeHandler::onRelease(qreal x, qreal y)
{
    Q_UNUSED(x)
    Q_UNUSED(y)
    // Mouse release handling is now done in QML
}