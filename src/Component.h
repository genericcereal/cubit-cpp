#pragma once
#include "Element.h"
#include "Scripts.h"
#include <QList>
#include <memory>

class Frame;

class Component : public Element
{
    Q_OBJECT
    Q_PROPERTY(QList<Frame*> variants READ variants NOTIFY variantsChanged)
    Q_PROPERTY(Scripts* scripts READ scripts NOTIFY scriptsChanged)
    
public:
    explicit Component(const QString &id, QObject *parent = nullptr);
    virtual ~Component();
    
    // Variants management
    QList<Frame*> variants() const { return m_variants; }
    void addVariant(Frame* variant);
    void removeVariant(Frame* variant);
    void clearVariants();
    
    // Scripts management
    Scripts* scripts() const;
    
signals:
    void variantsChanged();
    void scriptsChanged();
    
private:
    QList<Frame*> m_variants;
    std::unique_ptr<Scripts> m_scripts;
};