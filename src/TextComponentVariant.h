#ifndef TEXTCOMPONENTVARIANT_H
#define TEXTCOMPONENTVARIANT_H

#include "Text.h"
#include "ComponentVariant.h"
#include <QString>

class TextComponentInstance;

class TextComponentVariant : public Text, public ComponentVariant
{
    Q_OBJECT
    Q_PROPERTY(bool instancesAcceptChildren READ instancesAcceptChildren WRITE setInstancesAcceptChildren NOTIFY instancesAcceptChildrenChanged)
    Q_PROPERTY(QStringList editableProperties READ editableProperties WRITE setEditableProperties NOTIFY editablePropertiesChanged)
    Q_PROPERTY(QString variantName READ variantName WRITE setVariantName NOTIFY variantNameChanged)

public:
    explicit TextComponentVariant(const QString &id, QObject *parent = nullptr);
    ~TextComponentVariant();
    
    // Property getters
    bool instancesAcceptChildren() const { return ComponentVariant::instancesAcceptChildren(); }
    QStringList editableProperties() const { return ComponentVariant::editableProperties(); }
    QString variantName() const { return ComponentVariant::variantName(); }
    
    // Property setters
    void setInstancesAcceptChildren(bool accept);
    void setEditableProperties(const QStringList& properties);
    void setVariantName(const QString& name);
    
    // ComponentVariant interface
    virtual void applyToInstance(ComponentInstance* instance) override;
    virtual ComponentVariant* clone(const QString& newId) const override;
    
    // Override scripts getter to return nullptr for variants
    Scripts* scripts() const override { return nullptr; }
    
    // Override to identify as component variant
    bool isComponentVariant() const override { return true; }
    
    // Override to prevent script execution for variants
    void executeScriptEvent(const QString& eventName, const QVariantMap& eventData = QVariantMap()) override { 
        Q_UNUSED(eventName); 
        Q_UNUSED(eventData); 
    }

signals:
    void instancesAcceptChildrenChanged();
    void editablePropertiesChanged();
    void variantNameChanged();
};

#endif // TEXTCOMPONENTVARIANT_H