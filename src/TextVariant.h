#ifndef TEXTVARIANT_H
#define TEXTVARIANT_H

#include "Text.h"
#include "ComponentVariant.h"
#include <QString>

class TextComponentInstance;

class TextVariant : public Text, public ComponentVariant
{
    Q_OBJECT
    Q_PROPERTY(bool instancesAcceptChildren READ instancesAcceptChildren WRITE setInstancesAcceptChildren NOTIFY instancesAcceptChildrenChanged)
    Q_PROPERTY(QStringList editableProperties READ editableProperties WRITE setEditableProperties NOTIFY editablePropertiesChanged)

public:
    explicit TextVariant(const QString &id, QObject *parent = nullptr);
    ~TextVariant();
    
    // Property getters
    bool instancesAcceptChildren() const { return m_instancesAcceptChildren; }
    QStringList editableProperties() const { return m_editableProperties; }
    
    // Property setters
    void setInstancesAcceptChildren(bool accept);
    void setEditableProperties(const QStringList& properties);
    
    // ComponentVariant interface
    virtual void applyToInstance(ComponentInstance* instance) override;
    
    // Override scripts getter to return nullptr for variants
    Scripts* scripts() const override { return nullptr; }
    
    // Override to identify as component variant
    bool isComponentVariant() const override { return true; }
    
    // Override to prevent script execution for variants
    void executeScriptEvent(const QString& eventName) override { Q_UNUSED(eventName); }

signals:
    void instancesAcceptChildrenChanged();
    void editablePropertiesChanged();

private:
    bool m_instancesAcceptChildren;
    QStringList m_editableProperties;
};

#endif // TEXTVARIANT_H