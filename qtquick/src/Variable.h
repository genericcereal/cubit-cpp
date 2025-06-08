#pragma once
#include "Element.h"
#include <QVariant>

class Variable : public Element {
    Q_OBJECT
    Q_PROPERTY(QVariant value READ value WRITE setValue NOTIFY valueChanged)
    
public:
    explicit Variable(int id, QObject *parent = nullptr);
    
    QVariant value() const { return m_value; }
    void setValue(const QVariant &value);
    
signals:
    void valueChanged();
    
private:
    QVariant m_value;
};