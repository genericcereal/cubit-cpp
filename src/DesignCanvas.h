#pragma once
#include "CanvasController.h"

class DesignCanvas : public CanvasController {
    Q_OBJECT
    Q_PROPERTY(QObject* hoveredElement READ hoveredElement WRITE setHoveredElement NOTIFY hoveredElementChanged)
    Q_PROPERTY(bool isAnimating READ isAnimating WRITE setIsAnimating NOTIFY isAnimatingChanged)
    Q_PROPERTY(bool isDesignControlsResizingDisabled READ isDesignControlsResizingDisabled WRITE setIsDesignControlsResizingDisabled NOTIFY isDesignControlsResizingDisabledChanged)
    Q_PROPERTY(bool isDesignControlsMovementDisabled READ isDesignControlsMovementDisabled WRITE setIsDesignControlsMovementDisabled NOTIFY isDesignControlsMovementDisabledChanged)
    Q_PROPERTY(bool isEditingShape READ isEditingShape WRITE setIsEditingShape NOTIFY isEditingShapeChanged)
    
public:
    explicit DesignCanvas(ElementModel& model,
                         SelectionManager& sel,
                         QObject *parent = nullptr);
    ~DesignCanvas() = default;
    
    // Hovered element management
    QObject* hoveredElement() const { return m_hoveredElement; }
    void setHoveredElement(QObject* element);
    
    // Animation state management
    bool isAnimating() const { return m_isAnimating; }
    void setIsAnimating(bool animating);
    
    // Design controls state management
    bool isDesignControlsResizingDisabled() const { return m_isDesignControlsResizingDisabled; }
    void setIsDesignControlsResizingDisabled(bool disabled);
    
    bool isDesignControlsMovementDisabled() const { return m_isDesignControlsMovementDisabled; }
    void setIsDesignControlsMovementDisabled(bool disabled);
    
    bool isEditingShape() const { return m_isEditingShape; }
    void setIsEditingShape(bool editing);
    
    // Design-specific mouse handling
    Q_INVOKABLE void updateHover(qreal x, qreal y);
    
    // Helper methods for parenting logic
    Q_INVOKABLE bool isDescendantOf(QObject* elementA, QObject* elementB) const;
    Q_INVOKABLE bool isChildOfSelected(QObject* element) const;
    Q_INVOKABLE void updateParentingDuringDrag();
    
signals:
    void hoveredElementChanged();
    void isAnimatingChanged();
    void isDesignControlsResizingDisabledChanged();
    void isDesignControlsMovementDisabledChanged();
    void isEditingShapeChanged();
    
private slots:
    void onSelectionChanged();
    void onModeChanged();
    void onElementAdded(Element* element);
    
private:
    QObject* m_hoveredElement = nullptr;
    bool m_isAnimating = false;
    bool m_isDesignControlsResizingDisabled = false;
    bool m_isDesignControlsMovementDisabled = false;
    bool m_isEditingShape = false;
    
    // Clear hover when appropriate
    void clearHoverIfSelected();
};