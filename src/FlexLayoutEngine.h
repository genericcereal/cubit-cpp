#ifndef FLEXLAYOUTENGINE_H
#define FLEXLAYOUTENGINE_H

#include <QObject>
#include <QList>
#include <QRectF>
#include <QMap>
#include <QTimer>
#include <QSet>
#include "Frame.h"

class Element;
class ElementModel;

class FlexLayoutEngine : public QObject
{
    Q_OBJECT
    
public:
    explicit FlexLayoutEngine(QObject *parent = nullptr);
    
    // Layout reason enum
    enum LayoutReason {
        General,
        GapChanged,
        JustifyChanged,
        AlignChanged
    };
    
    // Main layout method
    void layoutChildren(Frame* parentFrame, ElementModel* elementModel, LayoutReason reason = General);
    
    // Calculate layout for a list of children
    void calculateFlexLayout(const QList<Element*>& children, 
                           Frame* parentFrame,
                           const QRectF& containerBounds);
    
    // Connect/disconnect child geometry change notifications
    void connectChildGeometrySignals(Frame* parentFrame, ElementModel* elementModel);
    void disconnectChildGeometrySignals(Frame* parentFrame);
    
    // Check if layout is in progress
    bool isLayouting() const { return m_isLayouting; }
    
    // Schedule a layout for a frame (batches multiple requests)
    void scheduleLayout(Frame* parentFrame, LayoutReason reason = General);
                           
private slots:
    void processPendingLayouts();
    
private:
    // Helper methods
    QList<Element*> getDirectChildren(const QString& parentId, ElementModel* elementModel) const;
    
    // Layout calculations
    void layoutRow(const QList<Element*>& children, 
                   Frame* parentFrame,
                   const QRectF& containerBounds);
                   
    void layoutColumn(const QList<Element*>& children, 
                      Frame* parentFrame,
                      const QRectF& containerBounds);
    
    // Calculate main axis position based on justify property
    qreal calculateMainAxisPosition(qreal containerSize,
                                   qreal totalChildrenSize,
                                   qreal totalGaps,
                                   int childCount,
                                   Frame::JustifyContent justify) const;
    
    // Calculate cross axis position based on align property
    qreal calculateCrossAxisPosition(qreal containerSize,
                                    qreal childSize,
                                    Frame::AlignItems align) const;
    
    // Calculate required size for parent to fit all children
    QSizeF calculateRequiredSize(const QList<Element*>& children, 
                                Frame* parentFrame) const;
    
    // Capture initial margins between parent and children
    void captureInitialMargins(Frame* parentFrame, const QList<Element*>& children);
    
    // Reset margins for a frame (call when children are added/removed)
    void resetMargins(const QString& frameId);
    
    // Track connections to avoid duplicates
    QMap<QString, QMetaObject::Connection> m_childConnections;
    
    // Flag to prevent infinite layout loops
    mutable bool m_isLayouting = false;
    
    // Layout batching
    QTimer* m_layoutBatchTimer = nullptr;
    struct PendingLayout {
        Frame* frame;
        LayoutReason reason;
    };
    QMap<Frame*, PendingLayout> m_pendingLayoutFrames;
    
    // Track initial margins for each parent frame
    struct FrameMargins {
        qreal left = -1;
        qreal right = -1;
        qreal top = -1;
        qreal bottom = -1;
        bool isInitialized() const { return left >= 0 && right >= 0 && top >= 0 && bottom >= 0; }
    };
    mutable QMap<QString, FrameMargins> m_frameMargins;
};

#endif // FLEXLAYOUTENGINE_H