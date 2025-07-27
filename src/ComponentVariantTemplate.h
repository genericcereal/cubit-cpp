#ifndef COMPONENTVARIANTTEMPLATE_H
#define COMPONENTVARIANTTEMPLATE_H

#include <QObject>
#include <QString>
#include <QStringList>
#include "ComponentVariant.h"
#include "Frame.h"
#include "Text.h"
#include "Shape.h"
#include "platforms/web/WebTextInput.h"

// Forward declarations
class ComponentInstance;

/**
 * Template-based ComponentVariant for Frame elements
 */
class FrameComponentVariantTemplate : public Frame, public ComponentVariant
{
    Q_OBJECT
    Q_PROPERTY(bool instancesAcceptChildren READ instancesAcceptChildren WRITE setInstancesAcceptChildren NOTIFY instancesAcceptChildrenChanged)
    Q_PROPERTY(QStringList editableProperties READ editableProperties WRITE setEditableProperties NOTIFY editablePropertiesChanged)
    Q_PROPERTY(QString variantName READ variantName WRITE setVariantName NOTIFY variantNameChanged)

public:
    explicit FrameComponentVariantTemplate(const QString &id, QObject *parent = nullptr);
    virtual ~FrameComponentVariantTemplate() = default;

    // Property getters
    bool instancesAcceptChildren() const { return ComponentVariant::instancesAcceptChildren(); }
    QStringList editableProperties() const { return ComponentVariant::editableProperties(); }
    QString variantName() const { return ComponentVariant::variantName(); }
    
    // Property setters
    void setInstancesAcceptChildren(bool accept);
    void setEditableProperties(const QStringList& properties);
    void setVariantName(const QString& name);
    
    // ComponentVariant interface
    void applyToInstance(ComponentInstance* instance) override;
    ComponentVariant* clone(const QString& newId) const override;
    
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

/**
 * Template-based ComponentVariant for Text elements
 */
class TextComponentVariantTemplate : public Text, public ComponentVariant
{
    Q_OBJECT
    Q_PROPERTY(bool instancesAcceptChildren READ instancesAcceptChildren WRITE setInstancesAcceptChildren NOTIFY instancesAcceptChildrenChanged)
    Q_PROPERTY(QStringList editableProperties READ editableProperties WRITE setEditableProperties NOTIFY editablePropertiesChanged)
    Q_PROPERTY(QString variantName READ variantName WRITE setVariantName NOTIFY variantNameChanged)

public:
    explicit TextComponentVariantTemplate(const QString &id, QObject *parent = nullptr);
    virtual ~TextComponentVariantTemplate() = default;

    // Property getters
    bool instancesAcceptChildren() const { return ComponentVariant::instancesAcceptChildren(); }
    QStringList editableProperties() const { return ComponentVariant::editableProperties(); }
    QString variantName() const { return ComponentVariant::variantName(); }
    
    // Property setters
    void setInstancesAcceptChildren(bool accept);
    void setEditableProperties(const QStringList& properties);
    void setVariantName(const QString& name);
    
    // ComponentVariant interface
    void applyToInstance(ComponentInstance* instance) override;
    ComponentVariant* clone(const QString& newId) const override;
    
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

/**
 * Template-based ComponentVariant for Shape elements
 */
class ShapeComponentVariantTemplate : public Shape, public ComponentVariant
{
    Q_OBJECT
    Q_PROPERTY(bool instancesAcceptChildren READ instancesAcceptChildren WRITE setInstancesAcceptChildren NOTIFY instancesAcceptChildrenChanged)
    Q_PROPERTY(QStringList editableProperties READ editableProperties WRITE setEditableProperties NOTIFY editablePropertiesChanged)
    Q_PROPERTY(QString variantName READ variantName WRITE setVariantName NOTIFY variantNameChanged)

public:
    explicit ShapeComponentVariantTemplate(const QString &id, QObject *parent = nullptr);
    virtual ~ShapeComponentVariantTemplate() = default;

    // Property getters
    bool instancesAcceptChildren() const { return ComponentVariant::instancesAcceptChildren(); }
    QStringList editableProperties() const { return ComponentVariant::editableProperties(); }
    QString variantName() const { return ComponentVariant::variantName(); }
    
    // Property setters
    void setInstancesAcceptChildren(bool accept);
    void setEditableProperties(const QStringList& properties);
    void setVariantName(const QString& name);
    
    // ComponentVariant interface
    void applyToInstance(ComponentInstance* instance) override;
    ComponentVariant* clone(const QString& newId) const override;
    
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

/**
 * Template-based ComponentVariant for WebTextInput elements
 */
class WebTextInputComponentVariantTemplate : public WebTextInput, public ComponentVariant
{
    Q_OBJECT
    Q_PROPERTY(bool instancesAcceptChildren READ instancesAcceptChildren WRITE setInstancesAcceptChildren NOTIFY instancesAcceptChildrenChanged)
    Q_PROPERTY(QStringList editableProperties READ editableProperties WRITE setEditableProperties NOTIFY editablePropertiesChanged)
    Q_PROPERTY(QString variantName READ variantName WRITE setVariantName NOTIFY variantNameChanged)

public:
    explicit WebTextInputComponentVariantTemplate(const QString &id, QObject *parent = nullptr);
    virtual ~WebTextInputComponentVariantTemplate() = default;

    // Property getters
    bool instancesAcceptChildren() const { return ComponentVariant::instancesAcceptChildren(); }
    QStringList editableProperties() const { return ComponentVariant::editableProperties(); }
    QString variantName() const { return ComponentVariant::variantName(); }
    
    // Property setters
    void setInstancesAcceptChildren(bool accept);
    void setEditableProperties(const QStringList& properties);
    void setVariantName(const QString& name);
    
    // ComponentVariant interface
    void applyToInstance(ComponentInstance* instance) override;
    ComponentVariant* clone(const QString& newId) const override;
    
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

// Type aliases for easier use
using FrameComponentVariant = FrameComponentVariantTemplate;
using TextComponentVariant = TextComponentVariantTemplate;
using ShapeComponentVariant = ShapeComponentVariantTemplate;
using WebTextInputComponentVariant = WebTextInputComponentVariantTemplate;

#endif // COMPONENTVARIANTTEMPLATE_H