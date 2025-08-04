#include "Shape.h"
#include "PropertyRegistry.h"
#include <QVariantMap>
#include <QDebug>

Shape::Shape(const QString &id, QObject *parent)
    : DesignElement(id, parent)
{
    elementType = Element::ShapeType;
    // Don't initialize joints here - wait until we have a size
    
    // Register properties
    registerProperties();
    
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
    for (const Joint& joint : m_joints) {
        result.append(joint.toVariantMap());
    }
    return result;
}

void Shape::setJoints(const QList<Joint>& joints)
{
    m_joints = joints;
    emit jointsChanged();
    // Notify property system that joints have changed
    if (m_properties) {
        m_properties->set("joints", this->joints());
    }
}

void Shape::setJoints(const QVariantList& joints)
{
    QList<Joint> newJoints;
    for (const QVariant& joint : joints) {
        if (joint.canConvert<QVariantMap>()) {
            QVariantMap jointMap = joint.toMap();
            newJoints.append(Joint::fromVariantMap(jointMap));
        }
    }
    setJoints(newJoints);
}

void Shape::setJointPositions(const QList<QPointF>& positions)
{
    QList<Joint> newJoints;
    for (int i = 0; i < positions.size(); ++i) {
        Joint joint;
        // Preserve existing joint properties if available
        if (i < m_joints.size()) {
            joint = m_joints[i];  // Copy existing joint properties
        }
        joint.position = positions[i];  // Update only the position
        newJoints.append(joint);
    }
    setJoints(newJoints);
}

QVariantList Shape::edges() const
{
    QVariantList result;
    for (const Edge& edge : m_edges) {
        result.append(edge.toVariantMap());
    }
    return result;
}

void Shape::setEdges(const QList<Edge>& edges)
{
    m_edges = edges;
    emit edgesChanged();
    // Notify property system that edges have changed
    if (m_properties) {
        m_properties->set("edges", this->edges());
    }
}

void Shape::setEdges(const QVariantList& edges)
{
    QList<Edge> newEdges;
    for (const QVariant& edge : edges) {
        if (edge.canConvert<QVariantMap>()) {
            QVariantMap edgeMap = edge.toMap();
            newEdges.append(Edge::fromVariantMap(edgeMap));
        }
    }
    setEdges(newEdges);
}

void Shape::addEdge(int fromIndex, int toIndex)
{
    if (fromIndex >= 0 && fromIndex < m_joints.size() && 
        toIndex >= 0 && toIndex < m_joints.size() && 
        fromIndex != toIndex) {
        Edge edge;
        edge.fromIndex = fromIndex;
        edge.toIndex = toIndex;
        m_edges.append(edge);
        emit edgesChanged();
        // Notify property system that edges have changed
        if (m_properties) {
            m_properties->set("edges", this->edges());
        }
    }
}

void Shape::setJointMirroring(int jointIndex, MirroringType mirroring)
{
    if (jointIndex >= 0 && jointIndex < m_joints.size()) {
        if (m_joints[jointIndex].mirroring != mirroring) {
            m_joints[jointIndex].mirroring = mirroring;
            emit jointsChanged();
        }
    }
}

void Shape::setJointCornerRadius(int jointIndex, qreal radius)
{
    if (jointIndex >= 0 && jointIndex < m_joints.size()) {
        if (!qFuzzyCompare(m_joints[jointIndex].cornerRadius, radius)) {
            m_joints[jointIndex].cornerRadius = radius;
            emit jointsChanged();
        }
    }
}

int Shape::getJointMirroring(int jointIndex) const
{
    if (jointIndex >= 0 && jointIndex < m_joints.size()) {
        return static_cast<int>(m_joints[jointIndex].mirroring);
    }
    return static_cast<int>(NoMirroring);
}

qreal Shape::getJointCornerRadius(int jointIndex) const
{
    if (jointIndex >= 0 && jointIndex < m_joints.size()) {
        return m_joints[jointIndex].cornerRadius;
    }
    return 0.0;
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
        case Pen:
            updatePenJoints();
            break;
    }
}

void Shape::updateSquareJoints()
{
    QList<Joint> newJoints;
    // Four corners of the square in normalized coordinates
    Joint topLeft;
    topLeft.position = QPointF(0.0, 0.0);
    newJoints.append(topLeft);
    
    Joint topRight;
    topRight.position = QPointF(1.0, 0.0);
    newJoints.append(topRight);
    
    Joint bottomRight;
    bottomRight.position = QPointF(1.0, 1.0);
    newJoints.append(bottomRight);
    
    Joint bottomLeft;
    bottomLeft.position = QPointF(0.0, 1.0);
    newJoints.append(bottomLeft);
    
    setJoints(newJoints);
}

void Shape::updateTriangleJoints()
{
    QList<Joint> newJoints;
    // Three corners of the triangle (isosceles triangle pointing up) in normalized coordinates
    Joint top;
    top.position = QPointF(0.5, 0.0);
    newJoints.append(top);
    
    Joint bottomRight;
    bottomRight.position = QPointF(1.0, 1.0);
    newJoints.append(bottomRight);
    
    Joint bottomLeft;
    bottomLeft.position = QPointF(0.0, 1.0);
    newJoints.append(bottomLeft);
    
    setJoints(newJoints);
}

