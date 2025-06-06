#pragma once
#include "Element.h"
#include <QString>
#include <QVariant>

class Variable : public Element {
    Q_OBJECT
public:
    explicit Variable(int id, QWidget *parent = nullptr);
    
    // Override Element virtual methods
    int getId() const override { return elementId; }
    QString getName() const override { return variableName; }
    
    void setValue(const QVariant &value);
    QVariant getValue() const { return value; }
    
    QString getType() const;
    
    // Override visual update
    void updateParentVisualState() override;

private:
    QString variableName;
    QVariant value;
};