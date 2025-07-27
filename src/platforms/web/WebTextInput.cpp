#include "WebTextInput.h"
#include "Scripts.h"
#include "Node.h"
#include "UniqueIdGenerator.h"
#include "Frame.h"
#include "PropertyRegistry.h"

WebTextInput::WebTextInput(const QString &id, QObject *parent)
    : DesignElement(id, parent)
{
    // Set element type
    elementType = Element::WebTextInputType;
    
    setName(QString("Input %1").arg(id.right(4)));  // Use last 4 digits for display
    
    // Add custom event nodes for WebTextInput
    // Note: These node definitions are also in NodeCatalog.qml
    if (scripts()) {
        // First, find the onEditorLoad node to get its position
        Node* onEditorLoadNode = nullptr;
        for (Node* node : scripts()->getAllNodes()) {
            if (node->nodeTitle() == "On Editor Load") {
                onEditorLoadNode = node;
                break;
            }
        }
        
        qreal baseY = onEditorLoadNode ? onEditorLoadNode->y() : 0;
        
        // Helper lambda to create event nodes with consistent configuration
        auto createEventNode = [this](const QString& title, qreal x, qreal y) -> Node* {
            QString nodeId = UniqueIdGenerator::generate16DigitId();
            Node* node = new Node(nodeId, scripts());
            node->setNodeTitle(title);
            node->setNodeType("Event");
            node->setX(x);
            node->setY(y);
            node->setWidth(150);
            node->setHeight(100);  // Height for two rows
            
            // First row: Done (Flow)
            Node::RowConfig flowConfig;
            flowConfig.hasSource = true;
            flowConfig.sourceLabel = "Done";
            flowConfig.sourceType = "Flow";
            flowConfig.sourcePortIndex = 0;
            node->addRow(flowConfig);
            
            // Second row: Value (String)
            Node::RowConfig valueConfig;
            valueConfig.hasSource = true;
            valueConfig.sourceLabel = "Value";
            valueConfig.sourceType = "String";
            valueConfig.sourcePortIndex = 1;
            node->addRow(valueConfig);
            
            // Add output ports
            node->addOutputPort("done");
            node->setOutputPortType(0, "Flow");
            node->addOutputPort("value");
            node->setOutputPortType(1, "String");
            
            return node;
        };
        
        // Create event nodes based on NodeCatalog.qml definitions
        Node* onChangeNode = createEventNode("On Change", -100, baseY + 120);
        Node* onBlurNode = createEventNode("On Blur", -100, baseY + 260);
        Node* onFocusNode = createEventNode("On Focus", -100, baseY + 400);
        
        // Add all the nodes to scripts
        scripts()->addNode(onChangeNode);
        scripts()->addNode(onBlurNode);
        scripts()->addNode(onFocusNode);
    }
    
    // Register properties
    registerProperties();
}

void WebTextInput::setPlaceholder(const QString &placeholder)
{
    if (m_placeholder != placeholder) {
        m_placeholder = placeholder;
        emit placeholderChanged();
    }
}

void WebTextInput::setValue(const QString &value)
{
    if (m_value != value) {
        m_value = value;
        emit valueChanged();
    }
}

void WebTextInput::setBorderColor(const QColor &color)
{
    if (m_borderColor != color) {
        m_borderColor = color;
        emit borderColorChanged();
    }
}

void WebTextInput::setBorderWidth(qreal width)
{
    if (!qFuzzyCompare(m_borderWidth, width)) {
        m_borderWidth = width;
        emit borderWidthChanged();
    }
}

void WebTextInput::setBorderRadius(qreal radius)
{
    if (!qFuzzyCompare(m_borderRadius, radius)) {
        m_borderRadius = radius;
        emit borderRadiusChanged();
    }
}

void WebTextInput::setIsEditing(bool editing)
{
    if (m_isEditing != editing) {
        m_isEditing = editing;
        emit isEditingChanged();
    }
}

