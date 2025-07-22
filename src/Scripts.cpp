#include "Scripts.h"
#include "Node.h"
#include "Edge.h"
#include "UniqueIdGenerator.h"
#include "ScriptCompiler.h"
#include "ConsoleMessageRepository.h"
#include <algorithm>

Scripts::Scripts(QObject *parent, bool isComponentInstance)
    : QObject(parent)
{
    // Initialize with appropriate nodes based on type
    if (isComponentInstance) {
        loadComponentInstanceNodes();
    } else {
        loadInitialNodes();
    }
}

Scripts::~Scripts() {
    // Destructor defined here so Node and Edge are complete types
}

// Node management
void Scripts::addNode(Node* node) {
    if (!node) return;
    
    // Check if node already exists
    auto it = std::find_if(m_nodes.begin(), m_nodes.end(),
        [node](const std::unique_ptr<Node>& n) { return n.get() == node; });
    
    if (it == m_nodes.end()) {
        m_nodes.push_back(std::unique_ptr<Node>(node));
        
        // Reset compiled state when graph changes
        if (m_isCompiled) {
            setIsCompiled(false);
        }
        
        emit nodeAdded(node);
        emit nodesChanged();
    }
}

void Scripts::removeNode(Node* node) {
    if (!node) return;
    
    auto it = std::find_if(m_nodes.begin(), m_nodes.end(),
        [node](const std::unique_ptr<Node>& n) { return n.get() == node; });
    
    if (it != m_nodes.end()) {
        // Remove any edges connected to this node
        QString nodeId = node->getId();
        auto connectedEdges = getEdgesForNode(nodeId);
        for (Edge* edge : connectedEdges) {
            removeEdge(edge);
        }
        
        m_nodes.erase(it);
        
        // Reset compiled state when graph changes
        if (m_isCompiled) {
            setIsCompiled(false);
        }
        
        emit nodeRemoved(node);
        emit nodesChanged();
    }
}

void Scripts::clearNodes() {
    if (!m_nodes.empty()) {
        m_nodes.clear();
        emit nodesChanged();
    }
}

Node* Scripts::getNode(const QString& nodeId) const {
    auto it = std::find_if(m_nodes.begin(), m_nodes.end(),
        [&nodeId](const std::unique_ptr<Node>& n) { return n->getId() == nodeId; });
    
    return (it != m_nodes.end()) ? it->get() : nullptr;
}

QList<Node*> Scripts::getAllNodes() const {
    QList<Node*> result;
    for (const auto& node : m_nodes) {
        result.append(node.get());
    }
    return result;
}

// Edge management
void Scripts::addEdge(Edge* edge) {
    if (!edge) return;
    
    // Check if edge already exists
    auto it = std::find_if(m_edges.begin(), m_edges.end(),
        [edge](const std::unique_ptr<Edge>& e) { return e.get() == edge; });
    
    if (it == m_edges.end()) {
        m_edges.push_back(std::unique_ptr<Edge>(edge));
        
        // Reset compiled state when graph changes
        if (m_isCompiled) {
            setIsCompiled(false);
        }
        
        emit edgeAdded(edge);
        emit edgesChanged();
    }
}

void Scripts::removeEdge(Edge* edge) {
    if (!edge) return;
    
    auto it = std::find_if(m_edges.begin(), m_edges.end(),
        [edge](const std::unique_ptr<Edge>& e) { return e.get() == edge; });
    
    if (it != m_edges.end()) {
        m_edges.erase(it);
        
        // Reset compiled state when graph changes
        if (m_isCompiled) {
            setIsCompiled(false);
        }
        
        emit edgeRemoved(edge);
        emit edgesChanged();
    }
}

void Scripts::clearEdges() {
    if (!m_edges.empty()) {
        m_edges.clear();
        emit edgesChanged();
    }
}

Edge* Scripts::getEdge(const QString& edgeId) const {
    auto it = std::find_if(m_edges.begin(), m_edges.end(),
        [&edgeId](const std::unique_ptr<Edge>& e) { return e->getId() == edgeId; });
    
    return (it != m_edges.end()) ? it->get() : nullptr;
}

QList<Edge*> Scripts::getAllEdges() const {
    QList<Edge*> result;
    for (const auto& edge : m_edges) {
        result.append(edge.get());
    }
    return result;
}

// Find edges connected to a node
QList<Edge*> Scripts::getEdgesForNode(const QString& nodeId) const {
    QList<Edge*> result;
    for (const auto& edge : m_edges) {
        if (edge->sourceNodeId() == nodeId || edge->targetNodeId() == nodeId) {
            result.append(edge.get());
        }
    }
    return result;
}

