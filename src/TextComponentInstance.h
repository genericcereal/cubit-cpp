#pragma once
#include "Text.h"
#include "ComponentInstance.h"
#include <QMetaProperty>
#include "ConnectionManager.h"

class Component;
class TextComponentVariant;
class Element;

class TextComponentInstance : public Text, public ComponentInstance
{
    Q_OBJECT
    Q_PROPERTY(QString instanceOf READ instanceOf WRITE setInstanceOf NOTIFY instanceOfChanged)
    Q_PROPERTY(Element* sourceVariant READ sourceVariant WRITE setSourceVariant NOTIFY sourceVariantChanged)
    
public:
    explicit TextComponentInstance(const QString &id, QObject *parent = nullptr);
    virtual ~TextComponentInstance();
    
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
    
    // Override Text property setters to track modifications
    void setContent(const QString &content);
    void setFont(const QFont &font);
    void setColor(const QColor &color);
    void setPosition(PositionType position);
    
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
    
    QString m_instanceOf;  // The ID of the Component this is an instance of
    Component* m_component = nullptr;
    Element* m_sourceVariant = nullptr;
    ConnectionManager m_variantConnections;
    ConnectionManager m_componentConnections;
    bool m_isDestructing = false; // Flag to indicate we're in the destructor
    QSet<QString> m_modifiedProperties; // Track which properties have been locally modified
};