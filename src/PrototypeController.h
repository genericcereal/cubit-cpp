#pragma once
#include <QObject>
#include <QPointF>
#include <QRectF>
#include <QHash>
#include <memory>

class ElementModel;
class SelectionManager;
class CanvasElement;

// Structure to store constraint values
struct ConstraintValues {
    qreal left = 0;
    qreal right = 0;
    qreal top = 0;
    qreal bottom = 0;
    bool leftAnchored = false;
    bool rightAnchored = false;
    bool topAnchored = false;
    bool bottomAnchored = false;
};

// Snapshot structure to capture canvas and element states
struct PrototypeSnapshot {
    QPointF canvasPosition;
    qreal canvasZoom = 1.0;
    QHash<QString, QRectF> elementPositions; // elementId -> QRectF(x, y, width, height)
    QHash<QString, ConstraintValues> elementConstraints; // elementId -> constraint values
};

class PrototypeController : public QObject {
    Q_OBJECT
    Q_PROPERTY(bool isPrototyping READ isPrototyping WRITE setIsPrototyping NOTIFY isPrototypingChanged)
    Q_PROPERTY(QRectF viewableArea READ viewableArea WRITE setViewableArea NOTIFY viewableAreaChanged)
    Q_PROPERTY(QString prototypeMode READ prototypeMode WRITE setPrototypeMode NOTIFY prototypeModeChanged)
    Q_PROPERTY(qreal animatedBoundingX READ animatedBoundingX NOTIFY animatedBoundsChanged)
    Q_PROPERTY(qreal animatedBoundingY READ animatedBoundingY NOTIFY animatedBoundsChanged)
    Q_PROPERTY(qreal animatedBoundingWidth READ animatedBoundingWidth NOTIFY animatedBoundsChanged)
    Q_PROPERTY(qreal animatedBoundingHeight READ animatedBoundingHeight NOTIFY animatedBoundsChanged)
    Q_PROPERTY(qreal selectedFrameX READ selectedFrameX NOTIFY selectedFramePositionChanged)
    Q_PROPERTY(qreal selectedFrameHeight READ selectedFrameHeight NOTIFY selectedFramePositionChanged)
    Q_PROPERTY(QString activeOuterFrame READ activeOuterFrame WRITE setActiveOuterFrame NOTIFY activeOuterFrameChanged)
    
public:
    explicit PrototypeController(ElementModel& model,
                                SelectionManager& sel,
                                QObject *parent = nullptr);
    ~PrototypeController() = default;
    
    // Prototyping state
    bool isPrototyping() const { return m_isPrototyping; }
    void setIsPrototyping(bool value);
    
    // Viewable area for prototyping
    QRectF viewableArea() const { return m_viewableArea; }
    void setViewableArea(const QRectF& area);
    
    // Prototype mode (ios, web, android)
    QString prototypeMode() const { return m_prototypeMode; }
    void setPrototypeMode(const QString& mode);
    
    // Snapshot access methods
    Q_INVOKABLE QPointF getSnapshotCanvasPosition() const;
    Q_INVOKABLE qreal getSnapshotCanvasZoom() const;
    Q_INVOKABLE QRectF getSnapshotElementPosition(const QString& elementId) const;
    Q_INVOKABLE void restoreElementPositionsFromSnapshot();
    
    // Helper methods
    Q_INVOKABLE void startPrototyping(const QPointF& canvasCenter, qreal currentZoom);
    Q_INVOKABLE void stopPrototyping();
    Q_INVOKABLE QRectF calculateViewportForMode(const QString& mode) const;
    Q_INVOKABLE bool isElementInViewableArea(qreal x, qreal y, qreal width, qreal height) const;
    Q_INVOKABLE void updateAnimatedBounds(const QList<QRectF>& elementBounds);
    
    // Animated bounds getters
    qreal animatedBoundingX() const { return m_animatedBoundingX; }
    qreal animatedBoundingY() const { return m_animatedBoundingY; }
    qreal animatedBoundingWidth() const { return m_animatedBoundingWidth; }
    qreal animatedBoundingHeight() const { return m_animatedBoundingHeight; }
    
    // Selected frame position for centering
    qreal selectedFrameX() const { return m_selectedFrameX; }
    qreal selectedFrameHeight() const { return m_selectedFrameHeight; }
    
    // Active outer frame property
    QString activeOuterFrame() const { return m_activeOuterFrame; }
    void setActiveOuterFrame(const QString& frameId);
    
private:
    void setDeviceFrames(bool isModeChange = false, qreal oldViewableHeight = 0.0);
    void updateChildLayouts(CanvasElement* parent);
    void onSelectionChanged();
    
signals:
    void isPrototypingChanged();
    void viewableAreaChanged();
    void prototypeModeChanged();
    void prototypingStarted();
    void prototypingStopped();
    void animatedBoundsChanged();
    void selectedFramePositionChanged();
    void activeOuterFrameChanged();
    void requestCanvasMove(const QPointF& canvasPoint, bool animated);
    
private:
    ElementModel& m_elementModel;
    SelectionManager& m_selectionManager;
    
    bool m_isPrototyping = false;
    QRectF m_viewableArea;
    QString m_prototypeMode = "web";
    std::unique_ptr<PrototypeSnapshot> m_prototypingStartSnapshot;
    
    // Animated bounds tracking
    qreal m_animatedBoundingX = 0;
    qreal m_animatedBoundingY = 0;
    qreal m_animatedBoundingWidth = 0;
    qreal m_animatedBoundingHeight = 0;
    
    // Selected frame position for centering
    qreal m_selectedFrameX = -1;
    qreal m_selectedFrameHeight = 0;
    
    // Active outer frame
    QString m_activeOuterFrame;
    bool m_isInitializingPrototype = false;
    
    // Default viewable area dimensions for different platforms
    static constexpr qreal IOS_WIDTH = 200.0;
    static constexpr qreal IOS_HEIGHT = 400.0;
    static constexpr qreal WEB_WIDTH = 800.0;
    static constexpr qreal WEB_HEIGHT = 400.0;
    static constexpr qreal ANDROID_WIDTH = 200.0;
    static constexpr qreal ANDROID_HEIGHT = 400.0;
};