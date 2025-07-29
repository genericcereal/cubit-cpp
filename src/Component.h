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
    Q_PROPERTY(QString componentType READ componentType WRITE setComponentType NOTIFY componentTypeChanged)
    
public:
    explicit Component(const QString &id, QObject *parent = nullptr);
    virtual ~Component();
    
    // Variants management - accepts DesignElements and Variables
    const QList<Element*>& variants() const { return m_variants; }
    void addVariant(Element* variant);
    void removeVariant(Element* variant);
    void clearVariants();
    
    // Type-specific helpers
    void addDesignElement(DesignElement* element);
    void addVariable(Variable* variable);
    
    // Scripts management
    Scripts* scripts() const;
    
    // Component type management
    QString componentType() const { return m_componentType; }
    void setComponentType(const QString &type);
    
    // QML compatibility methods
    Q_INVOKABLE bool isComponentVariant() const { return false; }
    Q_INVOKABLE bool isComponentInstance() const { return false; }
    
signals:
    void variantsChanged();
    void scriptsChanged();
    void componentTypeChanged();
    
private:
    QList<Element*> m_variants;
    std::unique_ptr<Scripts> m_scripts;
    QString m_componentType;
};