#pragma once
#include <QObject>
#include <QPointF>

// Forward declarations
class Application;
class DesignCanvas;
class SelectionManager;
class ElementModel;

// Forward declare Project for Q_PROPERTY
class Project;
Q_DECLARE_OPAQUE_POINTER(Project*)

class DesignControlsController : public QObject {
    Q_OBJECT
    
    // Control state properties
    Q_PROPERTY(bool isResizingEnabled READ isResizingEnabled NOTIFY isResizingEnabledChanged)
    Q_PROPERTY(bool isMovementEnabled READ isMovementEnabled NOTIFY isMovementEnabledChanged)
    Q_PROPERTY(bool isAnyTextEditing READ isAnyTextEditing NOTIFY isAnyTextEditingChanged)
    Q_PROPERTY(bool isAnimating READ isAnimating NOTIFY isAnimatingChanged)
    Q_PROPERTY(bool isPrototyping READ isPrototyping NOTIFY isPrototypingChanged)
    
    // Project property for per-window instances
    Q_PROPERTY(Project* project READ project WRITE setProject NOTIFY projectChanged)
    
public:
    // Default constructor for QML
    explicit DesignControlsController(QObject* parent = nullptr);
    // Constructor that takes a Project for per-window instance
    explicit DesignControlsController(Project* project, QObject* parent = nullptr);
    // Legacy constructor for global instance (to be removed)
    explicit DesignControlsController(Application* app, QObject* parent = nullptr);
    ~DesignControlsController() = default;
    
    // Control state accessors
    bool isResizingEnabled() const;
    bool isMovementEnabled() const;
    bool isAnyTextEditing() const;
    bool isAnimating() const;
    bool isPrototyping() const;
    
    // Project property accessors
    Project* project() const { return m_project; }
    void setProject(Project* project);
    
    // Helper methods for coordinate transformation
    Q_INVOKABLE QPointF mapToCanvas(QObject* parent, const QPointF& point, qreal zoom, QObject* flickable, const QPointF& canvasMin) const;
    
signals:
    void isResizingEnabledChanged();
    void isMovementEnabledChanged();
    void isAnyTextEditingChanged();
    void isAnimatingChanged();
    void isPrototypingChanged();
    void projectChanged();
    
private slots:
    void onSelectionChanged();
    void onDesignControlsStateChanged();
    
private:
    Application* m_application = nullptr;  // For legacy global instance
    Project* m_project = nullptr;          // For per-window instance
    
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