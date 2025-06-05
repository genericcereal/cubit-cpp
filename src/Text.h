#pragma once
#include "Element.h"
#include <QString>

class Text : public Element {
    Q_OBJECT
public:
    explicit Text(int id, QWidget *parent = nullptr);
    
    // Override Element virtual methods
    int getId() const override { return textId; }
    QString getName() const override { return textName; }
    
    void setText(const QString &text);
    QString getText() const { return content; }

private:
    int textId;
    QString textName;
    QString content;
};