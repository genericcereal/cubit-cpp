#pragma once
#include <QObject>
#include <QPointF>
#include <QRectF>
#include <QHash>
#include <memory>

class ElementModel;
class SelectionManager;
class CanvasElement;
class HitTestService;
class CanvasController;

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
    QHash<QString, QString> webTextInputValues; // elementId -> original value
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
    Q_PROPERTY(QString hoveredElement READ hoveredElement NOTIFY hoveredElementChanged)
    Q_PROPERTY(QString activeElement READ activeElement NOTIFY activeElementChanged)
    Q_PROPERTY(bool isSimulatingScroll READ isSimulatingScroll WRITE setIsSimulatingScroll NOTIFY isSimulatingScrollChanged)
    Q_PROPERTY(qreal scrollOffsetX READ scrollOffsetX WRITE setScrollOffsetX NOTIFY scrollOffsetXChanged)
    Q_PROPERTY(qreal scrollOffsetY READ scrollOffsetY WRITE setScrollOffsetY NOTIFY scrollOffsetYChanged)
    Q_PROPERTY(QString previousActiveFrameId READ previousActiveFrameId WRITE setPreviousActiveFrameId NOTIFY previousActiveFrameIdChanged)
    
public:
    explicit PrototypeController(ElementModel& model,
                                SelectionManager& sel,
                                QObject *parent = nullptr);
    ~PrototypeController() = default;
    
    // Set the canvas controller (must be called after construction)
    void setCanvasController(CanvasController* controller) { m_canvasController = controller; }
    
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
    
    // Hovered element property
    QString hoveredElement() const { return m_hoveredElement; }
    
    // Active element property
    QString activeElement() const { return m_activeElement; }
    
    // Method to update hovered element (to be called from QML)
    Q_INVOKABLE void updateHoveredElement(const QPointF& canvasPoint);
    
    // Method to handle clicks in prototype mode
    Q_INVOKABLE void handlePrototypeClick(const QPointF& canvasPoint);
    
    // Method to clear any active WebTextInput
    Q_INVOKABLE void clearActiveInput();
    
    // Scroll simulation properties
    bool isSimulatingScroll() const { return m_isSimulatingScroll; }
    void setIsSimulatingScroll(bool value);
    
    qreal scrollOffsetX() const { return m_scrollOffsetX; }
    void setScrollOffsetX(qreal offset);
    
    qreal scrollOffsetY() const { return m_scrollOffsetY; }
    void setScrollOffsetY(qreal offset);
    
    QString previousActiveFrameId() const { return m_previousActiveFrameId; }
    void setPreviousActiveFrameId(const QString& frameId);
    
    // Method to reset scroll position for a specific frame
    Q_INVOKABLE void resetFrameScrollPosition(const QString& frameId);
    
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
    void hoveredElementChanged();
    void activeElementChanged();
    void requestCanvasMove(const QPointF& canvasPoint, bool animated);
    void isSimulatingScrollChanged();
    void scrollOffsetXChanged();
    void scrollOffsetYChanged();
    void previousActiveFrameIdChanged();
    
private:
    ElementModel& m_elementModel;
    SelectionManager& m_selectionManager;
    CanvasController* m_canvasController = nullptr;
    
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
    
    // Hovered element
    QString m_hoveredElement;
    
    // Currently active element (e.g., WebTextInput being edited)
    QString m_activeElement;
    
    // Scroll simulation state
    bool m_isSimulatingScroll = false;
    qreal m_scrollOffsetX = 0.0;
    qreal m_scrollOffsetY = 0.0;
    QString m_previousActiveFrameId;
    
    // Default viewable area dimensions for different platforms
    static constexpr qreal IOS_WIDTH = 375.0;
    static constexpr qreal IOS_HEIGHT = 812.0;
    static constexpr qreal WEB_WIDTH = 800.0;
    static constexpr qreal WEB_HEIGHT = 400.0;
    static constexpr qreal ANDROID_WIDTH = 200.0;
    static constexpr qreal ANDROID_HEIGHT = 400.0;
};