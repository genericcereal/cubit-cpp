#ifndef COMPONENTVARIANT_H
#define COMPONENTVARIANT_H

#include <QString>
#include <QStringList>
#include <QList>

class ComponentInstance;

class ComponentVariant
{
public:
    explicit ComponentVariant(const QString& id);
    virtual ~ComponentVariant() = default;

    QString id() const { return m_id; }
    QString variantName() const { return m_variantName; }
    void setVariantName(const QString& name) { m_variantName = name; }
    QStringList editableProperties() const { return m_editableProperties; }
    void setEditableProperties(const QStringList& props) { m_editableProperties = props; }
    bool instancesAcceptChildren() const { return m_instancesAcceptChildren; }
    void setInstancesAcceptChildren(bool ok) { m_instancesAcceptChildren = ok; }
    
    const QList<ComponentInstance*>& instances() const { return m_instances; }
    void addInstance(ComponentInstance* instance);
    void removeInstance(ComponentInstance* instance);
    
    // Implement in derived class to push your per-type properties
    virtual void applyToInstance(ComponentInstance* instance) = 0;

protected:
    QString m_id;
    QString m_variantName;
    QStringList m_editableProperties;
    bool m_instancesAcceptChildren = true;
    QList<ComponentInstance*> m_instances;
};

#endif // COMPONENTVARIANT_H