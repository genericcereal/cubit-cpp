#include "DesignElement.h"
#include "Scripts.h"
#include "ScriptExecutor.h"
#include "Application.h"
#include "Canvas.h"
#include <QCoreApplication>
#include <QDebug>

DesignElement::DesignElement(const QString &id, QObject *parent)
    : CanvasElement(Element::FrameType, id, parent)  // Use FrameType as default, subclasses will override
{
    m_scripts = std::make_unique<Scripts>(this);
}

DesignElement::~DesignElement() {
    // Destructor defined here so Scripts is complete type
}

Scripts* DesignElement::scripts() const {
    return m_scripts.get();
}

void DesignElement::executeScriptEvent(const QString& eventName) {
    qDebug() << "DesignElement::executeScriptEvent called for" << getId() << "event:" << eventName;
    
    if (!m_scripts) {
        qWarning() << "DesignElement: No scripts available";
        return;
    }
    
    qDebug() << "DesignElement" << getId() << "has" << m_scripts->nodeCount() << "nodes," << m_scripts->edgeCount() << "edges";
    
    // Create a temporary script executor for this element
    ScriptExecutor executor(this);
    executor.setScripts(m_scripts.get());
    
    // Get element model and canvas controller from Application
    // Since design elements are part of a canvas, we can get the active canvas
    Application* app = qobject_cast<Application*>(qApp);
    if (app && app->activeCanvas()) {
        Canvas* canvas = app->activeCanvas();
        if (canvas->elementModel()) {
            executor.setElementModel(canvas->elementModel());
        }
        if (canvas->controller()) {
            executor.setCanvasController(canvas->controller());
        }
    }
    
    executor.executeEvent(eventName);
}