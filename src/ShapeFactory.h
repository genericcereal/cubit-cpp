#ifndef SHAPEFACTORY_H
#define SHAPEFACTORY_H

#include "ElementFactory.h"
#include "Shape.h"
#include "ShapeComponentVariant.h"
#include "ShapeComponentInstance.h"

class ShapeFactory : public ElementFactoryBase<Shape, ShapeComponentVariant, ShapeComponentInstance>
{
public:
    QString typeName() const override { return "shape"; }
    QString displayName() const override { return "Shape"; }
    QString category() const override { return "Basic"; }
    bool isContainer() const override { return false; }
    bool acceptsChildren() const override { return false; }
    
    QList<PropertyDefinition> propertyDefinitions() const override;
};

#endif // SHAPEFACTORY_H