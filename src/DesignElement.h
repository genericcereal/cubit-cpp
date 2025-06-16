#ifndef DESIGNELEMENT_H
#define DESIGNELEMENT_H

#include "CanvasElement.h"
#include <memory>
#include <QString>

class Scripts;
Q_DECLARE_OPAQUE_POINTER(Scripts*)

class DesignElement : public CanvasElement
{
    Q_OBJECT
    Q_PROPERTY(Scripts* scripts READ scripts NOTIFY scriptsChanged)
    
public:
    explicit DesignElement(const QString &id, QObject *parent = nullptr);
    virtual ~DesignElement();
    
    // Override to identify as design element
    virtual bool isDesignElement() const { return true; }
    virtual bool isScriptElement() const { return false; }
    
    // Scripts management
    Scripts* scripts() const;
    
    // Execute a script event
    Q_INVOKABLE void executeScriptEvent(const QString& eventName);
    
signals:
    void scriptsChanged();
    
protected:
    std::unique_ptr<Scripts> m_scripts;
};

#endif // DESIGNELEMENT_H