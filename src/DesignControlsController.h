#pragma once
#include <QObject>
#include <QPointF>

class Application;
class DesignCanvas;
class SelectionManager;
class ElementModel;

class DesignControlsController : public QObject {
    Q_OBJECT
    
    // Control state properties
    Q_PROPERTY(bool isResizingEnabled READ isResizingEnabled NOTIFY isResizingEnabledChanged)
    Q_PROPERTY(bool isMovementEnabled READ isMovementEnabled NOTIFY isMovementEnabledChanged)
    Q_PROPERTY(bool isAnyTextEditing READ isAnyTextEditing NOTIFY isAnyTextEditingChanged)
    Q_PROPERTY(bool isAnimating READ isAnimating NOTIFY isAnimatingChanged)
    Q_PROPERTY(bool isPrototyping READ isPrototyping NOTIFY isPrototypingChanged)
    
public:
    explicit DesignControlsController(Application* app, QObject* parent = nullptr);
    ~DesignControlsController() = default;
    
    // Control state accessors
    bool isResizingEnabled() const;
    bool isMovementEnabled() const;
    bool isAnyTextEditing() const;
    bool isAnimating() const;
    bool isPrototyping() const;
    
    // Helper methods for coordinate transformation
    Q_INVOKABLE QPointF mapToCanvas(QObject* parent, const QPointF& point, qreal zoom, QObject* flickable, const QPointF& canvasMin) const;
    
signals:
    void isResizingEnabledChanged();
    void isMovementEnabledChanged();
    void isAnyTextEditingChanged();
    void isAnimatingChanged();
    void isPrototypingChanged();
    
private slots:
    void onActiveCanvasChanged();
    void onSelectionChanged();
    void onDesignControlsStateChanged();
    
private:
    Application* m_application;
    
    // Helper methods
    DesignCanvas* getActiveDesignCanvas() const;
    SelectionManager* getActiveSelectionManager() const;
    ElementModel* getActiveElementModel() const;
    
    // Cache for current state
    mutable bool m_cachedIsResizingEnabled = true;
    mutable bool m_cachedIsMovementEnabled = true;
    mutable bool m_cachedIsAnyTextEditing = false;
    mutable bool m_cachedIsAnimating = false;
    mutable bool m_cachedIsPrototyping = false;
    
    // Update cache and emit signals if needed
    void updateCache();
    void connectToActiveCanvas();
    void disconnectFromActiveCanvas();
    
    QMetaObject::Connection m_canvasConnection;
    QMetaObject::Connection m_selectionConnection;
    QMetaObject::Connection m_resizingDisabledConnection;
    QMetaObject::Connection m_movementDisabledConnection;
    QMetaObject::Connection m_animatingConnection;
    QMetaObject::Connection m_prototypingConnection;
};