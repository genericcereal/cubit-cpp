#pragma once
#include "IModeHandler.h"
#include "CanvasController.h"
#include <QPointF>
#include <functional>

class CreationManager;
class ElementModel;
class SelectionManager;
class CommandHistory;

class HtmlModeHandler : public IModeHandler {
public:
    HtmlModeHandler(ElementModel* elementModel,
                    SelectionManager* selectionManager,
                    CommandHistory* commandHistory,
                    std::function<void(CanvasController::Mode)> setModeFunc);
    
    void onPress(qreal x, qreal y) override;
    void onMove(qreal x, qreal y) override;
    void onRelease(qreal x, qreal y) override;
    
private:
    ElementModel* m_elementModel;
    SelectionManager* m_selectionManager;
    CommandHistory* m_commandHistory;
    std::function<void(CanvasController::Mode)> m_setModeFunc;
    
    // Drag state
    QPointF m_startPos;
    bool m_isDragging = false;
};