#include "Edge.h"
#include <QPainterPath>
#include <QtMath>

Edge::Edge(const QString &id, QObject *parent)
    : ScriptElement(id, parent)
    , m_sourcePortIndex(-1)
    , m_targetPortIndex(-1)
    , m_edgeColor("#94A3B8")  // Default slate color
    , m_edgeWidth(2.0)
    , m_isActive(false)
    , m_sourceHandleType("right")
    , m_targetHandleType("left")
    , m_sourcePortType("Flow")
    , m_targetPortType("Flow")
{
    // Set element type
    elementType = Element::EdgeType;
    
    // Set default name using last 4 digits of ID
    setName(QString("Edge %1").arg(id.right(4)));
    
    // Set object name for type identification
    setObjectName("Edge");
    
    // Initialize with a bounding box that will be updated based on endpoints
    setWidth(1);
    setHeight(1);
}

void Edge::setSourceNodeId(const QString &nodeId)
{
    if (m_sourceNodeId != nodeId) {
        m_sourceNodeId = nodeId;
        emit sourceNodeIdChanged();
        emit elementChanged();
        updateGeometry();
    }
}

void Edge::setTargetNodeId(const QString &nodeId)
{
    if (m_targetNodeId != nodeId) {
        m_targetNodeId = nodeId;
        emit targetNodeIdChanged();
        emit elementChanged();
        updateGeometry();
    }
}

void Edge::setSourcePortIndex(int index)
{
    if (m_sourcePortIndex != index) {
        m_sourcePortIndex = index;
        emit sourcePortIndexChanged();
        emit elementChanged();
        updateGeometry();
    }
}

void Edge::setTargetPortIndex(int index)
{
    if (m_targetPortIndex != index) {
        m_targetPortIndex = index;
        emit targetPortIndexChanged();
        emit elementChanged();
        updateGeometry();
    }
}

void Edge::setEdgeColor(const QColor &color)
{
    if (m_edgeColor != color) {
        m_edgeColor = color;
        emit edgeColorChanged();
        emit elementChanged();
    }
}

void Edge::setEdgeWidth(qreal width)
{
    if (m_edgeWidth != width) {
        m_edgeWidth = width;
        emit edgeWidthChanged();
        emit elementChanged();
    }
}

void Edge::setIsActive(bool active)
{
    if (m_isActive != active) {
        m_isActive = active;
        emit isActiveChanged();
    }
}

void Edge::setSourcePoint(const QPointF &point)
{
    if (m_sourcePoint != point) {
        m_sourcePoint = point;
        emit sourcePointChanged();
        updateControlPoints();
        updateGeometry();
        emit elementChanged();
    }
}

void Edge::setTargetPoint(const QPointF &point)
{
    if (m_targetPoint != point) {
        m_targetPoint = point;
        emit targetPointChanged();
        updateControlPoints();
        updateGeometry();
        emit elementChanged();
    }
}

void Edge::setSourceHandleType(const QString &type)
{
    if (m_sourceHandleType != type) {
        m_sourceHandleType = type;
        emit sourceHandleTypeChanged();
        updateControlPoints();
        emit elementChanged();
    }
}

void Edge::setTargetHandleType(const QString &type)
{
    if (m_targetHandleType != type) {
        m_targetHandleType = type;
        emit targetHandleTypeChanged();
        updateControlPoints();
        emit elementChanged();
    }
}

bool Edge::isConnected() const
{
    return !m_sourceNodeId.isEmpty() && !m_targetNodeId.isEmpty() &&
           m_sourcePortIndex >= 0 && m_targetPortIndex >= 0;
}

bool Edge::isPartiallyConnected() const
{
    return (!m_sourceNodeId.isEmpty() && m_sourcePortIndex >= 0) ||
           (!m_targetNodeId.isEmpty() && m_targetPortIndex >= 0);
}

void Edge::updateGeometry()
{
    // Update the bounding box to encompass the edge and its control points
    if (m_sourcePoint.isNull() || m_targetPoint.isNull()) {
        return;
    }
    
    // Calculate bounding box that includes all points
    qreal minX = qMin(qMin(m_sourcePoint.x(), m_targetPoint.x()), 
                      qMin(m_controlPoint1.x(), m_controlPoint2.x()));
    qreal maxX = qMax(qMax(m_sourcePoint.x(), m_targetPoint.x()), 
                      qMax(m_controlPoint1.x(), m_controlPoint2.x()));
    qreal minY = qMin(qMin(m_sourcePoint.y(), m_targetPoint.y()), 
                      qMin(m_controlPoint1.y(), m_controlPoint2.y()));
    qreal maxY = qMax(qMax(m_sourcePoint.y(), m_targetPoint.y()), 
                      qMax(m_controlPoint1.y(), m_controlPoint2.y()));
    
    // Add some padding for the edge width and selection
    qreal padding = m_edgeWidth + 5;
    setX(minX - padding);
    setY(minY - padding);
    setWidth(maxX - minX + 2 * padding);
    setHeight(maxY - minY + 2 * padding);
    
    emit geometryChanged();
}

void Edge::updateControlPoints()
{
    // Calculate bezier control points based on handle types and positions
    qreal horizontalOffset = qAbs(m_targetPoint.x() - m_sourcePoint.x()) * 0.5;
    horizontalOffset = qMax(horizontalOffset, 50.0); // Minimum offset for nice curves
    
    // Control point 1 extends from source
    if (m_sourceHandleType == "right") {
        m_controlPoint1 = QPointF(m_sourcePoint.x() + horizontalOffset, m_sourcePoint.y());
    } else {
        m_controlPoint1 = QPointF(m_sourcePoint.x() - horizontalOffset, m_sourcePoint.y());
    }
    
    // Control point 2 extends from target
    if (m_targetHandleType == "left") {
        m_controlPoint2 = QPointF(m_targetPoint.x() - horizontalOffset, m_targetPoint.y());
    } else {
        m_controlPoint2 = QPointF(m_targetPoint.x() + horizontalOffset, m_targetPoint.y());
    }
    
    emit controlPoint1Changed();
    emit controlPoint2Changed();
}

bool Edge::containsPoint(const QPointF &point) const
{
    // If we don't have valid endpoints, fall back to bounding box
    if (m_sourcePoint.isNull() || m_targetPoint.isNull()) {
        return rect().contains(point);
    }
    
    // Create a bezier path for the edge
    QPainterPath path;
    path.moveTo(m_sourcePoint);
    path.cubicTo(m_controlPoint1, m_controlPoint2, m_targetPoint);
    
    // Create a stroker to expand the path by the edge width plus some tolerance
    QPainterPathStroker stroker;
    stroker.setWidth(m_edgeWidth + 10); // Add 10px tolerance for easier clicking
    QPainterPath strokedPath = stroker.createStroke(path);
    
    // Check if the point is within the stroked path
    return strokedPath.contains(point);
}

void Edge::setSourcePortType(const QString &type)
{
    if (m_sourcePortType != type) {
        m_sourcePortType = type;
        emit sourcePortTypeChanged();
        emit elementChanged();
    }
}

void Edge::setTargetPortType(const QString &type)
{
    if (m_targetPortType != type) {
        m_targetPortType = type;
        emit targetPortTypeChanged();
        emit elementChanged();
    }
}