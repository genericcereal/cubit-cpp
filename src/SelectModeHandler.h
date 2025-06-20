#pragma once
#include "IModeHandler.h"
#include <QPointF>

class HitTestService;
class SelectionManager;

class SelectModeHandler : public IModeHandler {
public:
    SelectModeHandler(HitTestService* hitTestService, 
                      SelectionManager* selectionManager);
    
    void onPress(qreal x, qreal y) override;
    void onMove(qreal x, qreal y) override;
    void onRelease(qreal x, qreal y) override;
    
private:
    HitTestService* m_hitTestService;
    SelectionManager* m_selectionManager;
};