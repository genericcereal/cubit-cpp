#ifndef COMPONENTINSTANCE_H
#define COMPONENTINSTANCE_H

#include <QString>

class ComponentVariant;

class ComponentInstance
{
public:
    explicit ComponentInstance(const QString& elementId);
    virtual ~ComponentInstance() = default;

    QString elementId() const;
    ComponentVariant* masterVariant() const;
    void setMasterVariant(ComponentVariant* variant);

protected:
    QString m_elementId;
    ComponentVariant* m_masterVariant = nullptr;
};

#endif // COMPONENTINSTANCE_H