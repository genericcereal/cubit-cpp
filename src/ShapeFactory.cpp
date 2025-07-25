#include "ShapeFactory.h"
#include <QMetaEnum>
#include <QColor>

QList<PropertyDefinition> ShapeFactory::propertyDefinitions() const
{
    QList<PropertyDefinition> props;
    
    // Get the Shape meta object for enum
    const QMetaObject* metaObj = &Shape::staticMetaObject;
    
    // Find ShapeType enum
    for (int i = 0; i < metaObj->enumeratorCount(); ++i) {
        QMetaEnum metaEnum = metaObj->enumerator(i);
        if (QString(metaEnum.name()) == "ShapeType") {
            props.append(PropertyDefinition("shapeType", metaEnum, Shape::Square, PropertyDefinition::Appearance));
            break;
        }
    }
    
    // Edge width property
    props.append(PropertyDefinition("edgeWidth", QMetaType::Double, 2.0, PropertyDefinition::Appearance));
    
    // Edge color property
    props.append(PropertyDefinition("edgeColor", QMetaType::QColor, QColor(0, 0, 0, 255), PropertyDefinition::Appearance));
    
    // Fill color property
    props.append(PropertyDefinition("fillColor", QMetaType::QColor, QColor(0, 120, 255, 255), PropertyDefinition::Appearance));
    
    // Has fill property
    props.append(PropertyDefinition("hasFill", QMetaType::Bool, false, PropertyDefinition::Appearance));
    
    return props;
}