#pragma once
#include "Element.h"
#include "Scripts.h"
#include <QList>
#include <memory>

class ComponentVariant;

class Component : public Element
{
    Q_OBJECT
    Q_PROPERTY(QList<ComponentVariant*> variants READ variants NOTIFY variantsChanged)
    Q_PROPERTY(Scripts* scripts READ scripts NOTIFY scriptsChanged)
    
public:
    explicit Component(const QString &id, QObject *parent = nullptr);
    virtual ~Component();
    
    // Variants management
    QList<ComponentVariant*> variants() const { return m_variants; }
    void addVariant(ComponentVariant* variant);
    void removeVariant(ComponentVariant* variant);
    void clearVariants();
    
    // Scripts management
    Scripts* scripts() const;
    
signals:
    void variantsChanged();
    void scriptsChanged();
    
private:
    QList<ComponentVariant*> m_variants;
    std::unique_ptr<Scripts> m_scripts;
};