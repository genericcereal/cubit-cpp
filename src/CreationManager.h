#pragma once
#include <QObject>
#include <QRectF>

class Element;
class ElementModel;
class SelectionManager;
class Node;
class Edge;

class CreationManager : public QObject {
    Q_OBJECT
    
public:
    enum class CanvasType {
        Design,
        Script
    };
    Q_ENUM(CanvasType)
    
    explicit CreationManager(QObject *parent = nullptr);
    ~CreationManager() = default;
    
    void setElementModel(ElementModel* elementModel) { m_elementModel = elementModel; }
    void setSelectionManager(SelectionManager* selectionManager) { m_selectionManager = selectionManager; }
    void setCanvasType(CanvasType type) { m_canvasType = type; }
    
    // Design element creation
    Element* createElement(const QString& type, qreal x, qreal y, qreal width, qreal height);
    
    // Drag-to-create operations
    Element* startDragCreation(const QString& type, const QPointF& startPos);
    void updateDragCreation(const QPointF& currentPos);
    Element* finishDragCreation();
    void cancelDragCreation();
    
    // Script element creation
    Node* createNode(qreal x, qreal y, const QString& title = "Node", const QString& color = QString());
    Edge* createEdge(const QString& sourceNodeId, const QString& targetNodeId, 
                     const QString& sourceHandleType, const QString& targetHandleType,
                     int sourcePortIndex, int targetPortIndex);
    Edge* createEdgeByPortId(const QString& sourceNodeId, const QString& targetNodeId,
                            const QString& sourcePortId, const QString& targetPortId);
    
signals:
    void elementCreated(Element* element);
    void nodeCreated(Node* node);
    void edgeCreated(Edge* edge);
    
private:
    ElementModel* m_elementModel = nullptr;
    SelectionManager* m_selectionManager = nullptr;
    CanvasType m_canvasType = CanvasType::Design;
    
    // Drag creation state
    Element* m_dragCreationElement = nullptr;
    QPointF m_dragCreationStartPos;
    QString m_dragCreationType;
    
    // Helper to set up default node ports
    void setupDefaultNodePorts(Node* node);
    
    // Helper to calculate edge connection points
    void calculateEdgePoints(Edge* edge, Element* sourceNode, Element* targetNode,
                           const QString& sourceHandleType, const QString& targetHandleType,
                           int sourcePortIndex, int targetPortIndex) const;
};