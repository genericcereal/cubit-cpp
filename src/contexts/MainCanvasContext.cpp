#include "MainCanvasContext.h"
#include "../ElementModel.h"
#include "../Element.h"
#include "../DesignElement.h"
#include "../Project.h"
#include "../PlatformConfig.h"
#include "../Component.h"
#include <QDebug>

MainCanvasContext::MainCanvasContext(QObject *parent)
    : CanvasContext(parent)
{
}

void MainCanvasContext::activateContext(ElementModel* targetModel)
{
    if (targetModel) {
        // Force a refresh of the element list to update parent-child relationships
        // This is needed when returning from globalElements mode where new instances
        // may have been created
        targetModel->refresh();
    }
}

void MainCanvasContext::deactivateContext(ElementModel* targetModel)
{
    Q_UNUSED(targetModel);
    // Nothing to clean up for main canvas
}

bool MainCanvasContext::shouldIncludeInHitTest(Element* element) const
{
    if (!element) return false;
    
    // Check if element is frozen (should not be hoverable/selectable)
    DesignElement* designElement = qobject_cast<DesignElement*>(element);
    if (designElement && designElement->isFrozen()) {
        return false;
    }
    
    // In main canvas context, we should exclude:
    // 1. Elements that are component variants (they should only be editable in variant mode)
    // 2. Elements that belong to any platform's globalElements (only their instances should be hit-testable)
    
    // Get the project from parent hierarchy
    Project* project = nullptr;
    QObject* p = parent();
    while (p && !project) {
        project = qobject_cast<Project*>(p);
        p = p->parent();
    }
    
    if (project) {
        // Check if this element belongs to any platform's globalElements
        QList<PlatformConfig*> platforms = project->getAllPlatforms();
        for (PlatformConfig* platform : platforms) {
            if (platform && platform->globalElements()) {
                if (platform->globalElements()->getElementById(element->getId())) {
                    // This is an original element in globalElements - exclude it
                    return false;
                }
            }
        }
        
        // Check if this element is a component variant
        ElementModel* model = project->elementModel();
        if (model) {
            QList<Element*> allElements = model->getAllElements();
            for (Element* el : allElements) {
                Component* component = qobject_cast<Component*>(el);
                if (component) {
                    const QList<Element*>& variants = component->variants();
                    if (variants.contains(element)) {
                        // This element is a component variant
                        return false;
                    }
                    
                    // Also check children of variants
                    for (Element* variant : variants) {
                        if (element->getParentElementId() == variant->getId()) {
                            return false;
                        }
                    }
                }
            }
        }
    }
    
    // Include all other elements (instances of global elements will pass through here)
    return true;
}