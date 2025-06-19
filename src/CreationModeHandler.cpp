#include "CreationModeHandler.h"
#include "ElementModel.h"
#include "SelectionManager.h"
#include "CommandHistory.h"
#include "CanvasElement.h"
#include "CanvasController.h"
#include "Config.h"
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
    if (!m_commandHistory || !m_elementModel || !m_selectionManager) return;
    
    // Store the start position for drag operation
    m_startPos = QPointF(x, y);
    m_isDragging = true;
    
    // Create element immediately at 1x1 size
    QRectF rect(x, y, 1, 1);
    auto command = std::make_unique<CreateDesignElementCommand>(
        m_elementModel, m_selectionManager, 
        m_cfg.elementType, rect, m_cfg.defaultPayload);
    m_commandHistory->execute(std::move(command));
    
    // The element is now created and selected, ready for resize
}

void CreationModeHandler::onMove(qreal x, qreal y)
{
    if (!m_isDragging || !m_selectionManager) return;
    
    CanvasElement* element = currentElement();
    if (!element) return;
    
    // Calculate new dimensions based on drag
    qreal width = qAbs(x - m_startPos.x());
    qreal height = qAbs(y - m_startPos.y());
    qreal left = qMin(x, m_startPos.x());
    qreal top = qMin(y, m_startPos.y());
    
    // Update element size and position directly (resize command will be created on release)
    element->setX(left);
    element->setY(top);
    element->setWidth(qMax(width, 1.0));
    element->setHeight(qMax(height, 1.0));
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