#pragma once
#include "CanvasElement.h"
#include <QColor>
#include <QPointF>

class Edge : public CanvasElement {
    Q_OBJECT
    Q_PROPERTY(QString sourceNodeId READ sourceNodeId WRITE setSourceNodeId NOTIFY sourceNodeIdChanged)
    Q_PROPERTY(QString targetNodeId READ targetNodeId WRITE setTargetNodeId NOTIFY targetNodeIdChanged)
    Q_PROPERTY(int sourcePortIndex READ sourcePortIndex WRITE setSourcePortIndex NOTIFY sourcePortIndexChanged)
    Q_PROPERTY(int targetPortIndex READ targetPortIndex WRITE setTargetPortIndex NOTIFY targetPortIndexChanged)
    Q_PROPERTY(QColor edgeColor READ edgeColor WRITE setEdgeColor NOTIFY edgeColorChanged)
    Q_PROPERTY(qreal edgeWidth READ edgeWidth WRITE setEdgeWidth NOTIFY edgeWidthChanged)
    Q_PROPERTY(bool isActive READ isActive WRITE setIsActive NOTIFY isActiveChanged)
    Q_PROPERTY(QPointF sourcePoint READ sourcePoint WRITE setSourcePoint NOTIFY sourcePointChanged)
    Q_PROPERTY(QPointF targetPoint READ targetPoint WRITE setTargetPoint NOTIFY targetPointChanged)
    Q_PROPERTY(QPointF controlPoint1 READ controlPoint1 NOTIFY controlPoint1Changed)
    Q_PROPERTY(QPointF controlPoint2 READ controlPoint2 NOTIFY controlPoint2Changed)
    Q_PROPERTY(QString sourceHandleType READ sourceHandleType WRITE setSourceHandleType NOTIFY sourceHandleTypeChanged)
    Q_PROPERTY(QString targetHandleType READ targetHandleType WRITE setTargetHandleType NOTIFY targetHandleTypeChanged)
    Q_PROPERTY(QString sourcePortType READ sourcePortType WRITE setSourcePortType NOTIFY sourcePortTypeChanged)
    Q_PROPERTY(QString targetPortType READ targetPortType WRITE setTargetPortType NOTIFY targetPortTypeChanged)
    
public:
    explicit Edge(const QString &id, QObject *parent = nullptr);
    
    // Property getters
    QString sourceNodeId() const { return m_sourceNodeId; }
    QString targetNodeId() const { return m_targetNodeId; }
    int sourcePortIndex() const { return m_sourcePortIndex; }
    int targetPortIndex() const { return m_targetPortIndex; }
    QColor edgeColor() const { return m_edgeColor; }
    qreal edgeWidth() const { return m_edgeWidth; }
    bool isActive() const { return m_isActive; }
    QPointF sourcePoint() const { return m_sourcePoint; }
    QPointF targetPoint() const { return m_targetPoint; }
    QPointF controlPoint1() const { return m_controlPoint1; }
    QPointF controlPoint2() const { return m_controlPoint2; }
    QString sourceHandleType() const { return m_sourceHandleType; }
    QString targetHandleType() const { return m_targetHandleType; }
    QString sourcePortType() const { return m_sourcePortType; }
    QString targetPortType() const { return m_targetPortType; }
    
    // Property setters
    void setSourceNodeId(const QString &nodeId);
    void setTargetNodeId(const QString &nodeId);
    void setSourcePortIndex(int index);
    void setTargetPortIndex(int index);
    void setEdgeColor(const QColor &color);
    void setEdgeWidth(qreal width);
    void setIsActive(bool active);
    void setSourcePoint(const QPointF &point);
    void setTargetPoint(const QPointF &point);
    void setSourceHandleType(const QString &type);
    void setTargetHandleType(const QString &type);
    void setSourcePortType(const QString &type);
    void setTargetPortType(const QString &type);
    
    // Edge state
    bool isConnected() const;
    bool isPartiallyConnected() const;
    
    // Update edge geometry based on connected nodes
    void updateGeometry();
    
    // Calculate bezier control points based on handle types
    void updateControlPoints();
    
    // Override hit testing for bezier curve
    bool containsPoint(const QPointF &point) const override;
    
signals:
    void sourceNodeIdChanged();
    void targetNodeIdChanged();
    void sourcePortIndexChanged();
    void targetPortIndexChanged();
    void edgeColorChanged();
    void edgeWidthChanged();
    void isActiveChanged();
    void sourcePointChanged();
    void targetPointChanged();
    void controlPoint1Changed();
    void controlPoint2Changed();
    void sourceHandleTypeChanged();
    void targetHandleTypeChanged();
    void sourcePortTypeChanged();
    void targetPortTypeChanged();
    
private:
    QString m_sourceNodeId;
    QString m_targetNodeId;
    int m_sourcePortIndex;
    int m_targetPortIndex;
    QColor m_edgeColor;
    qreal m_edgeWidth;
    bool m_isActive;
    QPointF m_sourcePoint;
    QPointF m_targetPoint;
    QPointF m_controlPoint1;
    QPointF m_controlPoint2;
    QString m_sourceHandleType; // "left" or "right"
    QString m_targetHandleType; // "left" or "right"
    QString m_sourcePortType; // "Flow" or "Variable"
    QString m_targetPortType; // "Flow" or "Variable"
};