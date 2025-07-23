#include "WebTextInputComponentInstance.h"
#include "WebTextInputComponentVariant.h"
#include "Component.h"
#include "Application.h"
#include "Project.h"
#include "ElementModel.h"
#include <QDebug>

// Define the static property list
const QStringList WebTextInputComponentInstance::s_variantPropertiesToSync = {
    "value", "placeholder", "position", "borderColor", "borderWidth", "borderRadius",
    "x", "y", "width", "height",
    "left", "right", "top", "bottom",
    "leftAnchored", "rightAnchored", "topAnchored", "bottomAnchored"
};

WebTextInputComponentInstance::WebTextInputComponentInstance(const QString &id, QObject *parent)
    : WebTextInput(id, parent)
    , ComponentInstance(id)
{
    // Set the element type
    elementType = Element::WebTextInputComponentInstanceType;
    
    // Set the name to "Instance" + last 4 digits of ID
    QString last4 = id.right(4);
    setName("Instance" + last4);
}

WebTextInputComponentInstance::~WebTextInputComponentInstance()
{
    disconnectFromVariant();
    disconnectFromComponent();
}

void WebTextInputComponentInstance::setInstanceOf(const QString &componentId)
{
    if (m_instanceOf == componentId) return;
    
    // Disconnect from old component
    disconnectFromComponent();
    
    m_instanceOf = componentId;
    emit instanceOfChanged();
    
    // Connect to new component
    connectToComponent();
}

void WebTextInputComponentInstance::setSourceVariant(Element* variant)
{
    if (m_sourceVariant == variant) return;
    
    // Disconnect from old variant
    disconnectFromVariant();
    
    m_sourceVariant = variant;
    emit sourceVariantChanged();
    
    // Connect to new variant
    connectToVariant();
}

QStringList WebTextInputComponentInstance::getEditableProperties() const
{
    if (m_sourceVariant) {
        if (WebTextInputComponentVariant* variant = qobject_cast<WebTextInputComponentVariant*>(m_sourceVariant)) {
            return variant->editableProperties();
        }
    }
    return QStringList();
}

void WebTextInputComponentInstance::executeScriptEvent(const QString& eventName, const QVariantMap& eventData)
{
    // Execute instance's own scripts
    WebTextInput::executeScriptEvent(eventName, eventData);
}

void WebTextInputComponentInstance::setValue(const QString &value)
{
    m_modifiedProperties.insert("value");
    WebTextInput::setValue(value);
}

void WebTextInputComponentInstance::setPlaceholder(const QString &placeholder)
{
    m_modifiedProperties.insert("placeholder");
    WebTextInput::setPlaceholder(placeholder);
}

void WebTextInputComponentInstance::setPosition(PositionType position)
{
    m_modifiedProperties.insert("position");
    WebTextInput::setPosition(position);
}

void WebTextInputComponentInstance::setBorderColor(const QColor &color)
{
    m_modifiedProperties.insert("borderColor");
    WebTextInput::setBorderColor(color);
}

void WebTextInputComponentInstance::setBorderWidth(qreal width)
{
    m_modifiedProperties.insert("borderWidth");
    WebTextInput::setBorderWidth(width);
}

void WebTextInputComponentInstance::setBorderRadius(qreal radius)
{
    m_modifiedProperties.insert("borderRadius");
    WebTextInput::setBorderRadius(radius);
}

void WebTextInputComponentInstance::onSourceVariantPropertyChanged()
{
    const QMetaObject* senderMeta = sender()->metaObject();
    int signalIndex = senderSignalIndex();
    
    if (signalIndex >= 0 && signalIndex < senderMeta->methodCount()) {
        QMetaMethod signal = senderMeta->method(signalIndex);
        QString signalName = signal.name();
        
        // Remove "Changed" suffix to get property name
        if (signalName.endsWith("Changed")) {
            QString propertyName = signalName.left(signalName.length() - 7);
            
            // Only sync if this property hasn't been locally modified
            if (!m_modifiedProperties.contains(propertyName) && 
                s_variantPropertiesToSync.contains(propertyName)) {
                syncPropertiesFromVariant();
            }
        }
    }
}

