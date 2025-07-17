#include "WebTextInput.h"
#include "Scripts.h"
#include "Node.h"
#include "UniqueIdGenerator.h"
#include "Frame.h"

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