#include "Edge.h"

Edge::Edge(const QString &id, QObject *parent)
    : Element(Element::EdgeType, id, parent)
    , m_sourcePortIndex(-1)
    , m_targetPortIndex(-1)
    , m_edgeColor("#94A3B8")  // Default slate color
    , m_edgeWidth(2.0)
    , m_isActive(false)
{
    // Edges don't have a traditional position/size, they're defined by their endpoints
    setWidth(0);
    setHeight(0);
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
        emit elementChanged();
    }
}

void Edge::setTargetPoint(const QPointF &point)
{
    if (m_targetPoint != point) {
        m_targetPoint = point;
        emit targetPointChanged();
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
    // This will be called when nodes move or ports change
    // The actual endpoint calculation will be done in QML based on node positions
    // For now, we just emit the changed signals
    emit geometryChanged();
}