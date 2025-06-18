#pragma once
#include "IModeHandler.h"
#include <QPointF>
#include <functional>

class CreationManager;
class ElementModel;
class SelectionManager;
class CommandHistory;
class Element;

class FrameModeHandler : public IModeHandler {
public:
    FrameModeHandler(CreationManager* creationManager,
                     ElementModel* elementModel,
                     SelectionManager* selectionManager,
                     CommandHistory* commandHistory,
                     std::function<void(const QString&)> setModeFunc);
    
    void onPress(qreal x, qreal y) override;
    void onMove(qreal x, qreal y) override;
    void onRelease(qreal x, qreal y) override;
    
private:
    CreationManager* m_creationManager;
    ElementModel* m_elementModel;
    SelectionManager* m_selectionManager;
    CommandHistory* m_commandHistory;
    std::function<void(const QString&)> m_setModeFunc;
};