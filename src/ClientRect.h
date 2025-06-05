#pragma once
#include <QFrame>
#include <QString>

class Canvas;

class ClientRect : public QFrame {
    Q_OBJECT
public:
    explicit ClientRect(int associatedElementId, Canvas *canvas, QWidget *parent = nullptr);
    
    int getAssociatedElementId() const { return elementId; }

protected:
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;

private:
    int elementId;
    Canvas *canvasRef;
};