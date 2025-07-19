#ifndef WEBTEXTINPUTFACTORY_H
#define WEBTEXTINPUTFACTORY_H

#include "ElementFactory.h"
#include "platforms/web/WebTextInput.h"
#include "WebTextInputComponentVariant.h"
#include "WebTextInputComponentInstance.h"

class WebTextInputFactory : public ElementFactoryBase<WebTextInput, WebTextInputComponentVariant, WebTextInputComponentInstance>
{
public:
    QString typeName() const override { return "webtextinput"; }
    QString displayName() const override { return "Web Text Input"; }
    QString category() const override { return "Web"; }
    bool isContainer() const override { return false; }
    bool acceptsChildren() const override { return false; }
    
    QList<PropertyDefinition> propertyDefinitions() const override;
};

#endif // WEBTEXTINPUTFACTORY_H