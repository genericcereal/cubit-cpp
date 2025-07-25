#ifndef SHAPE_H
#define SHAPE_H

#include "DesignElement.h"
#include <QPointF>
#include <QColor>
#include <QVariantList>

class Shape : public DesignElement
{
    Q_OBJECT
    Q_PROPERTY(ShapeType shapeType READ shapeType WRITE setShapeType NOTIFY shapeTypeChanged)
    Q_PROPERTY(QVariantList joints READ joints NOTIFY jointsChanged)
    Q_PROPERTY(qreal edgeWidth READ edgeWidth WRITE setEdgeWidth NOTIFY edgeWidthChanged)
    Q_PROPERTY(QColor edgeColor READ edgeColor WRITE setEdgeColor NOTIFY edgeColorChanged)
    Q_PROPERTY(QColor fillColor READ fillColor WRITE setFillColor NOTIFY fillColorChanged)

public:
    enum ShapeType {
        Square,
        Triangle,
        Line
    };
    Q_ENUM(ShapeType)

    explicit Shape(const QString &id, QObject *parent = nullptr);
    virtual ~Shape() = default;

    // Shape type
    ShapeType shapeType() const { return m_shapeType; }
    void setShapeType(ShapeType type);

    // Joints (corner points)
    QVariantList joints() const;
    QList<QPointF> jointsAsPoints() const { return m_joints; }
    void setJoints(const QList<QPointF>& joints);
    Q_INVOKABLE void setJoints(const QVariantList& joints);

    // Edge properties
    qreal edgeWidth() const { return m_edgeWidth; }
    void setEdgeWidth(qreal width);

    QColor edgeColor() const { return m_edgeColor; }
    void setEdgeColor(const QColor& color);

    // Fill properties
    QColor fillColor() const { return m_fillColor; }
    void setFillColor(const QColor& color);

    // Override geometry setters to update joints
    void setWidth(qreal w) override;
    void setHeight(qreal h) override;
    void setRect(const QRectF &rect) override;

    // Initialize shape based on type
    Q_INVOKABLE void initializeShape();

signals:
    void shapeTypeChanged();
    void jointsChanged();
    void edgeWidthChanged();
    void edgeColorChanged();
    void fillColorChanged();

private:
    void updateJointsForShape();
    void updateSquareJoints();
    void updateTriangleJoints();
    void updateLineJoints();
    void updateName();

    ShapeType m_shapeType = Square;
    QList<QPointF> m_joints;
    qreal m_edgeWidth = 2.0;
    QColor m_edgeColor = QColor(0, 0, 0, 255); // Black
    QColor m_fillColor = QColor(0, 120, 255, 255); // Blue
};

#endif // SHAPE_H