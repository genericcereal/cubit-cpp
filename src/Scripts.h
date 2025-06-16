#ifndef SCRIPTS_H
#define SCRIPTS_H

#include <QObject>
#include <QQmlEngine>
#include <QQmlListProperty>
#include <vector>
#include <memory>

class Node;
class Edge;

class Scripts : public QObject {
    Q_OBJECT
    Q_PROPERTY(QQmlListProperty<Node> nodes READ nodes NOTIFY nodesChanged)
    Q_PROPERTY(QQmlListProperty<Edge> edges READ edges NOTIFY edgesChanged)
    Q_PROPERTY(int nodeCount READ nodeCount NOTIFY nodesChanged)
    Q_PROPERTY(int edgeCount READ edgeCount NOTIFY edgesChanged)

public:
    explicit Scripts(QObject *parent = nullptr);
    ~Scripts();

    // Node management
    Q_INVOKABLE void addNode(Node* node);
    Q_INVOKABLE void removeNode(Node* node);
    Q_INVOKABLE void clearNodes();
    Q_INVOKABLE Node* getNode(const QString& nodeId) const;
    Q_INVOKABLE QList<Node*> getAllNodes() const;
    
    // Edge management
    Q_INVOKABLE void addEdge(Edge* edge);
    Q_INVOKABLE void removeEdge(Edge* edge);
    Q_INVOKABLE void clearEdges();
    Q_INVOKABLE Edge* getEdge(const QString& edgeId) const;
    Q_INVOKABLE QList<Edge*> getAllEdges() const;
    
    // Find edges connected to a node
    Q_INVOKABLE QList<Edge*> getEdgesForNode(const QString& nodeId) const;
    Q_INVOKABLE QList<Edge*> getIncomingEdges(const QString& nodeId) const;
    Q_INVOKABLE QList<Edge*> getOutgoingEdges(const QString& nodeId) const;
    
    // Clear all scripts
    Q_INVOKABLE void clear();

    // Property getters
    QQmlListProperty<Node> nodes();
    QQmlListProperty<Edge> edges();
    int nodeCount() const;
    int edgeCount() const;

signals:
    void nodesChanged();
    void edgesChanged();
    void nodeAdded(Node* node);
    void nodeRemoved(Node* node);
    void edgeAdded(Edge* edge);
    void edgeRemoved(Edge* edge);

private:
    std::vector<std::unique_ptr<Node>> m_nodes;
    std::vector<std::unique_ptr<Edge>> m_edges;
    
    // Initialize default nodes
    void loadInitialNodes();
    
    // QML list property helpers
    static void appendNode(QQmlListProperty<Node>* list, Node* node);
    static qsizetype nodeCount(QQmlListProperty<Node>* list);
    static Node* nodeAt(QQmlListProperty<Node>* list, qsizetype index);
    static void clearNodes(QQmlListProperty<Node>* list);
    
    static void appendEdge(QQmlListProperty<Edge>* list, Edge* edge);
    static qsizetype edgeCount(QQmlListProperty<Edge>* list);
    static Edge* edgeAt(QQmlListProperty<Edge>* list, qsizetype index);
    static void clearEdges(QQmlListProperty<Edge>* list);
};

#endif // SCRIPTS_H