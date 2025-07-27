#ifndef COMPONENTINSTANCETEMPLATE_H
#define COMPONENTINSTANCETEMPLATE_H

#include <QObject>
#include <QString>
#include <QStringList>
#include <QHash>
#include <QSet>
#include <QPointer>
#include "ComponentInstance.h"
#include "Frame.h"
#include "Text.h"
#include "Shape.h"
#include "ConnectionManager.h"
#include "platforms/web/WebTextInput.h"

// Forward declarations
class Component;
class Element;
class CanvasElement;

/**
 * Template-based ComponentInstance for Frame elements
 */
class FrameComponentInstanceTemplate : public Frame, public ComponentInstance
{
    Q_OBJECT
    Q_PROPERTY(QString sourceElementId READ sourceElementId WRITE setSourceElementId NOTIFY sourceElementIdChanged)
    Q_PROPERTY(Element* sourceVariant READ sourceVariant NOTIFY sourceVariantChanged)
    Q_PROPERTY(QString componentId READ componentId NOTIFY componentIdChanged)

public:
    explicit FrameComponentInstanceTemplate(const QString &id, QObject *parent = nullptr);
    virtual ~FrameComponentInstanceTemplate();

    // Property getters
    QString sourceElementId() const { return m_sourceElementId; }
    Element* sourceVariant() const { return m_isSourceVariant ? m_sourceElement.data() : nullptr; }
    QString componentId() const { return m_componentId; }
    
    // Property setters
    void setSourceElementId(const QString &elementId);
    
    // Get the source component (only valid if source is a variant)
    Q_INVOKABLE Component* sourceComponent() const;
    
    // Set source variant by element pointer (for variant switching)
    Q_INVOKABLE void setSourceVariant(Element* variant);
    
    // Get editable properties from source element
    Q_INVOKABLE QStringList getEditableProperties() const;
    
    // Override to identify this as a visual element
    bool isVisual() const override { return true; }
    
    // Override to identify this as a component instance
    bool isComponentInstance() const override { return true; }
    
    // Override to execute scripts (now executes instance's own scripts)
    Q_INVOKABLE void executeScriptEvent(const QString& eventName, const QVariantMap& eventData = QVariantMap()) override;

signals:
    void sourceElementIdChanged();
    void sourceVariantChanged();
    void componentIdChanged();

private slots:
    void onSourcePropertyChanged();
    void onSourceGeometryChanged();

private:
    void connectToSourceElement();
    void disconnectFromSourceElement();
    void updateFromSourceElement();
    void updateComponentInfo();
    
    QString m_sourceElementId;
    QPointer<Frame> m_sourceElement;
    bool m_isSourceVariant = false;
    QString m_componentId;
    bool m_isDestroying = false;
};

/**
 * Template-based ComponentInstance for Text elements
 */
class TextComponentInstanceTemplate : public Text, public ComponentInstance
{
    Q_OBJECT
    Q_PROPERTY(QString sourceElementId READ sourceElementId WRITE setSourceElementId NOTIFY sourceElementIdChanged)
    Q_PROPERTY(Element* sourceVariant READ sourceVariant NOTIFY sourceVariantChanged)
    Q_PROPERTY(QString componentId READ componentId NOTIFY componentIdChanged)

public:
    explicit TextComponentInstanceTemplate(const QString &id, QObject *parent = nullptr);
    virtual ~TextComponentInstanceTemplate();

    // Property getters
    QString sourceElementId() const { return m_sourceElementId; }
    Element* sourceVariant() const { return m_isSourceVariant ? m_sourceElement.data() : nullptr; }
    QString componentId() const { return m_componentId; }
    
    // Property setters
    void setSourceElementId(const QString &elementId);
    
    // Get the source component (only valid if source is a variant)
    Q_INVOKABLE Component* sourceComponent() const;
    
    // Set source variant by element pointer (for variant switching)
    Q_INVOKABLE void setSourceVariant(Element* variant);
    
    // Get editable properties from source element
    Q_INVOKABLE QStringList getEditableProperties() const;
    
    // Override to identify this as a visual element
    bool isVisual() const override { return true; }
    
    // Override to identify this as a component instance
    bool isComponentInstance() const override { return true; }
    
    // Override to execute scripts
    Q_INVOKABLE void executeScriptEvent(const QString& eventName, const QVariantMap& eventData = QVariantMap()) override;

signals:
    void sourceElementIdChanged();
    void sourceVariantChanged();
    void componentIdChanged();

private slots:
    void onSourcePropertyChanged();
    void onSourceGeometryChanged();

private:
    void connectToSourceElement();
    void disconnectFromSourceElement();
    void updateFromSourceElement();
    void updateComponentInfo();
    
    QString m_sourceElementId;
    QPointer<Text> m_sourceElement;
    bool m_isSourceVariant = false;
    QString m_componentId;
    bool m_isDestroying = false;
};

