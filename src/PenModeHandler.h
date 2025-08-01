#pragma once
#include "IModeHandler.h"
#include "CanvasController.h"
#include "Shape.h"
#include <QPointF>
#include <QList>
#include <functional>

class ElementModel;
class SelectionManager;
class CommandHistory;
class CreateDesignElementCommand;
class ShapeControlsController;

class PenModeHandler : public IModeHandler {
public:
    PenModeHandler(ElementModel* model,
                    SelectionManager* selection,
                    CommandHistory* history,
                    std::function<void(CanvasController::Mode)> setMode,
                    ShapeControlsController* shapeControlsController = nullptr);

    void onPress(qreal x, qreal y) override;
    void onMove(qreal x, qreal y) override;
    void onRelease(qreal x, qreal y) override;
    
    // Handle enter key to finish pen creation
    void onEnterPressed();

private:
    ElementModel*         m_elementModel;
    SelectionManager*     m_selectionManager;
    CommandHistory*       m_commandHistory;
    std::function<void(CanvasController::Mode)> m_setModeFunc;
    ShapeControlsController* m_shapeControlsController;

    QList<Shape::Joint>   m_joints;
    Shape*                m_currentPen = nullptr;
    CreateDesignElementCommand* m_currentCommand = nullptr;
    bool                  m_isCreatingPen = false;
    bool                  m_startNewPath = false;
    QPointF               m_currentMousePos;
    
    void createInitialPen(const QPointF& startPoint);
    void addJointToPen(const QPointF& newPoint);
    void finishPenCreation();
    void updatePenBounds();
};