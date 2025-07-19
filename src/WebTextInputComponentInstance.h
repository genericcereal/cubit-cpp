#ifndef WEBTEXTINPUTCOMPONENTINSTANCE_H
#define WEBTEXTINPUTCOMPONENTINSTANCE_H

#include "platforms/web/WebTextInput.h"
#include "ComponentInstance.h"
#include <QMetaProperty>
#include "ConnectionManager.h"

class Component;
class WebTextInputComponentVariant;
class Element;
Q_DECLARE_OPAQUE_POINTER(WebTextInputComponentVariant*)

class WebTextInputComponentInstance : public WebTextInput, public ComponentInstance
{
    Q_OBJECT
    Q_PROPERTY(QString instanceOf READ instanceOf WRITE setInstanceOf NOTIFY instanceOfChanged)
    Q_PROPERTY(Element* sourceVariant READ sourceVariant WRITE setSourceVariant NOTIFY sourceVariantChanged)
    
public:
    explicit WebTextInputComponentInstance(const QString &id, QObject *parent = nullptr);
    virtual ~WebTextInputComponentInstance();
    
    // Property getters
    QString instanceOf() const { return m_instanceOf; }
    Element* sourceVariant() const { return m_sourceVariant; }
    
    // Property setters
    void setInstanceOf(const QString &componentId);
    void setSourceVariant(Element* variant);
    
    // Get editable properties from source variant
    Q_INVOKABLE QStringList getEditableProperties() const;
    
    // Get the source component
    Q_INVOKABLE Component* sourceComponent() const { return m_component; }
    
    // Override to identify this as a visual element
    virtual bool isVisual() const override { return true; }
    
    // Override to identify this as a component instance
    virtual bool isComponentInstance() const override { return true; }
    
    // Override to execute scripts (now executes instance's own scripts)
    Q_INVOKABLE virtual void executeScriptEvent(const QString& eventName, const QVariantMap& eventData = QVariantMap()) override;
    
    // Override WebTextInput property setters to track modifications
    void setValue(const QString &value);
    void setPlaceholder(const QString &placeholder);
    void setPosition(PositionType position);
    void setBorderColor(const QColor &color);
    void setBorderWidth(qreal width);
    void setBorderRadius(qreal radius);
    
signals:
    void instanceOfChanged();
    void sourceVariantChanged();
    
private slots:
    void onSourceVariantPropertyChanged();
    void onComponentVariantsChanged();
    
private:
    void connectToComponent();
    void disconnectFromComponent();
    void connectToVariant();
    void disconnectFromVariant();
    void syncPropertiesFromVariant();
    
    // Static property lists for consistency
    static const QStringList s_variantPropertiesToSync;
    
    QString m_instanceOf;  // The ID of the Component this is an instance of
    Component* m_component = nullptr;
    Element* m_sourceVariant = nullptr;
    ConnectionManager m_variantConnections;
    ConnectionManager m_componentConnections;
    QSet<QString> m_modifiedProperties; // Track which properties have been locally modified
};

#endif // WEBTEXTINPUTCOMPONENTINSTANCE_H