void WebTextInputComponentInstance::onComponentVariantsChanged()
{
    // Re-sync with variant when component's variants change
    if (m_sourceVariant) {
        syncPropertiesFromVariant();
    }
}

void WebTextInputComponentInstance::connectToComponent()
{
    if (m_instanceOf.isEmpty()) return;
    
    // Get ElementModel from parent chain
    ElementModel* model = nullptr;
    QObject* p = parent();
    while (p && !model) {
        model = qobject_cast<ElementModel*>(p);
        p = p->parent();
    }
    if (!model) return;
    
    Element* element = model->getElementById(m_instanceOf);
    m_component = qobject_cast<Component*>(element);
    
    if (m_component) {
        QMetaObject::Connection conn = connect(m_component, &Component::variantsChanged,
                                              this, &WebTextInputComponentInstance::onComponentVariantsChanged);
        m_componentConnections.add(conn);
    }
}

void WebTextInputComponentInstance::disconnectFromComponent()
{
    m_componentConnections.clear();
    m_component = nullptr;
}

void WebTextInputComponentInstance::connectToVariant()
{
    if (!m_sourceVariant) return;
    
    WebTextInputComponentVariant* variant = qobject_cast<WebTextInputComponentVariant*>(m_sourceVariant);
    if (!variant) return;
    
    // Connect to all property change signals
    for (const QString& propName : s_variantPropertiesToSync) {
        QString signalName = propName + "Changed";
        QByteArray signalSignature = QMetaObject::normalizedSignature((signalName + "()").toUtf8());
        
        int signalIndex = variant->metaObject()->indexOfSignal(signalSignature);
        if (signalIndex >= 0) {
            QMetaMethod signal = variant->metaObject()->method(signalIndex);
            QMetaMethod slot = metaObject()->method(
                metaObject()->indexOfSlot("onSourceVariantPropertyChanged()"));
            
            QMetaObject::Connection conn = connect(variant, signal, this, slot);
            m_variantConnections.add(conn);
        }
    }
    
    // Initial sync
    syncPropertiesFromVariant();
}

void WebTextInputComponentInstance::disconnectFromVariant()
{
    m_variantConnections.clear();
}

void WebTextInputComponentInstance::syncPropertiesFromVariant()
{
    if (!m_sourceVariant) return;
    
    WebTextInputComponentVariant* variant = qobject_cast<WebTextInputComponentVariant*>(m_sourceVariant);
    if (!variant) return;
    
    // Sync properties that haven't been locally modified
    if (!m_modifiedProperties.contains("value")) {
        WebTextInput::setValue(variant->value());
    }
    // WebTextInput doesn't have font/color properties
    if (!m_modifiedProperties.contains("placeholder")) {
        WebTextInput::setPlaceholder(variant->placeholder());
    }
    if (!m_modifiedProperties.contains("position")) {
        WebTextInput::setPosition(variant->position());
    }
    if (!m_modifiedProperties.contains("borderColor")) {
        WebTextInput::setBorderColor(variant->borderColor());
    }
    if (!m_modifiedProperties.contains("borderWidth")) {
        WebTextInput::setBorderWidth(variant->borderWidth());
    }
    if (!m_modifiedProperties.contains("borderRadius")) {
        WebTextInput::setBorderRadius(variant->borderRadius());
    }
    
    // Always sync geometry and anchors
    setRect(variant->rect());
    setLeft(variant->left());
    setRight(variant->right());
    setTop(variant->top());
    setBottom(variant->bottom());
    setLeftAnchored(variant->leftAnchored());
    setRightAnchored(variant->rightAnchored());
    setTopAnchored(variant->topAnchored());
    setBottomAnchored(variant->bottomAnchored());
}