#ifndef COMPONENTVARIANT_H
#define COMPONENTVARIANT_H

#include <QString>
#include <QList>

class ComponentInstance;

class ComponentVariant
{
public:
    explicit ComponentVariant(const QString& id);
    virtual ~ComponentVariant() = default;

    QString id() const;
    QString variantName() const;
    void setVariantName(const QString& name);
    
    const QList<ComponentInstance*>& instances() const;
    void addInstance(ComponentInstance* instance);
    void removeInstance(ComponentInstance* instance);
    
    virtual void applyToInstance(ComponentInstance* instance) = 0;

protected:
    QString m_id;
    QString m_variantName;
    QList<ComponentInstance*> m_instances;
};

#endif // COMPONENTVARIANT_H