QList<Edge*> Scripts::getIncomingEdges(const QString& nodeId) const {
    QList<Edge*> result;
    for (const auto& edge : m_edges) {
        if (edge->targetNodeId() == nodeId) {
            result.append(edge.get());
        }
    }
    return result;
}

QList<Edge*> Scripts::getOutgoingEdges(const QString& nodeId) const {
    QList<Edge*> result;
    for (const auto& edge : m_edges) {
        if (edge->sourceNodeId() == nodeId) {
            result.append(edge.get());
        }
    }
    return result;
}

// Clear all scripts
void Scripts::clear() {
    clearEdges();
    clearNodes();
}

// Compile the script graph to JSON
QString Scripts::compile(ElementModel* elementModel) {
    ScriptCompiler compiler;
    QString result = compiler.compile(this, elementModel);
    
    if (result.isEmpty()) {
        QString error = compiler.getLastError();
        ConsoleMessageRepository::instance()->addError("Compilation failed: " + error);
        m_compiledScript.clear();
        setIsCompiled(false);
        return QString();
    }
    
    // Store the compiled script
    m_compiledScript = result;
    setIsCompiled(true);
    emit compiledScriptChanged();
    
    // Console output removed - scripts compiled successfully
    
    return result;
}

// Property getters
QQmlListProperty<Node> Scripts::nodes() {
    return QQmlListProperty<Node>(this, nullptr,
        &Scripts::appendNode,
        &Scripts::nodeCount,
        &Scripts::nodeAt,
        &Scripts::clearNodes);
}

QQmlListProperty<Edge> Scripts::edges() {
    return QQmlListProperty<Edge>(this, nullptr,
        &Scripts::appendEdge,
        &Scripts::edgeCount,
        &Scripts::edgeAt,
        &Scripts::clearEdges);
}

int Scripts::nodeCount() const {
    return static_cast<int>(m_nodes.size());
}

int Scripts::edgeCount() const {
    return static_cast<int>(m_edges.size());
}

bool Scripts::isCompiled() const {
    return m_isCompiled;
}

QString Scripts::compiledScript() const {
    return m_compiledScript;
}

// Property setters
void Scripts::setIsCompiled(bool compiled) {
    if (m_isCompiled != compiled) {
        m_isCompiled = compiled;
        
        // Clear compiled script when marking as not compiled
        if (!compiled) {
            m_compiledScript.clear();
            emit compiledScriptChanged();
        }
        
        emit isCompiledChanged();
    }
}

void Scripts::setCompiledScript(const QString& script) {
    if (m_compiledScript != script) {
        m_compiledScript = script;
        emit compiledScriptChanged();
    }
}

// QML list property helpers
void Scripts::appendNode(QQmlListProperty<Node>* list, Node* node) {
    Scripts* scripts = qobject_cast<Scripts*>(list->object);
    if (scripts) {
        scripts->addNode(node);
    }
}

qsizetype Scripts::nodeCount(QQmlListProperty<Node>* list) {
    Scripts* scripts = qobject_cast<Scripts*>(list->object);
    return scripts ? scripts->nodeCount() : 0;
}

Node* Scripts::nodeAt(QQmlListProperty<Node>* list, qsizetype index) {
    Scripts* scripts = qobject_cast<Scripts*>(list->object);
    if (scripts && index >= 0 && index < static_cast<qsizetype>(scripts->m_nodes.size())) {
        return scripts->m_nodes[index].get();
    }
    return nullptr;
}

void Scripts::clearNodes(QQmlListProperty<Node>* list) {
    Scripts* scripts = qobject_cast<Scripts*>(list->object);
    if (scripts) {
        scripts->clearNodes();
    }
}

void Scripts::appendEdge(QQmlListProperty<Edge>* list, Edge* edge) {
    Scripts* scripts = qobject_cast<Scripts*>(list->object);
    if (scripts) {
        scripts->addEdge(edge);
    }
}

qsizetype Scripts::edgeCount(QQmlListProperty<Edge>* list) {
    Scripts* scripts = qobject_cast<Scripts*>(list->object);
    return scripts ? scripts->edgeCount() : 0;
}

Edge* Scripts::edgeAt(QQmlListProperty<Edge>* list, qsizetype index) {
    Scripts* scripts = qobject_cast<Scripts*>(list->object);
    if (scripts && index >= 0 && index < static_cast<qsizetype>(scripts->m_edges.size())) {
        return scripts->m_edges[index].get();
    }
    return nullptr;
}

void Scripts::clearEdges(QQmlListProperty<Edge>* list) {
    Scripts* scripts = qobject_cast<Scripts*>(list->object);
    if (scripts) {
        scripts->clearEdges();
    }
}

