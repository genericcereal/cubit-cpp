#pragma once
#include "Element.h"
#include <QString>

class Frame : public Element {
    Q_OBJECT
public:
    explicit Frame(int id, QWidget *parent = nullptr);
    
    // Override Element virtual methods
    int getId() const override { return frameId; }
    QString getName() const override { return frameName; }

private:
    int frameId;
    QString frameName;
};