void Shape::updatePenJoints()
{
    // Check if we have square corner joints that need to be replaced with line joints
    bool hasSquareCornerJoints = (m_joints.size() == 4 &&
                                  m_joints[0].position == QPointF(0.0, 0.0) &&
                                  m_joints[1].position == QPointF(1.0, 0.0) &&
                                  m_joints[2].position == QPointF(1.0, 1.0) &&
                                  m_joints[3].position == QPointF(0.0, 1.0));
    
    // For pens, set default joints if we don't have any joints yet OR if we have square corner joints
    // This preserves custom joints created by PenModeHandler or loaded from file, but replaces square corner joints
    if (m_joints.isEmpty()) {
        QList<Joint> newJoints;
        // Two endpoints of the pen (diagonal from top-left to bottom-right) in normalized coordinates
        Joint start;
        start.position = QPointF(0.0, 0.0);
        newJoints.append(start);
        
        Joint end;
        end.position = QPointF(1.0, 1.0);
        newJoints.append(end);
        
        setJoints(newJoints);
    } else if (hasSquareCornerJoints) {
        QList<Joint> newJoints;
        // Two endpoints of the pen (diagonal from top-left to bottom-right) in normalized coordinates
        Joint start;
        start.position = QPointF(0.0, 0.0);
        newJoints.append(start);
        
        Joint end;
        end.position = QPointF(1.0, 1.0);
        newJoints.append(end);
        
        setJoints(newJoints);
    } else {
        // Keep existing joints
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
        case Pen:
            shapeName = "Pen";
            break;
    }
    
    // Use last 4 digits of ID for consistency with other elements
    setName(QString("%1 %2").arg(shapeName).arg(getId().right(4)));
}

void Shape::registerProperties() {
    // Call parent implementation first
    DesignElement::registerProperties();
    
    // Register Shape-specific properties
    m_properties->registerProperty("shapeType", static_cast<int>(Square));
    m_properties->registerProperty("joints", QVariantList());
    m_properties->registerProperty("edges", QVariantList());
    m_properties->registerProperty("edgeWidth", 2.0);
    m_properties->registerProperty("edgeColor", QColor(0, 0, 0, 255)); // Black
    m_properties->registerProperty("fillColor", QColor(0, 120, 255, 255)); // Blue
    m_properties->registerProperty("lineJoin", QString("miter"));
    m_properties->registerProperty("lineCap", QString("round"));
}

QVariant Shape::getProperty(const QString& name) const {
    // Handle Shape-specific properties
    if (name == "shapeType") return static_cast<int>(shapeType());
    if (name == "joints") return joints();
    if (name == "edges") return edges();
    if (name == "edgeWidth") return edgeWidth();
    if (name == "edgeColor") return edgeColor();
    if (name == "fillColor") return fillColor();
    if (name == "lineJoin") return lineJoin();
    if (name == "lineCap") return lineCap();
    
    // Fall back to parent implementation
    return DesignElement::getProperty(name);
}

void Shape::setProperty(const QString& name, const QVariant& value) {
    // Handle Shape-specific properties
    if (name == "shapeType") {
        setShapeType(static_cast<ShapeType>(value.toInt()));
        return;
    }
    if (name == "joints") {
        setJoints(value.toList());
        return;
    }
    if (name == "edges") {
        setEdges(value.toList());
        return;
    }
    if (name == "edgeWidth") {
        setEdgeWidth(value.toReal());
        return;
    }
    if (name == "edgeColor") {
        setEdgeColor(value.value<QColor>());
        return;
    }
    if (name == "fillColor") {
        setFillColor(value.value<QColor>());
        return;
    }
    if (name == "lineJoin") {
        setLineJoin(value.toString());
        return;
    }
    if (name == "lineCap") {
        setLineCap(value.toString());
        return;
    }
    
    // Fall back to parent implementation
    DesignElement::setProperty(name, value);
}

QList<PropertyDefinition> Shape::staticPropertyDefinitions() {
    QList<PropertyDefinition> props;
    
    // Shape properties
    props.append(PropertyDefinition("shapeType", QMetaType::Int, static_cast<int>(Shape::Square), PropertyDefinition::Appearance));
    props.append(PropertyDefinition("joints", QMetaType::QVariantList, QVariantList(), PropertyDefinition::Advanced, false)); // Not editable
    props.append(PropertyDefinition("edges", QMetaType::QVariantList, QVariantList(), PropertyDefinition::Advanced, false)); // Not editable
    
    // Appearance properties
    props.append(PropertyDefinition("edgeWidth", QMetaType::Double, 2.0, PropertyDefinition::Appearance));
    props.append(PropertyDefinition("edgeColor", QMetaType::QColor, QColor(0, 0, 0, 255), PropertyDefinition::Appearance));
    props.append(PropertyDefinition("fillColor", QMetaType::QColor, QColor(0, 120, 255, 255), PropertyDefinition::Appearance));
    
    // Line style properties
    props.append(PropertyDefinition("lineJoin", QMetaType::QString, QString("miter"), PropertyDefinition::Appearance));
    props.append(PropertyDefinition("lineCap", QMetaType::QString, QString("round"), PropertyDefinition::Appearance));
    
    return props;
}

void Shape::setLineJoin(const QString& lineJoin)
{
    if (m_lineJoin != lineJoin) {
        m_lineJoin = lineJoin;
        emit lineJoinChanged();
    }
}

void Shape::setLineCap(const QString& lineCap)
{
    if (m_lineCap != lineCap) {
        m_lineCap = lineCap;
        emit lineCapChanged();
    }
}