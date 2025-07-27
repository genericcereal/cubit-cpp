#include "Text.h"
#include "Frame.h"
#include "PropertyRegistry.h"
#include <QDebug>

Text::Text(const QString &id, QObject *parent)
    : DesignElement(id, parent)
    , m_content("Text")
    , m_color(Qt::black)
{
    // Set element type
    elementType = Element::TextType;
    
    setName(QString("Text %1").arg(id.right(4)));  // Use last 4 digits for display
    
    // Register properties
    registerProperties();
}

void Text::setContent(const QString &content)
{
    if (m_content != content) {
        m_content = content;
        emit contentChanged();
        emit elementChanged();
    }
}

void Text::setFont(const QFont &font)
{
    // Force emit fontChanged even if Qt thinks fonts are equal
    // This ensures pixel size changes are propagated
    m_font = font;
    emit fontChanged();
    emit elementChanged();
}

void Text::setColor(const QColor &color)
{
    if (m_color != color) {
        m_color = color;
        emit colorChanged();
        emit elementChanged();
    }
}

void Text::setIsEditing(bool editing)
{
    if (m_isEditing != editing) {
        m_isEditing = editing;
        emit isEditingChanged();
        emit elementChanged();
    }
}

void Text::setPosition(PositionType position)
{
    if (m_position != position) {
        m_position = position;
        emit positionChanged();
        emit elementChanged();
        
        // Trigger layout on parent if it's a frame with flex
        if (parentElement()) {
            Frame* parentFrame = qobject_cast<Frame*>(parentElement());
            if (parentFrame && parentFrame->flex()) {
                parentFrame->triggerLayout();
            }
        }
    }
}

void Text::setWidth(qreal w)
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

void Text::setHeight(qreal h)
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

void Text::setRect(const QRectF &rect)
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

void Text::exitEditMode()
{
    // Simply exit edit mode - the TextField will handle saving via onIsEditingChanged
    setIsEditing(false);
}

QList<PropertyDefinition> Text::propertyDefinitions() const {
    // Use the static method to get consistent property definitions
    return Text::staticPropertyDefinitions();
}

void Text::registerProperties() {
    // Call parent implementation first
    DesignElement::registerProperties();
    
    // Register Text-specific properties
    m_properties->registerProperty("content", QString("Text"));
    m_properties->registerProperty("font", QFont());
    m_properties->registerProperty("color", QColor(Qt::black));
    m_properties->registerProperty("isEditing", false);
    m_properties->registerProperty("position", static_cast<int>(Absolute));
}

QVariant Text::getProperty(const QString& name) const {
    // Handle Text-specific properties
    if (name == "content") return content();
    if (name == "font") return font();
    if (name == "color") return color();
    if (name == "isEditing") return isEditing();
    if (name == "position") return static_cast<int>(position());
    
    // Fall back to parent implementation
    return DesignElement::getProperty(name);
}

void Text::setProperty(const QString& name, const QVariant& value) {
    // Handle Text-specific properties
    if (name == "content") {
        setContent(value.toString());
        return;
    }
    if (name == "font") {
        setFont(value.value<QFont>());
        return;
    }
    if (name == "color") {
        setColor(value.value<QColor>());
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

QList<PropertyDefinition> Text::staticPropertyDefinitions() {
    QList<PropertyDefinition> props;
    
    // Typography properties
    props.append(PropertyDefinition("content", QMetaType::QString, QString("Text"), PropertyDefinition::Typography));
    props.append(PropertyDefinition("font", QMetaType::QFont, QFont(), PropertyDefinition::Typography));
    props.append(PropertyDefinition("color", QMetaType::QColor, QColor(Qt::black), PropertyDefinition::Typography));
    
    // Position properties
    props.append(PropertyDefinition("position", QMetaType::Int, static_cast<int>(Text::Absolute), PropertyDefinition::Layout));
    
    // Behavior properties
    props.append(PropertyDefinition("isEditing", QMetaType::Bool, false, PropertyDefinition::Behavior, false)); // Not editable
    
    return props;
}