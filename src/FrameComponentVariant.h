#ifndef FRAMECOMPONENTVARIANT_H
#define FRAMECOMPONENTVARIANT_H

#include "Frame.h"
#include <QString>

class FrameComponentVariant : public Frame
{
    Q_OBJECT
    Q_PROPERTY(bool instancesAcceptChildren READ instancesAcceptChildren WRITE setInstancesAcceptChildren NOTIFY instancesAcceptChildrenChanged)
    Q_PROPERTY(QStringList editableProperties READ editableProperties WRITE setEditableProperties NOTIFY editablePropertiesChanged)

public:
    explicit FrameComponentVariant(const QString &id, QObject *parent = nullptr);
    ~FrameComponentVariant();
    
    // Property getters
    bool instancesAcceptChildren() const { return m_instancesAcceptChildren; }
    QStringList editableProperties() const { return m_editableProperties; }
    
    // Property setters
    void setInstancesAcceptChildren(bool accept);
    void setEditableProperties(const QStringList& properties);

signals:
    void instancesAcceptChildrenChanged();
    void editablePropertiesChanged();

private:
    bool m_instancesAcceptChildren;
    QStringList m_editableProperties;
};

#endif // FRAMECOMPONENTVARIANT_H