void WebTextInput::setPosition(PositionType position)
{
    if (m_position != position) {
        m_position = position;
        emit positionChanged();
        
        // Trigger layout on parent if it's a frame with flex
        if (parentElement()) {
            Frame* parentFrame = qobject_cast<Frame*>(parentElement());
            if (parentFrame && parentFrame->flex()) {
                parentFrame->triggerLayout();
            }
        }
    }
}

void WebTextInput::setWidth(qreal w)
{
    DesignElement::setWidth(w);
    
    // Trigger layout on parent if it's a frame with flex and this element has relative position
    if (parentElement() && m_position == Relative) {
        Frame* parentFrame = qobject_cast<Frame*>(parentElement());
        if (parentFrame && parentFrame->flex()) {
            parentFrame->triggerLayout();
        }
    }
}

void WebTextInput::setHeight(qreal h)
{
    DesignElement::setHeight(h);
    
    // Trigger layout on parent if it's a frame with flex and this element has relative position
    if (parentElement() && m_position == Relative) {
        Frame* parentFrame = qobject_cast<Frame*>(parentElement());
        if (parentFrame && parentFrame->flex()) {
            parentFrame->triggerLayout();
        }
    }
}

void WebTextInput::setRect(const QRectF &rect)
{
    DesignElement::setRect(rect);
    
    // Trigger layout on parent if it's a frame with flex and this element has relative position
    if (parentElement() && m_position == Relative) {
        Frame* parentFrame = qobject_cast<Frame*>(parentElement());
        if (parentFrame && parentFrame->flex()) {
            parentFrame->triggerLayout();
        }
    }
}

void WebTextInput::executeScriptEvent(const QString& eventName, const QVariantMap& eventData)
{
    // Create event data containing the current value, merging with any provided data
    QVariantMap mergedEventData = eventData;
    mergedEventData["value"] = m_value;
    
    // Call the base implementation with the merged event data
    DesignElement::executeScriptEvent(eventName, mergedEventData);
}

void WebTextInput::registerProperties() {
    // Call parent implementation first
    DesignElement::registerProperties();
    
    // Register WebTextInput-specific properties
    m_properties->registerProperty("placeholder", QString("Enter text..."));
    m_properties->registerProperty("value", QString());
    m_properties->registerProperty("borderColor", QColor(170, 170, 170)); // #aaaaaa
    m_properties->registerProperty("borderWidth", 1.0);
    m_properties->registerProperty("borderRadius", 4.0);
    m_properties->registerProperty("isEditing", false);
    m_properties->registerProperty("position", static_cast<int>(Absolute));
}

QVariant WebTextInput::getProperty(const QString& name) const {
    // Handle WebTextInput-specific properties
    if (name == "placeholder") return placeholder();
    if (name == "value") return value();
    if (name == "borderColor") return borderColor();
    if (name == "borderWidth") return borderWidth();
    if (name == "borderRadius") return borderRadius();
    if (name == "isEditing") return isEditing();
    if (name == "position") return static_cast<int>(position());
    
    // Fall back to parent implementation
    return DesignElement::getProperty(name);
}

void WebTextInput::setProperty(const QString& name, const QVariant& value) {
    // Handle WebTextInput-specific properties
    if (name == "placeholder") {
        setPlaceholder(value.toString());
        return;
    }
    if (name == "value") {
        setValue(value.toString());
        return;
    }
    if (name == "borderColor") {
        setBorderColor(value.value<QColor>());
        return;
    }
    if (name == "borderWidth") {
        setBorderWidth(value.toReal());
        return;
    }
    if (name == "borderRadius") {
        setBorderRadius(value.toReal());
        return;
    }
    if (name == "isEditing") {
        setIsEditing(value.toBool());
        return;
    }
    if (name == "position") {
        setPosition(static_cast<PositionType>(value.toInt()));
        return;
    }
    
    // Fall back to parent implementation
    DesignElement::setProperty(name, value);
}