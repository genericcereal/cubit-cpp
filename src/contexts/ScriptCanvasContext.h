#ifndef SCRIPTCANVASCONTEXT_H
#define SCRIPTCANVASCONTEXT_H

#include "../CanvasContext.h"
#include "../Scripts.h"
#include <QPointer>
class Element;
class DesignElement;
// class Component; // Component system removed
class PlatformConfig;

/**
 * Context for script mode - shows nodes and edges
 */
class ScriptCanvasContext : public CanvasContext
{
    Q_OBJECT
    
public:
    explicit ScriptCanvasContext(Scripts* scripts, QObject* editingElement = nullptr, QObject *parent = nullptr);
    
    QString contextType() const override { return "script"; }
    QString displayName() const override;
    
    void activateContext(ElementModel* targetModel) override;
    void deactivateContext(ElementModel* targetModel) override;
    
    Scripts* getScripts() const override { return m_scripts; }
    
    QPointF getCenterPoint() const override { return QPointF(400, 200); } // Default onLoad node position
    
private:
    QPointer<Scripts> m_scripts;
    QPointer<QObject> m_editingElement;
    
    QString getEditingElementName() const;
};

#endif // SCRIPTCANVASCONTEXT_H