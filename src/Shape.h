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
    Q_PROPERTY(QVariantList joints READ joints WRITE setJoints NOTIFY jointsChanged)
    Q_PROPERTY(QVariantList edges READ edges WRITE setEdges NOTIFY edgesChanged)
    Q_PROPERTY(qreal edgeWidth READ edgeWidth WRITE setEdgeWidth NOTIFY edgeWidthChanged)
    Q_PROPERTY(QColor edgeColor READ edgeColor WRITE setEdgeColor NOTIFY edgeColorChanged)
    Q_PROPERTY(QColor fillColor READ fillColor WRITE setFillColor NOTIFY fillColorChanged)
    Q_PROPERTY(QString lineJoin READ lineJoin WRITE setLineJoin NOTIFY lineJoinChanged)
    Q_PROPERTY(QString lineCap READ lineCap WRITE setLineCap NOTIFY lineCapChanged)

public:
    enum ShapeType {
        Square,
        Triangle,
        Pen
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
        bool isPathStart = false; // Indicates this joint starts a new path

        // Convert to/from QVariantMap for QML
        QVariantMap toVariantMap() const {
            QVariantMap map;
            map["x"] = position.x();
            map["y"] = position.y();
            map["mirroring"] = static_cast<int>(mirroring);
            map["cornerRadius"] = cornerRadius;
            map["isPathStart"] = isPathStart;
            return map;
        }

        static Joint fromVariantMap(const QVariantMap& map) {
            Joint joint;
            joint.position = QPointF(map.value("x", 0.0).toDouble(), 
                                   map.value("y", 0.0).toDouble());
            joint.mirroring = static_cast<MirroringType>(map.value("mirroring", NoMirroring).toInt());
            joint.cornerRadius = map.value("cornerRadius", 0.0).toDouble();
            joint.isPathStart = map.value("isPathStart", false).toBool();
            return joint;
        }
    };

    struct Edge {
        int fromIndex;
        int toIndex;

        // Convert to/from QVariantMap for QML
        QVariantMap toVariantMap() const {
            QVariantMap map;
            map["from"] = fromIndex;
            map["to"] = toIndex;
            return map;
        }

        static Edge fromVariantMap(const QVariantMap& map) {
            Edge edge;
            edge.fromIndex = map.value("from", -1).toInt();
            edge.toIndex = map.value("to", -1).toInt();
            return edge;
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

    // Edges (connections between joints)
    QVariantList edges() const;
    void setEdges(const QList<Edge>& edges);
    Q_INVOKABLE void setEdges(const QVariantList& edges);
    Q_INVOKABLE void addEdge(int fromIndex, int toIndex);
    
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

    // Line style properties
    QString lineJoin() const { return m_lineJoin; }
    void setLineJoin(const QString& lineJoin);

    QString lineCap() const { return m_lineCap; }
    void setLineCap(const QString& lineCap);

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
    void edgesChanged();
    void edgeWidthChanged();
    void edgeColorChanged();
    void fillColorChanged();
    void lineJoinChanged();
    void lineCapChanged();

private:
    void updateJointsForShape();
    void updateSquareJoints();
    void updateTriangleJoints();
    void updatePenJoints();
    void updateName();

    ShapeType m_shapeType = Square;
    QList<Joint> m_joints;
    QList<Edge> m_edges;
    qreal m_edgeWidth = 2.0;
    QColor m_edgeColor = QColor(0, 0, 0, 255); // Black
    QColor m_fillColor = QColor(0, 120, 255, 255); // Blue
    QString m_lineJoin = "miter"; // miter, round, or bevel
    QString m_lineCap = "round"; // butt, round, or square
};

#endif // SHAPE_H