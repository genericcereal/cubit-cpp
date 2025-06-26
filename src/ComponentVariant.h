#ifndef COMPONENTVARIANT_H
#define COMPONENTVARIANT_H

#include "Frame.h"
#include <QString>

class ComponentVariant : public Frame
{
    Q_OBJECT
    Q_PROPERTY(bool instancesAcceptChildren READ instancesAcceptChildren WRITE setInstancesAcceptChildren NOTIFY instancesAcceptChildrenChanged)

public:
    explicit ComponentVariant(const QString &id, QObject *parent = nullptr);
    ~ComponentVariant();
    
    // Property getters
    bool instancesAcceptChildren() const { return m_instancesAcceptChildren; }
    
    // Property setters
    void setInstancesAcceptChildren(bool accept);

signals:
    void instancesAcceptChildrenChanged();

private:
    bool m_instancesAcceptChildren;
};

#endif // COMPONENTVARIANT_H