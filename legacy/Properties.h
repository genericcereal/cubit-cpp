#pragma once
#include <QWidget>

class Canvas;
class Property;

class Properties : public QWidget {
    Q_OBJECT
public:
    explicit Properties(QWidget *parent = nullptr);
    void setCanvas(Canvas *canvas);
    
public slots:
    void updateFromSelection();
    
private:
    Canvas *canvasRef;
    Property *nameProperty;
    Property *positionProperty;
    Property *sizeProperty;
    Property *rotationProperty;
    Property *overflowProperty;
};