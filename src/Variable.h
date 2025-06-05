#pragma once
#include "Element.h"
#include <QString>
#include <QVariant>

class Variable : public Element {
    Q_OBJECT
public:
    explicit Variable(int id, QWidget *parent = nullptr);
    
    // Override Element virtual methods
    int getId() const override { return variableId; }
    QString getName() const override { return variableName; }
    
    void setValue(const QVariant &value);
    QVariant getValue() const { return value; }
    
    QString getType() const;

private:
    int variableId;
    QString variableName;
    QVariant value;
};