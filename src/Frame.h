#pragma once
#include "Element.h"
#include <QString>
#include <QColor>

class Frame : public Element {
    Q_OBJECT
public:
    explicit Frame(int id, QWidget *parent = nullptr);
    
    // Override Element virtual methods
    int getId() const override { return frameId; }
    QString getName() const override { return frameName; }
    
    // Get frame color
    QColor getColor() const { return QColor(200, 200, 200); }

private:
    int frameId;
    QString frameName;
};