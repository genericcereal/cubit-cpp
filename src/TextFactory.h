#ifndef TEXTFACTORY_H
#define TEXTFACTORY_H

#include "ElementFactory.h"
#include "Text.h"
#include "TextComponentVariant.h"
#include "TextComponentInstance.h"

class TextFactory : public ElementFactoryBase<Text, TextComponentVariant, TextComponentInstance>
{
public:
    QString typeName() const override { return "text"; }
    QString displayName() const override { return "Text"; }
    QString category() const override { return "Basic"; }
    bool isContainer() const override { return false; }
    bool acceptsChildren() const override { return false; }
    
    QList<PropertyDefinition> propertyDefinitions() const override;
};

#endif // TEXTFACTORY_H