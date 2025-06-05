#pragma once
#include <QWidget>

class ActionsPanel;
class Canvas;

class PanelLayer : public QWidget {
    Q_OBJECT
public:
    explicit PanelLayer(QWidget *parent = nullptr);
    void setCanvas(Canvas *canvas);
    ActionsPanel* getActionsPanel() { return actionsPanel; }

protected:
    void resizeEvent(QResizeEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;

private:
    ActionsPanel *actionsPanel;
    Canvas *canvasRef;
};