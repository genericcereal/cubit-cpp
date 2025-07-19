#include "ElementFactory.h"
#include "DesignElement.h"
#include <QDebug>

void ElementFactory::applyDefaultProperties(DesignElement* element) const
{
    if (!element) {
        qWarning() << "Cannot apply properties to null element";
        return;
    }
    
    const auto properties = propertyDefinitions();
    for (const PropertyDefinition& prop : properties) {
        if (prop.isValid()) {
            element->setProperty(prop.name().toUtf8(), prop.defaultValue());
        }
    }
}