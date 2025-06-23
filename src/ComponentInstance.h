#pragma once
#include "Frame.h"
#include <QMetaProperty>

class Component;
class ComponentVariant;
Q_DECLARE_OPAQUE_POINTER(ComponentVariant*)

class ComponentInstance : public Frame
{
    Q_OBJECT
    Q_PROPERTY(QString instanceOf READ instanceOf WRITE setInstanceOf NOTIFY instanceOfChanged)
    Q_PROPERTY(ComponentVariant* sourceVariant READ sourceVariant NOTIFY sourceVariantChanged)
    
public:
    explicit ComponentInstance(const QString &id, QObject *parent = nullptr);
    virtual ~ComponentInstance();
    
    // Property getters
    QString instanceOf() const { return m_instanceOf; }
    ComponentVariant* sourceVariant() const { return m_sourceVariant; }
    
    // Property setters
    void setInstanceOf(const QString &componentId);
    
    // Override to identify this as a visual element
    virtual bool isVisual() const override { return true; }
    
    // Override size setters to propagate changes to the variant
    virtual void setWidth(qreal width) override;
    virtual void setHeight(qreal height) override;
    virtual void setRect(const QRectF &rect) override;
    
signals:
    void instanceOfChanged();
    void sourceVariantChanged();
    
private slots:
    void onSourceVariantPropertyChanged();
    void onComponentVariantsChanged();
    void onVariantChildAdded();
    void onVariantChildRemoved();
    void onInstanceChildPropertyChanged();
    
private:
    void connectToComponent();
    void disconnectFromComponent();
    void connectToVariant();
    void disconnectFromVariant();
    void syncPropertiesFromVariant();
    void createChildInstances();
    void clearChildInstances();
    void updateChildInstancesForResize();
    CanvasElement* createInstanceOfElement(CanvasElement* sourceElement, CanvasElement* parent);
    void syncElementProperties(CanvasElement* target, CanvasElement* source);
    void connectToSourceElement(CanvasElement* instanceElement, CanvasElement* sourceElement);
    
    QString m_instanceOf;  // The ID of the Component this is an instance of
    Component* m_component = nullptr;
    ComponentVariant* m_sourceVariant = nullptr;
    QList<QMetaObject::Connection> m_variantConnections;
    QList<QMetaObject::Connection> m_componentConnections;
    QHash<QString, CanvasElement*> m_childInstances; // Maps source element ID to instance
    QHash<CanvasElement*, QList<QMetaObject::Connection>> m_childConnections; // Connections per child
    bool m_isDestructing = false; // Flag to indicate we're in the destructor
};