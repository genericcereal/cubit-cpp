#ifndef MAINCANVASCONTEXT_H
#define MAINCANVASCONTEXT_H

#include "../CanvasContext.h"

/**
 * Context for the main design canvas - shows project's root elements
 */
class MainCanvasContext : public CanvasContext
{
    Q_OBJECT
    
public:
    explicit MainCanvasContext(QObject *parent = nullptr);
    
    QString contextType() const override { return "design"; }
    QString displayName() const override { return "Design Canvas"; }
    
    void activateContext(ElementModel* targetModel) override;
    void deactivateContext(ElementModel* targetModel) override;
    
    bool shouldIncludeInHitTest(Element* element) const override;
};

#endif // MAINCANVASCONTEXT_H