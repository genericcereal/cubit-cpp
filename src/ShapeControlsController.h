#pragma once

#include <QObject>
#include <QPointF>
#include <QRectF>
#include <QList>
#include <QVariant>
#include "Shape.h"

class ElementModel;
class SelectionManager;

class ShapeControlsController : public QObject
{
    Q_OBJECT
    Q_PROPERTY(Shape* selectedShape READ selectedShape WRITE setSelectedShape NOTIFY selectedShapeChanged)
    Q_PROPERTY(int selectedJointIndex READ selectedJointIndex WRITE setSelectedJointIndex NOTIFY selectedJointIndexChanged)
    Q_PROPERTY(bool isDragging READ isDragging WRITE setIsDragging NOTIFY isDraggingChanged)
    Q_PROPERTY(QPointF dragStartPos READ dragStartPos WRITE setDragStartPos NOTIFY dragStartPosChanged)

public:
    explicit ShapeControlsController(QObject *parent = nullptr);
    ~ShapeControlsController() = default;

    // Property getters
    Shape* selectedShape() const { return m_selectedShape; }
    int selectedJointIndex() const { return m_selectedJointIndex; }
    bool isDragging() const { return m_isDragging; }
    QPointF dragStartPos() const { return m_dragStartPos; }

    // Property setters
    void setSelectedShape(Shape* shape);
    void setSelectedJointIndex(int index);
    void setIsDragging(bool dragging);
    void setDragStartPos(const QPointF& pos);

    // Joint manipulation
    Q_INVOKABLE void startJointDrag(int jointIndex, const QPointF& startPos);
    Q_INVOKABLE void updateJointPosition(const QPointF& canvasPos);
    Q_INVOKABLE void endJointDrag();

    // Shape manipulation
    Q_INVOKABLE void startShapeMove(const QPointF& startPos);
    Q_INVOKABLE void updateShapePosition(const QPointF& delta);
    Q_INVOKABLE void endShapeMove();

signals:
    void selectedShapeChanged();
    void selectedJointIndexChanged();
    void isDraggingChanged();
    void dragStartPosChanged();

private:
    Shape* m_selectedShape = nullptr;
    int m_selectedJointIndex = -1;
    bool m_isDragging = false;
    QPointF m_dragStartPos;
    
    // Store original state for undo
    QList<QPointF> m_originalJoints;
    QRectF m_originalBounds;
};