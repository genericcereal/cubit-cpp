#ifndef DESIGNELEMENT_H
#define DESIGNELEMENT_H

#include "CanvasElement.h"

class DesignElement : public CanvasElement
{
    Q_OBJECT
    
public:
    explicit DesignElement(const QString &id, QObject *parent = nullptr);
    virtual ~DesignElement() = default;
    
    // Override to identify as design element
    virtual bool isDesignElement() const { return true; }
    virtual bool isScriptElement() const { return false; }
};

#endif // DESIGNELEMENT_H