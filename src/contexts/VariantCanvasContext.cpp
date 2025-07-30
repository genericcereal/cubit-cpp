#include "VariantCanvasContext.h"
#include "../Component.h"
#include "../DesignElement.h"
#include "../CanvasElement.h"
#include "../ElementModel.h"
#include "../Element.h"
#include <QDebug>

VariantCanvasContext::VariantCanvasContext(QObject* editingElement, QObject *parent)
    : CanvasContext(parent)
    , m_editingElement(editingElement)
{
}

QString VariantCanvasContext::displayName() const
{
    if (Component* comp = getComponent()) {
        return QString("Component: %1").arg(comp->getName());
    }
    if (DesignElement* elem = getDesignElement()) {
        return QString("Element: %1").arg(elem->getName());
    }
    return "Variant Canvas";
}

void VariantCanvasContext::activateContext(ElementModel* targetModel)
{
    Q_UNUSED(targetModel);
    // Nothing to do - in variant mode, elements are already in the model
    // The ElementFilterProxy will handle showing only variant elements
    
    if (Component* comp = getComponent()) {
        // qDebug() << "VariantCanvasContext: Activated for component" << comp->getName() 
        //          << "with" << comp->variants().size() << "variants";
    }
}

void VariantCanvasContext::deactivateContext(ElementModel* targetModel)
{
    Q_UNUSED(targetModel);
    // Nothing to clean up - elements remain in model
}

QRectF VariantCanvasContext::getPreferredViewport() const
{
    // Return empty rect to use default viewport
    return QRectF();
}

QPointF VariantCanvasContext::getCenterPoint() const
{
    // Center at origin for variant editing
    return QPointF(0, 0);
}

bool VariantCanvasContext::shouldIncludeInHitTest(Element* element) const
{
    if (!element) return false;
    
    // Check if element is frozen (should not be hoverable/selectable)
    DesignElement* designElement = qobject_cast<DesignElement*>(element);
    if (designElement && designElement->isFrozen()) {
        return false;
    }
    
    // Exclude script elements (Nodes and Edges) - they should only be hit-testable in script mode
    CanvasElement* canvasElement = qobject_cast<CanvasElement*>(element);
    if (canvasElement && canvasElement->isScriptElement()) {
        return false;
    }
    
    // In variant mode, we should hit test:
    // 1. The component's variants
    // 2. Children of those variants
    
    if (Component* comp = getComponent()) {
        // Check if this element is one of the component's variants
        const QList<Element*>& variants = comp->variants();
        
        // First check if any variants are null
        for (Element* variant : variants) {
            if (!variant) {
                qWarning() << "VariantCanvasContext: Found null variant in component" << comp->getName();
                continue;
            }
        }
        
        if (variants.contains(element)) {
            // qDebug() << "VariantCanvasContext: Including variant element" << element->getId();
            return true;
        }
        
        // Check if this element is a child of any variant
        for (Element* variant : variants) {
            if (!variant) continue; // Skip null variants
            
            if (element->getParentElementId() == variant->getId()) {
                // qDebug() << "VariantCanvasContext: Including child element" << element->getId() 
                //          << "of variant" << variant->getId();
                return true;
            }
        }
        
        // qDebug() << "VariantCanvasContext: Excluding element" << element->getId() 
        //          << "- not a variant or child of component" << comp->getName();
        return false;
    }
    
    // If editing a DesignElement (not a Component), include all elements
    return true;
}

Component* VariantCanvasContext::getComponent() const
{
    return qobject_cast<Component*>(m_editingElement.data());
}

DesignElement* VariantCanvasContext::getDesignElement() const
{
    return qobject_cast<DesignElement*>(m_editingElement.data());
}