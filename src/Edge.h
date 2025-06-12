#pragma once
#include "Element.h"
#include <QColor>
#include <QPointF>

class Edge : public Element {
    Q_OBJECT
    Q_PROPERTY(QString sourceNodeId READ sourceNodeId WRITE setSourceNodeId NOTIFY sourceNodeIdChanged)
    Q_PROPERTY(QString targetNodeId READ targetNodeId WRITE setTargetNodeId NOTIFY targetNodeIdChanged)
    Q_PROPERTY(int sourcePortIndex READ sourcePortIndex WRITE setSourcePortIndex NOTIFY sourcePortIndexChanged)
    Q_PROPERTY(int targetPortIndex READ targetPortIndex WRITE setTargetPortIndex NOTIFY targetPortIndexChanged)
    Q_PROPERTY(QColor edgeColor READ edgeColor WRITE setEdgeColor NOTIFY edgeColorChanged)
    Q_PROPERTY(qreal edgeWidth READ edgeWidth WRITE setEdgeWidth NOTIFY edgeWidthChanged)
    Q_PROPERTY(bool isActive READ isActive WRITE setIsActive NOTIFY isActiveChanged)
    Q_PROPERTY(QPointF sourcePoint READ sourcePoint NOTIFY sourcePointChanged)
    Q_PROPERTY(QPointF targetPoint READ targetPoint NOTIFY targetPointChanged)
    
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
    
    // Edge state
    bool isConnected() const;
    bool isPartiallyConnected() const;
    
    // Update edge geometry based on connected nodes
    void updateGeometry();
    
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
};