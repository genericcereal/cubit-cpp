#ifndef VARIANTCANVASCONTEXT_H
#define VARIANTCANVASCONTEXT_H

#include "../CanvasContext.h"
#include <QPointer>

class Component;
class DesignElement;

/**
 * Context for editing component variants
 */
class VariantCanvasContext : public CanvasContext
{
    Q_OBJECT
    
public:
    explicit VariantCanvasContext(QObject* editingElement, QObject *parent = nullptr);
    
    QString contextType() const override { return "variant"; }
    QString displayName() const override;
    
    void activateContext(ElementModel* targetModel) override;
    void deactivateContext(ElementModel* targetModel) override;
    
    QRectF getPreferredViewport() const override;
    QPointF getCenterPoint() const override;
    
    bool shouldIncludeInHitTest(Element* element) const override;
    
private:
    QPointer<QObject> m_editingElement;
    Component* getComponent() const;
    DesignElement* getDesignElement() const;
};

#endif // VARIANTCANVASCONTEXT_H