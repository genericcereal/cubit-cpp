#ifndef SHAPE_H
#define SHAPE_H

#include "DesignElement.h"
#include "PropertyDefinition.h"
#include <QPointF>
#include <QColor>
#include <QVariantList>
#include <QList>
#include <QVariantMap>

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

    enum MirroringType {
        NoMirroring,
        MirrorAngle,
        MirrorAngleAndLength
    };
    Q_ENUM(MirroringType)

    struct Joint {
        QPointF position;
        MirroringType mirroring = NoMirroring;
        qreal cornerRadius = 0.0;

        // Convert to/from QVariantMap for QML
        QVariantMap toVariantMap() const {
            QVariantMap map;
            map["x"] = position.x();
            map["y"] = position.y();
            map["mirroring"] = static_cast<int>(mirroring);
            map["cornerRadius"] = cornerRadius;
            return map;
        }

        static Joint fromVariantMap(const QVariantMap& map) {
            Joint joint;
            joint.position = QPointF(map.value("x", 0.0).toDouble(), 
                                   map.value("y", 0.0).toDouble());
            joint.mirroring = static_cast<MirroringType>(map.value("mirroring", NoMirroring).toInt());
            joint.cornerRadius = map.value("cornerRadius", 0.0).toDouble();
            return joint;
        }
    };

    explicit Shape(const QString &id, QObject *parent = nullptr);
    virtual ~Shape() = default;
    
    // Static factory support methods
    static QString staticTypeName() { return "shape"; }
    static QList<PropertyDefinition> staticPropertyDefinitions();

    // Shape type
    ShapeType shapeType() const { return m_shapeType; }
    void setShapeType(ShapeType type);

    // Joints (corner points)
    QVariantList joints() const;
    void setJoints(const QList<Joint>& joints);
    Q_INVOKABLE void setJoints(const QVariantList& joints);
    
    // Helper method for backward compatibility
    void setJointPositions(const QList<QPointF>& positions);
    
    // Joint property access
    Q_INVOKABLE void setJointMirroring(int jointIndex, MirroringType mirroring);
    Q_INVOKABLE void setJointCornerRadius(int jointIndex, qreal radius);
    Q_INVOKABLE int getJointMirroring(int jointIndex) const;
    Q_INVOKABLE qreal getJointCornerRadius(int jointIndex) const;

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

    // Register Shape properties with PropertyRegistry
    void registerProperties() override;
    
    // Override property access for custom handling
    QVariant getProperty(const QString& name) const override;
    void setProperty(const QString& name, const QVariant& value) override;

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
    QList<Joint> m_joints;
    qreal m_edgeWidth = 2.0;
    QColor m_edgeColor = QColor(0, 0, 0, 255); // Black
    QColor m_fillColor = QColor(0, 120, 255, 255); // Blue
};

#endif // SHAPE_H