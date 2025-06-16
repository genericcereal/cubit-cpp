#include "DesignElement.h"
#include "Scripts.h"
#include "ScriptExecutor.h"
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
    if (!m_scripts) {
        qWarning() << "DesignElement: No scripts available";
        return;
    }
    
    // Create a temporary script executor for this element
    ScriptExecutor executor(this);
    executor.setScripts(m_scripts.get());
    
    // TODO: Set element model and canvas controller if needed
    // For now, just execute the event
    executor.executeEvent(eventName);
}