void Scripts::loadInitialNodes() {
    // Create the onEditorLoad node
    QString nodeId = UniqueIdGenerator::generate16DigitId();
    Node* onEditorLoadNode = new Node(nodeId, this);
    
    // Configure the onEditorLoad node based on NodeCatalog.qml
    onEditorLoadNode->setNodeTitle("On Editor Load");
    onEditorLoadNode->setNodeType("Event");
    
    // Set position (centered in a reasonable default location)
    onEditorLoadNode->setX(-100);
    onEditorLoadNode->setY(0);
    onEditorLoadNode->setWidth(150);
    onEditorLoadNode->setHeight(80);
    
    // Configure the node's ports based on the catalog
    // onEditorLoad has one source port: "done" (Flow type)
    Node::RowConfig rowConfig;
    rowConfig.hasSource = true;
    rowConfig.sourceLabel = "Done";
    rowConfig.sourceType = "Flow";
    rowConfig.sourcePortIndex = 0;
    onEditorLoadNode->addRow(rowConfig);
    
    // Add output port
    onEditorLoadNode->addOutputPort("done");
    onEditorLoadNode->setOutputPortType(0, "Flow");
    
    // Add the node to the scripts
    addNode(onEditorLoadNode);
}

void Scripts::loadComponentInstanceNodes() {
    // Create the onEditorLoad node
    QString onEditorLoadId = UniqueIdGenerator::generate16DigitId();
    Node* onEditorLoadNode = new Node(onEditorLoadId, this);
    
    // Configure the onEditorLoad node
    onEditorLoadNode->setNodeTitle("On Editor Load");
    onEditorLoadNode->setNodeType("Event");
    
    // Set position 
    onEditorLoadNode->setX(-200);
    onEditorLoadNode->setY(0);
    onEditorLoadNode->setWidth(150);
    onEditorLoadNode->setHeight(80);
    
    // Configure the node's ports
    Node::RowConfig rowConfig;
    rowConfig.hasSource = true;
    rowConfig.sourceLabel = "Done";
    rowConfig.sourceType = "Flow";
    rowConfig.sourcePortIndex = 0;
    onEditorLoadNode->addRow(rowConfig);
    
    // Add output port
    onEditorLoadNode->addOutputPort("done");
    onEditorLoadNode->setOutputPortType(0, "Flow");
    
    // Add the onEditorLoad node
    addNode(onEditorLoadNode);
    
    // Create the ComponentOnEditorLoadEvents node
    QString componentEventsId = UniqueIdGenerator::generate16DigitId();
    Node* componentEventsNode = new Node(componentEventsId, this);
    
    // Configure the ComponentOnEditorLoadEvents node
    componentEventsNode->setNodeTitle("Component On Editor Load Events");
    componentEventsNode->setNodeType("Operation");
    
    // Set position (to the right of onEditorLoad)
    componentEventsNode->setX(50);
    componentEventsNode->setY(0);
    componentEventsNode->setWidth(250);
    componentEventsNode->setHeight(80);
    
    // Configure the node's ports
    Node::RowConfig componentRowConfig;
    componentRowConfig.hasTarget = true;
    componentRowConfig.targetLabel = "Exec";
    componentRowConfig.targetType = "Flow";
    componentRowConfig.targetPortIndex = 0;
    componentRowConfig.hasSource = true;
    componentRowConfig.sourceLabel = "Done";
    componentRowConfig.sourceType = "Flow";
    componentRowConfig.sourcePortIndex = 0;
    componentEventsNode->addRow(componentRowConfig);
    
    // Add ports
    componentEventsNode->addInputPort("exec");
    componentEventsNode->setInputPortType(0, "Flow");
    componentEventsNode->addOutputPort("done");
    componentEventsNode->setOutputPortType(0, "Flow");
    
    // Add the ComponentOnEditorLoadEvents node
    addNode(componentEventsNode);
    
    // Create an edge connecting onEditorLoad's "done" to ComponentOnEditorLoadEvents's "exec"
    QString edgeId = UniqueIdGenerator::generate16DigitId();
    Edge* edge = new Edge(edgeId, this);
    
    // Set source (onEditorLoad "done" port - output port index 0)
    edge->setSourceNodeId(onEditorLoadId);
    edge->setSourcePortIndex(0);  // "done" is at index 0
    edge->setSourcePortType("Flow");
    
    // Set target (ComponentOnEditorLoadEvents "exec" port - input port index 0)
    edge->setTargetNodeId(componentEventsId);
    edge->setTargetPortIndex(0);  // "exec" is at index 0
    edge->setTargetPortType("Flow");
    
    // Add the edge
    addEdge(edge);
}