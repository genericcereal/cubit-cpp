#pragma once

#include <QFrame>

class ElementList;

class DetailPanel : public QFrame {
    Q_OBJECT
public:
    explicit DetailPanel(QWidget *parent = nullptr);
    ElementList* getElementList() const { return elementList; }

private:
    ElementList *elementList;
};