#pragma once
#include "IModeHandler.h"
#include "CanvasController.h"
#include <functional>
#include <QPointF>
#include <QVariant>
#include <QString>

class ElementModel;
class SelectionManager;
class CommandHistory;
class CanvasElement;
class CanvasController;

class CreationModeHandler : public IModeHandler {
public:
    struct Config {
        QString elementType;       // e.g. "frame", "text", "webtextinput"
        QVariant defaultPayload;   // e.g. "Text", "<h1>HTML</h1>", or QVariant()
    };

    CreationModeHandler(Config cfg,
                        ElementModel* model,
                        SelectionManager* selection,
                        CommandHistory* history,
                        std::function<void(CanvasController::Mode)> setMode);

    void onPress(qreal x, qreal y) override;
    void onMove(qreal x, qreal y) override;
    void onRelease(qreal x, qreal y) override;

private:
    Config                m_cfg;
    ElementModel*         m_elementModel;
    SelectionManager*     m_selectionManager;
    CommandHistory*       m_commandHistory;
    std::function<void(CanvasController::Mode)> m_setModeFunc;

    QPointF               m_startPos;
    bool                  m_isDragging = false;

    CanvasElement* currentElement() const;
};