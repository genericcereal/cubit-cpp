#pragma once
#include <QFrame>  // instead of QWidget
#include <QList>

class Action;
class Canvas;

class ActionsPanel : public QFrame {
    Q_OBJECT
public:
    explicit ActionsPanel(QWidget *parent = nullptr);
    void onActionClicked(Action *action);
    void setCanvas(Canvas *canvas);
    void setModeSelection(const QString &modeName);

private:
    QList<Action*> actions;
    Canvas *canvasRef;
};