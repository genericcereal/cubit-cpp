#include "FrameFactory.h"
#include <QMetaEnum>
#include <QColor>
#include <QFont>

QList<PropertyDefinition> FrameFactory::propertyDefinitions() const
{
    QList<PropertyDefinition> props;
    
    // Appearance properties
    props.append(PropertyDefinition("fill", QMetaType::QColor, QColor(173, 216, 230), PropertyDefinition::Appearance));
    props.append(PropertyDefinition("borderColor", QMetaType::QColor, QColor(Qt::black), PropertyDefinition::Appearance));
    props.append(PropertyDefinition("borderWidth", QMetaType::Int, 0, PropertyDefinition::Appearance));
    props.append(PropertyDefinition("borderRadius", QMetaType::Int, 0, PropertyDefinition::Appearance));
    
    // Layout properties
    props.append(PropertyDefinition("flex", QMetaType::Bool, false, PropertyDefinition::Layout));
    props.append(PropertyDefinition("gap", QMetaType::Double, 0.0, PropertyDefinition::Layout));
    
    // Size properties
    props.append(PropertyDefinition("widthType", QMetaType::Int, Frame::SizeFixed, PropertyDefinition::Layout));
    props.append(PropertyDefinition("heightType", QMetaType::Int, Frame::SizeFixed, PropertyDefinition::Layout));
    
    // Position properties
    props.append(PropertyDefinition("position", QMetaType::Int, Frame::Absolute, PropertyDefinition::Layout));
    
    // Flex layout properties
    props.append(PropertyDefinition("orientation", QMetaType::Int, Frame::Row, PropertyDefinition::Layout));
    props.append(PropertyDefinition("justify", QMetaType::Int, Frame::JustifyStart, PropertyDefinition::Layout));
    props.append(PropertyDefinition("align", QMetaType::Int, Frame::AlignStart, PropertyDefinition::Layout));
    
    // Behavior properties
    props.append(PropertyDefinition("acceptsChildren", QMetaType::Bool, true, PropertyDefinition::Behavior));
    props.append(PropertyDefinition("controlled", QMetaType::Bool, true, PropertyDefinition::Behavior));
    props.append(PropertyDefinition("overflow", QMetaType::Int, Frame::Hidden, PropertyDefinition::Behavior));
    
    // Advanced properties
    props.append(PropertyDefinition("platform", QMetaType::QString, QString(), PropertyDefinition::Advanced));
    props.append(PropertyDefinition("role", QMetaType::Int, Frame::undefined, PropertyDefinition::Advanced));
    
    // Add enum properties
    props.append(getEnumProperties());
    
    return props;
}

QList<PropertyDefinition> FrameFactory::getEnumProperties()
{
    QList<PropertyDefinition> enumProps;
    
    // Get the Frame meta object
    const QMetaObject* metaObj = &Frame::staticMetaObject;
    
    // Find and add enum properties
    for (int i = 0; i < metaObj->enumeratorCount(); ++i) {
        QMetaEnum metaEnum = metaObj->enumerator(i);
        QString enumName = metaEnum.name();
        
        // Map enum names to property names
        if (enumName == "ColorFormat") {
            enumProps.append(PropertyDefinition("colorFormat", metaEnum, Frame::HEX, PropertyDefinition::Appearance));
        }
        // Add more enum mappings as needed
    }
    
    return enumProps;
}