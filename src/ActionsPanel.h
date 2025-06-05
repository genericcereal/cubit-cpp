#pragma once
#include <QFrame>  // instead of QWidget
#include <QList>

class Action;
class CanvasBase;
class Canvas;
class GLCanvas;

class ActionsPanel : public QFrame {
    Q_OBJECT
public:
    explicit ActionsPanel(QWidget *parent = nullptr);
    void onActionClicked(Action *action);
    void setCanvas(Canvas *canvas);
    void setGLCanvas(GLCanvas *canvas);
    void setModeSelection(const QString &modeName);

private:
    QList<Action*> actions;
    Canvas *cpuCanvasRef;
    GLCanvas *glCanvasRef;
};