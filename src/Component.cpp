#include "Component.h"
#include "CanvasElement.h"
#include "DesignElement.h"
#include "Variable.h"
#include "Scripts.h"
#include <QDebug>

Component::Component(const QString &id, QObject *parent)
    : Element(ComponentType, id, parent)
    , m_scripts(std::make_unique<Scripts>(this))
{
    // Set the name to "Component" + last 4 digits of ID
    QString last4 = id.right(4);
    setName("Component" + last4);
}

Component::~Component()
{
}

void Component::addVariant(Element* variant)
{
    if (variant && !m_variants.contains(variant)) {
        // Accept DesignElements (Frame, Text, Html, ComponentVariant) and Variables
        CanvasElement* canvasElement = qobject_cast<CanvasElement*>(variant);
        if ((canvasElement && canvasElement->isDesignElement()) || qobject_cast<Variable*>(variant)) {
            m_variants.append(variant);
            qDebug() << "Component::addVariant - Added" << variant->getTypeName() << variant->getId() 
                     << "to Component" << getId() << "- Total variants:" << m_variants.size();
            emit variantsChanged();
        }
    }
}

void Component::removeVariant(Element* variant)
{
    if (m_variants.removeOne(variant)) {
        emit variantsChanged();
    }
}

void Component::clearVariants()
{
    if (!m_variants.isEmpty()) {
        m_variants.clear();
        emit variantsChanged();
    }
}

void Component::addDesignElement(DesignElement* element)
{
    if (element) {
        addVariant(element);
    }
}

void Component::addVariable(Variable* variable)
{
    if (variable) {
        addVariant(variable);
    }
}

Scripts* Component::scripts() const
{
    return m_scripts.get();
}