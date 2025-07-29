#include "GlobalElementsContext.h"
#include "../PlatformConfig.h"
#include "../ElementModel.h"
#include "../Element.h"
#include "../DesignElement.h"
#include <QDebug>

GlobalElementsContext::GlobalElementsContext(PlatformConfig* platform, QObject *parent)
    : CanvasContext(parent)
    , m_platform(platform)
{
}

QString GlobalElementsContext::displayName() const
{
    if (m_platform) {
        return QString("Global Elements - %1").arg(m_platform->name());
    }
    return "Global Elements";
}

void GlobalElementsContext::activateContext(ElementModel* targetModel)
{
    if (!targetModel || !m_platform) return;
    
    // Clear any previously tracked elements
    m_addedElementIds.clear();
    
    // Get the global elements from the platform
    ElementModel* globalElements = m_platform->globalElements();
    if (!globalElements) {
        return;
    }
    
    // Get all elements from the platform's global elements
    auto elements = globalElements->getAllElements();
    
    // We need to be very careful here. The elements have QObject parent relationships
    // that we cannot change. We'll add them to the model but must ensure proper cleanup.
    for (Element* element : elements) {
        if (element) {
            // Check if element is already in target model to avoid duplicates
            if (!targetModel->getElementById(element->getId())) {
                // Track which elements we're adding
                m_addedElementIds.append(element->getId());
                // Add to model but platform's globalElements retains ownership
                // The element's QObject parent remains unchanged
                targetModel->addElement(element);
            }
        }
    }
}

void GlobalElementsContext::deactivateContext(ElementModel* targetModel)
{
    if (!targetModel) return;
    
    
    // Remove the global elements we temporarily added
    // Use removeElementWithoutDelete to avoid deleting elements owned by the platform
    for (const QString& elementId : m_addedElementIds) {
        Element* element = targetModel->getElementById(elementId);
        if (element) {
            targetModel->removeElementWithoutDelete(element);
        }
    }
    
    // Clear the tracking list
    m_addedElementIds.clear();
}

bool GlobalElementsContext::shouldIncludeInHitTest(Element* element) const
{
    if (!element) return false;
    
    // Check if element is frozen (should not be hoverable/selectable)
    DesignElement* designElement = qobject_cast<DesignElement*>(element);
    if (designElement && designElement->isFrozen()) {
        return false;
    }
    
    // In global elements context, we want to ONLY hit test the global elements
    // from the platform. Check if this element belongs to the platform's globalElements.
    if (m_platform && m_platform->globalElements()) {
        ElementModel* globalElements = m_platform->globalElements();
        // Check if the element exists in the platform's global elements model
        return globalElements->getElementById(element->getId()) != nullptr;
    }
    
    // Fallback to checking our tracked list
    return m_addedElementIds.contains(element->getId());
}