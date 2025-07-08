#pragma once
#include "CanvasController.h"

class DesignCanvas : public CanvasController {
    Q_OBJECT
    Q_PROPERTY(QObject* hoveredElement READ hoveredElement WRITE setHoveredElement NOTIFY hoveredElementChanged)
    
public:
    explicit DesignCanvas(ElementModel& model,
                         SelectionManager& sel,
                         QObject *parent = nullptr);
    ~DesignCanvas() = default;
    
    // Hovered element management
    QObject* hoveredElement() const { return m_hoveredElement; }
    void setHoveredElement(QObject* element);
    
    // Design-specific mouse handling
    Q_INVOKABLE void updateHover(qreal x, qreal y);
    
    // Helper methods for parenting logic
    Q_INVOKABLE bool isDescendantOf(QObject* elementA, QObject* elementB) const;
    Q_INVOKABLE bool isChildOfSelected(QObject* element) const;
    Q_INVOKABLE void updateParentingDuringDrag();
    
signals:
    void hoveredElementChanged();
    
private slots:
    void onSelectionChanged();
    void onModeChanged();
    
private:
    QObject* m_hoveredElement = nullptr;
    
    // Clear hover when appropriate
    void clearHoverIfSelected();
};