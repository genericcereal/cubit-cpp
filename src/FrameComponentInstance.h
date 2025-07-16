#pragma once
#include "Frame.h"
#include "ComponentInstance.h"
#include <QMetaProperty>
#include "ConnectionManager.h"

class Component;
class FrameComponentVariant;
class Element;
Q_DECLARE_OPAQUE_POINTER(FrameComponentVariant*)
Q_DECLARE_OPAQUE_POINTER(Element*)

class FrameComponentInstance : public Frame, public ComponentInstance
{
    Q_OBJECT
    Q_PROPERTY(QString instanceOf READ instanceOf WRITE setInstanceOf NOTIFY instanceOfChanged)
    Q_PROPERTY(Element* sourceVariant READ sourceVariant WRITE setSourceVariant NOTIFY sourceVariantChanged)
    
public:
    explicit FrameComponentInstance(const QString &id, QObject *parent = nullptr);
    virtual ~FrameComponentInstance();
    
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
    
    // Override to execute scripts (now executes instance's own scripts)
    Q_INVOKABLE virtual void executeScriptEvent(const QString& eventName, const QVariantMap& eventData = QVariantMap()) override;
    
    // Override Frame property setters to track modifications
    void setFill(const QColor &color);
    void setBorderColor(const QColor &color);
    void setBorderWidth(int width);
    void setBorderRadius(int radius);
    void setOverflow(OverflowMode mode);
    void setFlex(bool flex);
    void setOrientation(LayoutOrientation orientation);
    void setGap(qreal gap);
    void setPosition(PositionType position);
    void setJustify(JustifyContent justify);
    void setAlign(AlignItems align);
    void setWidthType(SizeType type);
    void setHeightType(SizeType type);
    
signals:
    void instanceOfChanged();
    void sourceVariantChanged();
    
private slots:
    void onSourceVariantPropertyChanged();
    void onComponentVariantsChanged();
    void onVariantChildAdded(Element* child);
    void onVariantChildRemoved(Element* child);
    void onInstanceChildPropertyChanged();
    void onElementAdded(Element* element);
    void onElementParentChanged();
    
private:
    void connectToComponent();
    void disconnectFromComponent();
    void connectToVariant();
    void disconnectFromVariant();
    void syncPropertiesFromVariant();
    void createChildInstances();
    void clearChildInstances();
    CanvasElement* createInstanceOfElement(CanvasElement* sourceElement, CanvasElement* parent);
    void syncElementProperties(CanvasElement* target, CanvasElement* source);
    void connectToSourceElement(CanvasElement* instanceElement, CanvasElement* sourceElement);
    
    // Static property lists for consistency
    static const QStringList s_variantPropertiesToSync;
    static const QStringList s_childPropertiesToTrack;
    
    QString m_instanceOf;  // The ID of the Component this is an instance of
    Component* m_component = nullptr;
    Element* m_sourceVariant = nullptr;
    ConnectionManager m_variantConnections;
    ConnectionManager m_componentConnections;
    QHash<QString, CanvasElement*> m_childInstances; // Maps source element ID to instance
    QHash<CanvasElement*, ConnectionManager> m_childConnections; // Connections per child
    bool m_isDestructing = false; // Flag to indicate we're in the destructor
    QSet<QString> m_modifiedProperties; // Track which properties have been locally modified
};