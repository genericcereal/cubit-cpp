#pragma once
#include "IModeHandler.h"
#include "CanvasController.h"
#include <QPointF>
#include <QList>
#include <functional>

class ElementModel;
class SelectionManager;
class CommandHistory;
class Shape;
class CreateDesignElementCommand;

class LineModeHandler : public IModeHandler {
public:
    LineModeHandler(ElementModel* model,
                    SelectionManager* selection,
                    CommandHistory* history,
                    std::function<void(CanvasController::Mode)> setMode);

    void onPress(qreal x, qreal y) override;
    void onMove(qreal x, qreal y) override;
    void onRelease(qreal x, qreal y) override;
    
    // Handle enter key to finish line creation
    void onEnterPressed();

private:
    ElementModel*         m_elementModel;
    SelectionManager*     m_selectionManager;
    CommandHistory*       m_commandHistory;
    std::function<void(CanvasController::Mode)> m_setModeFunc;

    QList<QPointF>        m_joints;
    Shape*                m_currentLine = nullptr;
    CreateDesignElementCommand* m_currentCommand = nullptr;
    bool                  m_isCreatingLine = false;
    QPointF               m_currentMousePos;
    
    void createInitialLine(const QPointF& startPoint);
    void addJointToLine(const QPointF& newPoint);
    void finishLineCreation();
    void updateLineBounds();
};