/**
 * Template-based ComponentInstance for Shape elements
 */
class ShapeComponentInstanceTemplate : public Shape, public ComponentInstance
{
    Q_OBJECT
    Q_PROPERTY(QString sourceElementId READ sourceElementId WRITE setSourceElementId NOTIFY sourceElementIdChanged)
    Q_PROPERTY(Element* sourceVariant READ sourceVariant NOTIFY sourceVariantChanged)
    Q_PROPERTY(QString componentId READ componentId NOTIFY componentIdChanged)

public:
    explicit ShapeComponentInstanceTemplate(const QString &id, QObject *parent = nullptr);
    virtual ~ShapeComponentInstanceTemplate();

    // Property getters
    QString sourceElementId() const { return m_sourceElementId; }
    Element* sourceVariant() const { return m_isSourceVariant ? m_sourceElement.data() : nullptr; }
    QString componentId() const { return m_componentId; }
    
    // Property setters
    void setSourceElementId(const QString &elementId);
    
    // Get the source component (only valid if source is a variant)
    Q_INVOKABLE Component* sourceComponent() const;
    
    // Set source variant by element pointer (for variant switching)
    Q_INVOKABLE void setSourceVariant(Element* variant);
    
    // Get editable properties from source element
    Q_INVOKABLE QStringList getEditableProperties() const;
    
    // Override to identify this as a visual element
    bool isVisual() const override { return true; }
    
    // Override to identify this as a component instance
    bool isComponentInstance() const override { return true; }
    
    // Override to execute scripts
    Q_INVOKABLE void executeScriptEvent(const QString& eventName, const QVariantMap& eventData = QVariantMap()) override;

signals:
    void sourceElementIdChanged();
    void sourceVariantChanged();
    void componentIdChanged();

private slots:
    void onSourcePropertyChanged();
    void onSourceGeometryChanged();

private:
    void connectToSourceElement();
    void disconnectFromSourceElement();
    void updateFromSourceElement();
    void updateComponentInfo();
    
    QString m_sourceElementId;
    QPointer<Shape> m_sourceElement;
    bool m_isSourceVariant = false;
    QString m_componentId;
    bool m_isDestroying = false;
};

/**
 * Template-based ComponentInstance for WebTextInput elements
 */
class WebTextInputComponentInstanceTemplate : public WebTextInput, public ComponentInstance
{
    Q_OBJECT
    Q_PROPERTY(QString sourceElementId READ sourceElementId WRITE setSourceElementId NOTIFY sourceElementIdChanged)
    Q_PROPERTY(Element* sourceVariant READ sourceVariant NOTIFY sourceVariantChanged)
    Q_PROPERTY(QString componentId READ componentId NOTIFY componentIdChanged)

public:
    explicit WebTextInputComponentInstanceTemplate(const QString &id, QObject *parent = nullptr);
    virtual ~WebTextInputComponentInstanceTemplate();

    // Property getters
    QString sourceElementId() const { return m_sourceElementId; }
    Element* sourceVariant() const { return m_isSourceVariant ? m_sourceElement.data() : nullptr; }
    QString componentId() const { return m_componentId; }
    
    // Property setters
    void setSourceElementId(const QString &elementId);
    
    // Get the source component (only valid if source is a variant)
    Q_INVOKABLE Component* sourceComponent() const;
    
    // Set source variant by element pointer (for variant switching)
    Q_INVOKABLE void setSourceVariant(Element* variant);
    
    // Get editable properties from source element
    Q_INVOKABLE QStringList getEditableProperties() const;
    
    // Override to identify this as a visual element
    bool isVisual() const override { return true; }
    
    // Override to identify this as a component instance
    bool isComponentInstance() const override { return true; }
    
    // Override to execute scripts
    Q_INVOKABLE void executeScriptEvent(const QString& eventName, const QVariantMap& eventData = QVariantMap()) override;

signals:
    void sourceElementIdChanged();
    void sourceVariantChanged();
    void componentIdChanged();

private slots:
    void onSourcePropertyChanged();
    void onSourceGeometryChanged();

private:
    void connectToSourceElement();
    void disconnectFromSourceElement();
    void updateFromSourceElement();
    void updateComponentInfo();
    
    QString m_sourceElementId;
    QPointer<WebTextInput> m_sourceElement;
    bool m_isSourceVariant = false;
    QString m_componentId;
    bool m_isDestroying = false;
};

// Type aliases for easier use
using FrameComponentInstance = FrameComponentInstanceTemplate;
using TextComponentInstance = TextComponentInstanceTemplate;
using ShapeComponentInstance = ShapeComponentInstanceTemplate;
using WebTextInputComponentInstance = WebTextInputComponentInstanceTemplate;

#endif // COMPONENTINSTANCETEMPLATE_H