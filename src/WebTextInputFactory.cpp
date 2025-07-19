#include "WebTextInputFactory.h"
#include "platforms/web/WebTextInput.h"
#include <QFont>
#include <QColor>

QList<PropertyDefinition> WebTextInputFactory::propertyDefinitions() const
{
    QList<PropertyDefinition> props;
    
    // Input properties
    props.append(PropertyDefinition("value", QMetaType::QString, "", PropertyDefinition::Typography));
    props.append(PropertyDefinition("placeholder", QMetaType::QString, "Enter text...", PropertyDefinition::Typography));
    
    // Border properties
    props.append(PropertyDefinition("borderColor", QMetaType::QColor, QColor(170, 170, 170), PropertyDefinition::Appearance));
    props.append(PropertyDefinition("borderWidth", QMetaType::Double, 1.0, PropertyDefinition::Appearance));
    props.append(PropertyDefinition("borderRadius", QMetaType::Double, 4.0, PropertyDefinition::Appearance));
    
    // Position properties
    props.append(PropertyDefinition("position", QMetaType::Int, WebTextInput::Absolute, PropertyDefinition::Layout));
    
    // Behavior properties (isEditing is internal, not user-editable)
    props.append(PropertyDefinition("isEditing", QMetaType::Bool, false, PropertyDefinition::Behavior, false));
    
    return props;
}