#ifndef SHAPECOMPONENTINSTANCE_H
#define SHAPECOMPONENTINSTANCE_H

#include "Shape.h"
#include "ComponentInstance.h"
#include <QMetaProperty>
#include "ConnectionManager.h"

class Component;
class Element;

// Forward declaration only - full definition in cpp file
class ShapeComponentVariant;
Q_DECLARE_OPAQUE_POINTER(ShapeComponentVariant*)

class ShapeComponentInstance : public Shape, public ComponentInstance
{
    Q_OBJECT
    Q_PROPERTY(QString instanceOf READ instanceOf WRITE setInstanceOf NOTIFY instanceOfChanged)
    Q_PROPERTY(Element* sourceVariant READ sourceVariant WRITE setSourceVariant NOTIFY sourceVariantChanged)
    
public:
    explicit ShapeComponentInstance(const QString &id, QObject *parent = nullptr);
    virtual ~ShapeComponentInstance();
    
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
    
    // Override Shape property setters to track modifications
    void setShapeType(ShapeType type);
    void setEdgeWidth(qreal width);
    void setEdgeColor(const QColor &color);
    void setFillColor(const QColor &color);
    
signals:
    void instanceOfChanged();
    void sourceVariantChanged();
    
protected:
    // Override to track property modifications
    bool setProperty(const char *name, const QVariant &value);
    
private:
    QString m_instanceOf;
    Component* m_component = nullptr;
    Element* m_sourceVariant = nullptr;
    ConnectionManager m_connections;
    
    // Track which properties have been overridden
    QSet<QString> m_overriddenProperties;
    
    void connectToVariant();
    void updateFromVariant();
    void onVariantPropertyChanged();
};

#endif // SHAPECOMPONENTINSTANCE_H