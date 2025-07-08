#pragma once
#include <QObject>
#include <QPointF>
#include <QRectF>
#include <memory>

class ElementModel;
class SelectionManager;

class PrototypeController : public QObject {
    Q_OBJECT
    Q_PROPERTY(bool isPrototyping READ isPrototyping WRITE setIsPrototyping NOTIFY isPrototypingChanged)
    Q_PROPERTY(QRectF viewableArea READ viewableArea WRITE setViewableArea NOTIFY viewableAreaChanged)
    Q_PROPERTY(QString prototypeMode READ prototypeMode WRITE setPrototypeMode NOTIFY prototypeModeChanged)
    Q_PROPERTY(QPointF prototypingStartPosition READ prototypingStartPosition WRITE setPrototypingStartPosition NOTIFY prototypingStartPositionChanged)
    Q_PROPERTY(qreal prototypingStartZoom READ prototypingStartZoom WRITE setPrototypingStartZoom NOTIFY prototypingStartZoomChanged)
    
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
    
    // Prototype mode (Web, Mobile, etc.)
    QString prototypeMode() const { return m_prototypeMode; }
    void setPrototypeMode(const QString& mode);
    
    // Start position for returning after prototyping
    QPointF prototypingStartPosition() const { return m_prototypingStartPosition; }
    void setPrototypingStartPosition(const QPointF& position);
    
    qreal prototypingStartZoom() const { return m_prototypingStartZoom; }
    void setPrototypingStartZoom(qreal zoom);
    
    // Helper methods
    Q_INVOKABLE void startPrototyping(const QPointF& canvasCenter, qreal currentZoom);
    Q_INVOKABLE void stopPrototyping();
    Q_INVOKABLE QRectF calculateViewportForMode(const QString& mode) const;
    Q_INVOKABLE bool isElementInViewableArea(qreal x, qreal y, qreal width, qreal height) const;
    
signals:
    void isPrototypingChanged();
    void viewableAreaChanged();
    void prototypeModeChanged();
    void prototypingStartPositionChanged();
    void prototypingStartZoomChanged();
    void prototypingStarted();
    void prototypingStopped();
    
private:
    ElementModel& m_elementModel;
    SelectionManager& m_selectionManager;
    
    bool m_isPrototyping = false;
    QRectF m_viewableArea;
    QString m_prototypeMode = "Web";
    QPointF m_prototypingStartPosition;
    qreal m_prototypingStartZoom = 1.0;
    
    // Default viewable area dimensions for different modes
    static constexpr qreal WEB_WIDTH = 400.0;
    static constexpr qreal WEB_HEIGHT = 400.0;
    static constexpr qreal MOBILE_WIDTH = 375.0;
    static constexpr qreal MOBILE_HEIGHT = 667.0;
};