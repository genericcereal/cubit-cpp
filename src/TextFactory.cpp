#include "TextFactory.h"
#include <QFont>
#include <QColor>

QList<PropertyDefinition> TextFactory::propertyDefinitions() const
{
    QList<PropertyDefinition> props;
    
    // Typography properties
    props.append(PropertyDefinition("content", QMetaType::QString, "Text", PropertyDefinition::Typography));
    props.append(PropertyDefinition("font", QMetaType::QFont, QFont(), PropertyDefinition::Typography));
    props.append(PropertyDefinition("color", QMetaType::QColor, QColor(Qt::black), PropertyDefinition::Typography));
    
    // Position properties
    props.append(PropertyDefinition("position", QMetaType::Int, Text::Absolute, PropertyDefinition::Layout));
    
    // Behavior properties (isEditing is internal, not user-editable)
    props.append(PropertyDefinition("isEditing", QMetaType::Bool, false, PropertyDefinition::Behavior, false));
    
    return props;
}