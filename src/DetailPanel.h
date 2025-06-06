#pragma once

#include <QFrame>

class ElementList;
class Properties;
class Canvas;

class DetailPanel : public QFrame {
    Q_OBJECT
public:
    explicit DetailPanel(QWidget *parent = nullptr);
    ElementList* getElementList() const { return elementList; }
    Properties* getProperties() const { return properties; }
    void setCanvas(Canvas *canvas);

private:
    ElementList *elementList;
    Properties *properties;
};