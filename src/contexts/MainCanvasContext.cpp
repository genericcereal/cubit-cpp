#include "MainCanvasContext.h"
#include "../ElementModel.h"
#include "../Element.h"
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
    Q_UNUSED(targetModel);
    // Main canvas context doesn't need to do anything special
    // The elements are already in the model
}

void MainCanvasContext::deactivateContext(ElementModel* targetModel)
{
    Q_UNUSED(targetModel);
    // Nothing to clean up for main canvas
}

bool MainCanvasContext::shouldIncludeInHitTest(Element* element) const
{
    if (!element) return false;
    
    // In main canvas context, we should exclude:
    // 1. Elements that belong to any platform's global elements
    // 2. Elements that are component variants
    
    // Get the project from parent hierarchy
    Project* project = nullptr;
    QObject* p = parent();
    while (p && !project) {
        project = qobject_cast<Project*>(p);
        p = p->parent();
    }
    
    if (project) {
        // Check all platforms to see if this element belongs to their global elements
        QList<PlatformConfig*> platforms = project->getAllPlatforms();
        for (PlatformConfig* platform : platforms) {
            if (platform && platform->globalElements()) {
                if (platform->globalElements()->getElementById(element->getId())) {
                    // This element belongs to a platform's global elements
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
    
    // Include all other elements
    return true;
}