#ifndef CANVASCONTEXT_H
#define CANVASCONTEXT_H

#include <QObject>
#include <QString>
#include <memory>
#include <QRectF>
#include "CanvasController.h"

class ElementModel;
class Scripts;
class Element;
class HitTestService;

/**
 * Base class for canvas contexts that provide elements from different sources
 * This allows the canvas to display elements from various locations (component variants,
 * global elements, linked objects, etc.) without hardcoding each case.
 */
class CanvasContext : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QString contextType READ contextType CONSTANT)
    Q_PROPERTY(QString displayName READ displayName NOTIFY displayNameChanged)
    Q_PROPERTY(bool isTemporary READ isTemporary CONSTANT)
    
public:
    explicit CanvasContext(QObject *parent = nullptr);
    virtual ~CanvasContext();
    
    // Context identification
    virtual QString contextType() const = 0;
    virtual QString displayName() const = 0;
    
    // Determines if elements are temporarily added to main model or use separate model
    virtual bool isTemporary() const { return false; }
    
    // Element provisioning
    virtual void activateContext(ElementModel* targetModel) = 0;
    virtual void deactivateContext(ElementModel* targetModel) = 0;
    
    // Optional: Provide scripts for script mode
    virtual Scripts* getScripts() const { return nullptr; }
    
    // Optional: Provide viewport hints
    virtual QRectF getPreferredViewport() const { return QRectF(); }
    virtual QPointF getCenterPoint() const { return QPointF(0, 0); }
    
    // Optional: Canvas type hint for controller
    virtual CanvasController::CanvasType getCanvasType() const;
    
    // Hit testing support
    virtual bool shouldIncludeInHitTest(Element* element) const { Q_UNUSED(element); return true; }
    virtual void configureHitTestService(HitTestService* service) { Q_UNUSED(service); }
    
signals:
    void displayNameChanged();
    void contextModified();
    
protected:
    // Helper methods for derived classes
    void notifyContextModified() { emit contextModified(); }
};

#endif // CANVASCONTEXT_H