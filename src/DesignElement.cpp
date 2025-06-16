#include "DesignElement.h"
#include "Scripts.h"

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