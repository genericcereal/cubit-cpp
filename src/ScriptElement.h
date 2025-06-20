#ifndef SCRIPTELEMENT_H
#define SCRIPTELEMENT_H

#include "CanvasElement.h"

class ScriptElement : public CanvasElement
{
    Q_OBJECT
    
public:
    explicit ScriptElement(const QString &id, QObject *parent = nullptr);
    virtual ~ScriptElement() = default;
    
    // Override to identify as script element
    virtual bool isDesignElement() const override { return false; }
    virtual bool isScriptElement() const override { return true; }
};

#endif // SCRIPTELEMENT_H