#pragma once
#include "CanvasController.h"

class DesignCanvas : public CanvasController {
    Q_OBJECT
    Q_PROPERTY(QObject* hoveredElement READ hoveredElement WRITE setHoveredElement NOTIFY hoveredElementChanged)
    Q_PROPERTY(bool isAnimating READ isAnimating WRITE setIsAnimating NOTIFY isAnimatingChanged)
    
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
    
    // Design-specific mouse handling
    Q_INVOKABLE void updateHover(qreal x, qreal y);
    
    // Helper methods for parenting logic
    Q_INVOKABLE bool isDescendantOf(QObject* elementA, QObject* elementB) const;
    Q_INVOKABLE bool isChildOfSelected(QObject* element) const;
    Q_INVOKABLE void updateParentingDuringDrag();
    
signals:
    void hoveredElementChanged();
    void isAnimatingChanged();
    
private slots:
    void onSelectionChanged();
    void onModeChanged();
    
private:
    QObject* m_hoveredElement = nullptr;
    bool m_isAnimating = false;
    
    // Clear hover when appropriate
    void clearHoverIfSelected();
};