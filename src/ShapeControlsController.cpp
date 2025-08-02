#include "ShapeControlsController.h"
#include "Shape.h"
#include "ElementModel.h"
#include "SelectionManager.h"
#include <QDebug>

ShapeControlsController::ShapeControlsController(QObject *parent)
    : QObject(parent)
{
}

void ShapeControlsController::setSelectedShape(Shape* shape)
{
    if (m_selectedShape != shape) {
        m_selectedShape = shape;
        emit selectedShapeChanged();
    }
}

void ShapeControlsController::setSelectedJointIndex(int index)
{
    if (m_selectedJointIndex != index) {
        m_selectedJointIndex = index;
        emit selectedJointIndexChanged();
    }
}

void ShapeControlsController::setIsDragging(bool dragging)
{
    if (m_isDragging != dragging) {
        m_isDragging = dragging;
        emit isDraggingChanged();
    }
}

void ShapeControlsController::setDragStartPos(const QPointF& pos)
{
    if (m_dragStartPos != pos) {
        m_dragStartPos = pos;
        emit dragStartPosChanged();
    }
}

void ShapeControlsController::startJointDrag(int jointIndex, const QPointF& startPos)
{
    if (!m_selectedShape) {
        return;
    }
    
    // Convert QVariantList to QList<QPointF>
    QVariantList jointVariants = m_selectedShape->joints();
    if (jointIndex < 0 || jointIndex >= jointVariants.size()) {
        return;
    }
    
    // Validate start position
    if (!qIsFinite(startPos.x()) || !qIsFinite(startPos.y())) {
        return;
    }
    
    
    // Store original state - convert QVariantList to QList<QPointF>
    m_originalJoints.clear();
    for (const QVariant& variant : jointVariants) {
        QPointF pt;
        if (variant.canConvert<QPointF>()) {
            pt = variant.toPointF();
        } else if (variant.canConvert<QVariantMap>()) {
            // Handle QVariantMap with "x" and "y" keys
            QVariantMap jointMap = variant.toMap();
            if (jointMap.contains("x") && jointMap.contains("y")) {
                pt = QPointF(jointMap["x"].toDouble(), jointMap["y"].toDouble());
            }
        }
        m_originalJoints.append(pt);
    }
    m_originalBounds = QRectF(m_selectedShape->x(), m_selectedShape->y(), 
                             m_selectedShape->width(), m_selectedShape->height());
    
    setSelectedJointIndex(jointIndex);
    setDragStartPos(startPos);
    setIsDragging(true);
}

void ShapeControlsController::updateJointPosition(const QPointF& canvasPos)
{
    if (!m_isDragging || !m_selectedShape || m_selectedJointIndex < 0) {
        return;
    }

    // Validate canvas position
    if (!qIsFinite(canvasPos.x()) || !qIsFinite(canvasPos.y())) {
        return;
    }


    // Create absolute joint positions
    QList<QPointF> absoluteJoints;
    for (int i = 0; i < m_originalJoints.size(); ++i) {
        if (i == m_selectedJointIndex) {
            // Use the new canvas position for the dragged joint
            absoluteJoints.append(canvasPos);
        } else {
            // Convert normalized joint to absolute position using original bounds
            qreal jointX = m_originalBounds.x() + m_originalJoints[i].x() * m_originalBounds.width();
            qreal jointY = m_originalBounds.y() + m_originalJoints[i].y() * m_originalBounds.height();
            absoluteJoints.append(QPointF(jointX, jointY));
        }
    }

    // Calculate new bounding box
    qreal minX = absoluteJoints[0].x(), minY = absoluteJoints[0].y();
    qreal maxX = absoluteJoints[0].x(), maxY = absoluteJoints[0].y();
    
    for (const auto& joint : absoluteJoints) {
        minX = qMin(minX, joint.x());
        minY = qMin(minY, joint.y());
        maxX = qMax(maxX, joint.x());
        maxY = qMax(maxY, joint.y());
    }
    
    // Check if the shape is becoming degenerate (all points in a line)
    bool isDegenerate = (qAbs(maxX - minX) < 1.0 && qAbs(maxY - minY) < 1.0);
    if (isDegenerate) {
        return;
    }

    // Calculate new shape bounds with minimum size constraint
    qreal newX = minX;
    qreal newY = minY;
    qreal newWidth = qMax(10.0, maxX - minX);  // Minimum 10px width
    qreal newHeight = qMax(10.0, maxY - minY); // Minimum 10px height
    

    // Convert absolute joints back to normalized coordinates
    QList<QPointF> normalizedJoints;
    for (const auto& joint : absoluteJoints) {
        qreal normX = (joint.x() - newX) / newWidth;
        qreal normY = (joint.y() - newY) / newHeight;
        
        // Clamp normalized coordinates to valid range
        normX = qBound(0.0, normX, 1.0);
        normY = qBound(0.0, normY, 1.0);
        
        normalizedJoints.append(QPointF(normX, normY));
    }


    // Update shape
    m_selectedShape->setX(newX);
    m_selectedShape->setY(newY);
    m_selectedShape->setWidth(newWidth);
    m_selectedShape->setHeight(newHeight);
    m_selectedShape->setJointPositions(normalizedJoints);
}

void ShapeControlsController::endJointDrag()
{
    
    // Ensure the shape is still valid and visible
    if (m_selectedShape) {
        // Force a geometry update to ensure the shape is properly rendered
        emit m_selectedShape->geometryChanged();
    }
    
    setSelectedJointIndex(-1);
    setIsDragging(false);
    m_originalJoints.clear();
    m_originalBounds = QRectF();
}

void ShapeControlsController::startShapeMove(const QPointF& startPos)
{
    if (!m_selectedShape) {
        return;
    }

    setDragStartPos(startPos);
    setIsDragging(true);
}

void ShapeControlsController::updateShapePosition(const QPointF& delta)
{
    if (!m_isDragging || !m_selectedShape) {
        return;
    }

    m_selectedShape->setX(m_selectedShape->x() + delta.x());
    m_selectedShape->setY(m_selectedShape->y() + delta.y());
}

void ShapeControlsController::endShapeMove()
{
    setIsDragging(false);
}

void ShapeControlsController::setLinePreviewPoint(const QPointF& point)
{
    if (m_linePreviewPoint != point) {
        m_linePreviewPoint = point;
        emit linePreviewPointChanged();
    }
}

void ShapeControlsController::setShowLinePreview(bool show)
{
    if (m_showLinePreview != show) {
        m_showLinePreview = show;
        emit showLinePreviewChanged();
    }
}

void ShapeControlsController::setHoveredJointIndex(int index)
{
    if (m_hoveredJointIndex != index) {
        m_hoveredJointIndex = index;
        emit hoveredJointIndexChanged();
    }
}

void ShapeControlsController::setIsEditingShape(bool editing)
{
    if (m_isEditingShape != editing) {
        m_isEditingShape = editing;
        emit isEditingShapeChanged();
    }
}

void ShapeControlsController::setIsShapeControlDragging(bool dragging)
{
    if (m_isShapeControlDragging != dragging) {
        m_isShapeControlDragging = dragging;
        emit isShapeControlDraggingChanged();
    }
}