#include "Shape.h"
#include <QVariantMap>
#include <QDebug>

Shape::Shape(const QString &id, QObject *parent)
    : DesignElement(id, parent)
{
    elementType = Element::ShapeType;
    // Don't initialize joints here - wait until we have a size
    
    // Set default name based on shape type
    updateName();
}

void Shape::setShapeType(ShapeType type)
{
    if (m_shapeType != type) {
        m_shapeType = type;
        updateJointsForShape();
        updateName();
        emit shapeTypeChanged();
    }
}

QVariantList Shape::joints() const
{
    QVariantList result;
    for (int i = 0; i < m_joints.size(); ++i) {
        const QPointF& point = m_joints[i];
        // Joints are already in normalized coordinates (0-1 range)
        QVariantMap jointMap;
        jointMap["x"] = point.x();
        jointMap["y"] = point.y();
        result.append(jointMap);
    }
    return result;
}

void Shape::setJoints(const QList<QPointF>& joints)
{
    if (m_joints != joints) {
        m_joints = joints;
        emit jointsChanged();
    }
}

void Shape::setJoints(const QVariantList& joints)
{
    QList<QPointF> newJoints;
    for (const QVariant& joint : joints) {
        if (joint.canConvert<QPointF>()) {
            newJoints.append(joint.toPointF());
        } else if (joint.canConvert<QVariantMap>()) {
            // Handle QVariantMap with "x" and "y" keys (from deserialization)
            QVariantMap jointMap = joint.toMap();
            if (jointMap.contains("x") && jointMap.contains("y")) {
                qreal x = jointMap["x"].toDouble();
                qreal y = jointMap["y"].toDouble();
                newJoints.append(QPointF(x, y));
            }
        }
    }
    setJoints(newJoints);
}

void Shape::setEdgeWidth(qreal width)
{
    if (!qFuzzyCompare(m_edgeWidth, width)) {
        m_edgeWidth = width;
        emit edgeWidthChanged();
    }
}

void Shape::setEdgeColor(const QColor& color)
{
    if (m_edgeColor != color) {
        m_edgeColor = color;
        emit edgeColorChanged();
    }
}

void Shape::setFillColor(const QColor& color)
{
    if (m_fillColor != color) {
        m_fillColor = color;
        emit fillColorChanged();
    }
}


void Shape::setWidth(qreal w)
{
    DesignElement::setWidth(w);
    updateJointsForShape();
}

void Shape::setHeight(qreal h)
{
    DesignElement::setHeight(h);
    updateJointsForShape();
}

void Shape::setRect(const QRectF &rect)
{
    DesignElement::setRect(rect);
    updateJointsForShape();
}

void Shape::initializeShape()
{
    updateJointsForShape();
}

void Shape::updateJointsForShape()
{
    // Don't update joints if we have no size
    if (width() <= 0 || height() <= 0) {
        return;
    }
    
    switch (m_shapeType) {
        case Square:
            updateSquareJoints();
            break;
        case Triangle:
            updateTriangleJoints();
            break;
        case Line:
            updateLineJoints();
            break;
    }
}

void Shape::updateSquareJoints()
{
    QList<QPointF> newJoints;
    // Four corners of the square in normalized coordinates
    newJoints.append(QPointF(0.0, 0.0));    // Top-left
    newJoints.append(QPointF(1.0, 0.0));    // Top-right
    newJoints.append(QPointF(1.0, 1.0));    // Bottom-right
    newJoints.append(QPointF(0.0, 1.0));    // Bottom-left
    setJoints(newJoints);
}

void Shape::updateTriangleJoints()
{
    QList<QPointF> newJoints;
    // Three corners of the triangle (isosceles triangle pointing up) in normalized coordinates
    newJoints.append(QPointF(0.5, 0.0));    // Top center
    newJoints.append(QPointF(1.0, 1.0));    // Bottom-right
    newJoints.append(QPointF(0.0, 1.0));    // Bottom-left
    setJoints(newJoints);
}

void Shape::updateLineJoints()
{
    
    // Check if we have square corner joints that need to be replaced with line joints
    bool hasSquareCornerJoints = (m_joints.size() == 4 &&
                                  m_joints[0] == QPointF(0.0, 0.0) &&
                                  m_joints[1] == QPointF(1.0, 0.0) &&
                                  m_joints[2] == QPointF(1.0, 1.0) &&
                                  m_joints[3] == QPointF(0.0, 1.0));
    
    // For lines, set default joints if we don't have any joints yet OR if we have square corner joints
    // This preserves custom joints created by LineModeHandler or loaded from file, but replaces square corner joints
    if (m_joints.isEmpty()) {
        QList<QPointF> newJoints;
        // Two endpoints of the line (diagonal from top-left to bottom-right) in normalized coordinates
        newJoints.append(QPointF(0.0, 0.0));    // Start point
        newJoints.append(QPointF(1.0, 1.0));    // End point
        setJoints(newJoints);
    } else if (hasSquareCornerJoints) {
        QList<QPointF> newJoints;
        // Two endpoints of the line (diagonal from top-left to bottom-right) in normalized coordinates
        newJoints.append(QPointF(0.0, 0.0));    // Start point
        newJoints.append(QPointF(1.0, 1.0));    // End point
        setJoints(newJoints);
    } else {
    }
}

void Shape::updateName()
{
    QString shapeName;
    switch (m_shapeType) {
        case Square:
            shapeName = "Square";
            break;
        case Triangle:
            shapeName = "Triangle";
            break;
        case Line:
            shapeName = "Line";
            break;
    }
    
    // Use last 4 digits of ID for consistency with other elements
    setName(QString("%1 %2").arg(shapeName).arg(getId().right(4)));
}