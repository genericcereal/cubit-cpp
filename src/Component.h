#pragma once
#include "Element.h"
#include "Scripts.h"
#include <QList>
#include <memory>

class DesignElement;
class Variable;

class Component : public Element
{
    Q_OBJECT
    Q_PROPERTY(QList<Element*> variants READ variants NOTIFY variantsChanged)
    Q_PROPERTY(Scripts* scripts READ scripts NOTIFY scriptsChanged)
    
public:
    explicit Component(const QString &id, QObject *parent = nullptr);
    virtual ~Component();
    
    // Variants management - accepts DesignElements and Variables
    QList<Element*> variants() const { return m_variants; }
    void addVariant(Element* variant);
    void removeVariant(Element* variant);
    void clearVariants();
    
    // Type-specific helpers
    void addDesignElement(DesignElement* element);
    void addVariable(Variable* variable);
    
    // Scripts management
    Scripts* scripts() const;
    
signals:
    void variantsChanged();
    void scriptsChanged();
    
private:
    QList<Element*> m_variants;
    std::unique_ptr<Scripts> m_scripts;
};