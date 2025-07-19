#ifndef FRAMEFACTORY_H
#define FRAMEFACTORY_H

#include "ElementFactory.h"
#include "Frame.h"
#include "FrameComponentVariant.h"
#include "FrameComponentInstance.h"

class FrameFactory : public ElementFactoryBase<Frame, FrameComponentVariant, FrameComponentInstance>
{
public:
    QString typeName() const override { return "frame"; }
    QString displayName() const override { return "Frame"; }
    QString category() const override { return "Basic"; }
    bool isContainer() const override { return true; }
    bool acceptsChildren() const override { return true; }
    
    QList<PropertyDefinition> propertyDefinitions() const override;
    
private:
    // Helper to get enum properties from Frame's meta object
    static QList<PropertyDefinition> getEnumProperties();
};

#endif // FRAMEFACTORY_H