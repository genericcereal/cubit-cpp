#pragma once
#include "IModeHandler.h"
#include "CanvasController.h"
#include <QPointF>
#include <functional>

class CreationManager;

class TextModeHandler : public IModeHandler {
public:
    TextModeHandler(CreationManager* creationManager,
                    std::function<void(CanvasController::Mode)> setModeFunc);
    
    void onPress(qreal x, qreal y) override;
    void onMove(qreal x, qreal y) override;
    void onRelease(qreal x, qreal y) override;
    
private:
    CreationManager* m_creationManager;
    std::function<void(CanvasController::Mode)> m_setModeFunc;
};