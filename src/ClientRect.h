#pragma once
#include <QFrame>
#include <QString>

class CanvasBase;

class ClientRect : public QFrame {
    Q_OBJECT
public:
    explicit ClientRect(int associatedElementId, CanvasBase *canvas, QWidget *parent = nullptr);
    
    int getAssociatedElementId() const { return elementId; }

protected:
    void mousePressEvent(QMouseEvent *event) override;

private:
    int elementId;
    CanvasBase *canvasRef;
};