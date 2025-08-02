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
    Q_PROPERTY(QPointF linePreviewPoint READ linePreviewPoint WRITE setLinePreviewPoint NOTIFY linePreviewPointChanged)
    Q_PROPERTY(bool showLinePreview READ showLinePreview WRITE setShowLinePreview NOTIFY showLinePreviewChanged)
    Q_PROPERTY(int hoveredJointIndex READ hoveredJointIndex WRITE setHoveredJointIndex NOTIFY hoveredJointIndexChanged)
    Q_PROPERTY(bool isEditingShape READ isEditingShape WRITE setIsEditingShape NOTIFY isEditingShapeChanged)
    Q_PROPERTY(bool isShapeControlDragging READ isShapeControlDragging WRITE setIsShapeControlDragging NOTIFY isShapeControlDraggingChanged)

public:
    explicit ShapeControlsController(QObject *parent = nullptr);
    ~ShapeControlsController() = default;

    // Property getters
    Shape* selectedShape() const { return m_selectedShape; }
    int selectedJointIndex() const { return m_selectedJointIndex; }
    bool isDragging() const { return m_isDragging; }
    QPointF dragStartPos() const { return m_dragStartPos; }
    QPointF linePreviewPoint() const { return m_linePreviewPoint; }
    bool showLinePreview() const { return m_showLinePreview; }
    int hoveredJointIndex() const { return m_hoveredJointIndex; }
    bool isEditingShape() const { return m_isEditingShape; }
    bool isShapeControlDragging() const { return m_isShapeControlDragging; }

    // Property setters
    void setSelectedShape(Shape* shape);
    Q_INVOKABLE void setSelectedJointIndex(int index);
    void setIsDragging(bool dragging);
    void setDragStartPos(const QPointF& pos);
    void setLinePreviewPoint(const QPointF& point);
    void setShowLinePreview(bool show);
    Q_INVOKABLE void setHoveredJointIndex(int index);
    Q_INVOKABLE void setIsEditingShape(bool editing);
    Q_INVOKABLE void setIsShapeControlDragging(bool dragging);

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
    void linePreviewPointChanged();
    void showLinePreviewChanged();
    void hoveredJointIndexChanged();
    void isEditingShapeChanged();
    void isShapeControlDraggingChanged();

private:
    Shape* m_selectedShape = nullptr;
    int m_selectedJointIndex = -1;
    bool m_isDragging = false;
    QPointF m_dragStartPos;
    QPointF m_linePreviewPoint;
    bool m_showLinePreview = false;
    int m_hoveredJointIndex = -1;
    bool m_isEditingShape = false;
    bool m_isShapeControlDragging = false;
    
    // Store original state for undo
    QList<QPointF> m_originalJoints;
    QRectF m_originalBounds;
};