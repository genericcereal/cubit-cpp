#ifndef GLOBALELEMENTSCONTEXT_H
#define GLOBALELEMENTSCONTEXT_H

#include "../CanvasContext.h"
#include <QPointer>
#include <QStringList>

class PlatformConfig;
class ElementModel;

/**
 * Context for viewing/editing platform-specific global elements
 */
class GlobalElementsContext : public CanvasContext
{
    Q_OBJECT
    
public:
    explicit GlobalElementsContext(PlatformConfig* platform, QObject *parent = nullptr);
    
    QString contextType() const override { return "globalElements"; }
    QString displayName() const override;
    
    bool isTemporary() const override { return true; }
    
    void activateContext(ElementModel* targetModel) override;
    void deactivateContext(ElementModel* targetModel) override;
    
    bool shouldIncludeInHitTest(Element* element) const override;
    
    QPointF getCenterPoint() const override { return QPointF(0, 0); }
    
private:
    QPointer<PlatformConfig> m_platform;
    QStringList m_addedElementIds;  // Track which elements we added
};

#endif // GLOBALELEMENTSCONTEXT_H