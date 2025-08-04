#ifndef ELEMENTTEMPLATES_H
#define ELEMENTTEMPLATES_H

#include <QString>
#include <QList>
#include <memory>
#include "PropertyDefinition.h"

class DesignElement;

/**
 * Template for creating elements without factories.
 * Each element type should specialize this template.
 */
template<typename TElement>
struct ElementTraits {
    static QString typeName() { return TElement::staticTypeName(); }
    static QList<PropertyDefinition> propertyDefinitions() { return TElement::staticPropertyDefinitions(); }
    
    static TElement* createElement(const QString& id) {
        return new TElement(id);
    }
};

/**
 * Template-based element creator that can be used directly
 * without the need for factory classes.
 */
template<typename TElement>
class ElementCreator {
public:
    static QString typeName() {
        return ElementTraits<TElement>::typeName();
    }
    
    static QList<PropertyDefinition> propertyDefinitions() {
        return ElementTraits<TElement>::propertyDefinitions();
    }
    
    static TElement* create(const QString& id) {
        return ElementTraits<TElement>::createElement(id);
    }
    
    static DesignElement* createAsDesignElement(const QString& id) {
        return static_cast<DesignElement*>(create(id));
    }
};

/**
 * Helper functions for element creation using templates
 */
namespace ElementTemplateHelpers {
    template<typename TElement>
    static TElement* create(const QString& id) {
        return ElementCreator<TElement>::create(id);
    }
}

#endif // ELEMENTTEMPLATES_H