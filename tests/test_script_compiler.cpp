#include <QtTest>
#include <QCoreApplication>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include "../src/ScriptCompiler.h"
#include "../src/Scripts.h"
#include "../src/Node.h"
#include "../src/Edge.h"

class tst_ScriptCompiler : public QObject
{
    Q_OBJECT

private slots:
    void initTestCase();
    void cleanupTestCase();
    void init();
    void cleanup();

    // Test cases
    void testEmptyScripts();
    void testSingleEventNode();
    void testConsoleLogWithValue();
    void testConsoleLogWithEdgeConnection();
    void testMultipleNodes();
    void testNodeValueProperty();

private:
    Scripts* m_scripts;
    ScriptCompiler* m_compiler;
};

void tst_ScriptCompiler::initTestCase()
{
    // Global test initialization
}

void tst_ScriptCompiler::cleanupTestCase()
{
    // Global test cleanup
}

void tst_ScriptCompiler::init()
{
    // Per-test initialization
    m_scripts = new Scripts(this);
    m_compiler = new ScriptCompiler(this);
    
    // Clear default nodes that Scripts creates
    m_scripts->clearNodes();
    m_scripts->clearEdges();
}

void tst_ScriptCompiler::cleanup()
{
    // Per-test cleanup
    delete m_compiler;
    delete m_scripts;
}

void tst_ScriptCompiler::testEmptyScripts()
{
    // Test compiling with no nodes
    QString result = m_compiler->compile(m_scripts);
    
    QVERIFY(result.isEmpty());
    QCOMPARE(m_compiler->getLastError(), "No nodes in the graph");
}

void tst_ScriptCompiler::testSingleEventNode()
{
    // Create a single event node
    Node* eventNode = new Node("node1", m_scripts);
    eventNode->setNodeTitle("On Editor Load");
    eventNode->setNodeType("Event");
    m_scripts->addNode(eventNode);
    
    QString result = m_compiler->compile(m_scripts);
    QVERIFY(!result.isEmpty());
    
    QJsonDocument doc = QJsonDocument::fromJson(result.toUtf8());
    QVERIFY(!doc.isNull());
    
    QJsonObject root = doc.object();
    QVERIFY(root.contains("oneditorload"));
    
    QJsonObject event = root["oneditorload"].toObject();
    QVERIFY(event.contains("functions"));
    QVERIFY(event.contains("outputs"));
    QVERIFY(event.contains("invoke"));
    QVERIFY(event.contains("next"));
    
    // Since this is a single event node with no connections, next should be empty
    QJsonArray next = event["next"].toArray();
    QCOMPARE(next.size(), 0);
}

void tst_ScriptCompiler::testConsoleLogWithValue()
{
    // Create event node
    Node* eventNode = new Node("event1", m_scripts);
    eventNode->setNodeTitle("On Editor Load");
    eventNode->setNodeType("Event");
    eventNode->setX(100);
    eventNode->setY(100);
    m_scripts->addNode(eventNode);
    
    // Create console log node with a value
    Node* consoleNode = new Node("console1", m_scripts);
    consoleNode->setNodeTitle("Console Log");
    consoleNode->setNodeType("Operation");
    consoleNode->setX(300);
    consoleNode->setY(100);
    consoleNode->setValue("Hello from test!");  // Set the value
    
    // Add row configuration for console log
    Node::RowConfig targetRow;
    targetRow.hasTarget = true;
    targetRow.targetLabel = "Message";
    targetRow.targetType = "Data";
    targetRow.targetPortIndex = 0;
    consoleNode->addRow(targetRow);
    
    m_scripts->addNode(consoleNode);
    
    // Connect event to console with flow edge
    Edge* flowEdge = new Edge("edge1", m_scripts);
    flowEdge->setSourceNodeId(eventNode->getId());
    flowEdge->setTargetNodeId(consoleNode->getId());
    flowEdge->setSourcePortIndex(0);
    flowEdge->setTargetPortIndex(0);
    flowEdge->setSourcePortType("Flow");
    flowEdge->setTargetPortType("Flow");
    m_scripts->addEdge(flowEdge);
    
    // Compile
    QString result = m_compiler->compile(m_scripts);
    QVERIFY(!result.isEmpty());
    
    QJsonDocument doc = QJsonDocument::fromJson(result.toUtf8());
    QVERIFY(!doc.isNull());
    
    QJsonObject root = doc.object();
    QJsonObject event = root["oneditorload"].toObject();
    
    // Check that next array contains the invoke ID for console node
    QVERIFY(event.contains("next"));
    QJsonArray next = event["next"].toArray();
    QCOMPARE(next.size(), 1);
    QCOMPARE(next[0].toString(), "invoke_0");  // First invoke should be invoke_0
    
    // Check outputs object contains the node value
    QJsonObject outputs = event["outputs"].toObject();
    QVERIFY(outputs.size() > 0);
    
    bool foundValue = false;
    for (auto it = outputs.begin(); it != outputs.end(); ++it) {
        QJsonObject output = it.value().toObject();
        if (output["type"].toString() == "literal" && 
            output["value"].toString() == "Hello from test!") {
            foundValue = true;
            break;
        }
    }
    QVERIFY2(foundValue, "Node value not found in outputs");
    
    // Check invoke object references the output
    QJsonObject invokes = event["invoke"].toObject();
    QCOMPARE(invokes.size(), 1);
    
    QString invokeKey = invokes.keys().first();
    QJsonObject invoke = invokes[invokeKey].toObject();
    QCOMPARE(invoke["nodeId"].toString(), consoleNode->getId());
    
    // Check params array has the output reference
    QJsonArray params = invoke["params"].toArray();
    if (params.isEmpty()) {
        qDebug() << "Params array is empty!";
        qDebug() << "Full compiled JSON:";
        qDebug() << result;
    }
    QVERIFY(params.size() > 0);
}

void tst_ScriptCompiler::testConsoleLogWithEdgeConnection()
{
    // Create event node
    Node* eventNode = new Node("event1", m_scripts);
    eventNode->setNodeTitle("On Editor Load");
    eventNode->setNodeType("Event");
    m_scripts->addNode(eventNode);
    
    // Create a data source node (simulating another node providing data)
    Node* dataNode = new Node("data1", m_scripts);
    dataNode->setNodeTitle("Variable");
    dataNode->setNodeType("Param");
    dataNode->setValue("Data from variable");
    m_scripts->addNode(dataNode);
    
    // Create console log node
    Node* consoleNode = new Node("console1", m_scripts);
    consoleNode->setNodeTitle("Console Log");
    consoleNode->setNodeType("Operation");
    m_scripts->addNode(consoleNode);
    
    // Connect with flow edge
    Edge* flowEdge = new Edge("edge1", m_scripts);
    flowEdge->setSourceNodeId(eventNode->getId());
    flowEdge->setTargetNodeId(consoleNode->getId());
    flowEdge->setSourcePortType("Flow");
    flowEdge->setTargetPortType("Flow");
    m_scripts->addEdge(flowEdge);
    
    // Connect data edge
    Edge* dataEdge = new Edge("edge2", m_scripts);
    dataEdge->setSourceNodeId(dataNode->getId());
    dataEdge->setTargetNodeId(consoleNode->getId());
    dataEdge->setSourcePortType("Data");
    dataEdge->setTargetPortType("Data");
    m_scripts->addEdge(dataEdge);
    
    // Compile
    QString result = m_compiler->compile(m_scripts);
    QVERIFY(!result.isEmpty());
    
    QJsonDocument doc = QJsonDocument::fromJson(result.toUtf8());
    QJsonObject root = doc.object();
    QJsonObject event = root["oneditorload"].toObject();
    
    // Verify the data node's value appears in outputs
    QJsonObject outputs = event["outputs"].toObject();
    bool foundDataValue = false;
    for (auto it = outputs.begin(); it != outputs.end(); ++it) {
        QJsonObject output = it.value().toObject();
        if (output["value"].toString() == "Data from variable") {
            foundDataValue = true;
            break;
        }
    }
    QVERIFY2(foundDataValue, "Data node value not found in outputs");
}

void tst_ScriptCompiler::testMultipleNodes()
{
    // Create a more complex graph
    Node* eventNode = new Node("event1", m_scripts);
    eventNode->setNodeTitle("On Editor Load");
    eventNode->setNodeType("Event");
    m_scripts->addNode(eventNode);
    
    Node* console1 = new Node("console1", m_scripts);
    console1->setNodeTitle("Console Log");
    console1->setNodeType("Operation");
    console1->setValue("First message");
    m_scripts->addNode(console1);
    
    Node* console2 = new Node("console2", m_scripts);
    console2->setNodeTitle("Console Log");
    console2->setNodeType("Operation");
    console2->setValue("Second message");
    m_scripts->addNode(console2);
    
    // Connect in sequence
    Edge* edge1 = new Edge("edge1", m_scripts);
    edge1->setSourceNodeId(eventNode->getId());
    edge1->setTargetNodeId(console1->getId());
    edge1->setSourcePortType("Flow");
    edge1->setTargetPortType("Flow");
    m_scripts->addEdge(edge1);
    
    Edge* edge2 = new Edge("edge2", m_scripts);
    edge2->setSourceNodeId(console1->getId());
    edge2->setTargetNodeId(console2->getId());
    edge2->setSourcePortType("Flow");
    edge2->setTargetPortType("Flow");
    m_scripts->addEdge(edge2);
    
    QString result = m_compiler->compile(m_scripts);
    QVERIFY(!result.isEmpty());
    
    QJsonDocument doc = QJsonDocument::fromJson(result.toUtf8());
    QJsonObject root = doc.object();
    QJsonObject event = root["oneditorload"].toObject();
    
    // Check both consoles are in invoke object
    QJsonObject invokes = event["invoke"].toObject();
    QCOMPARE(invokes.size(), 2);
    
    // Find the invokes for each console
    QJsonObject firstInvoke, secondInvoke;
    for (auto it = invokes.begin(); it != invokes.end(); ++it) {
        QJsonObject invoke = it.value().toObject();
        if (invoke["nodeId"].toString() == console1->getId()) {
            firstInvoke = invoke;
        } else if (invoke["nodeId"].toString() == console2->getId()) {
            secondInvoke = invoke;
        }
    }
    
    // Check first invoke has next pointing to second console's invoke ID
    QVERIFY(!firstInvoke.isEmpty());
    QVERIFY(firstInvoke.contains("next"));
    QJsonArray firstNext = firstInvoke["next"].toArray();
    QCOMPARE(firstNext.size(), 1);
    QCOMPARE(firstNext[0].toString(), "invoke_1");  // Second invoke should be invoke_1
    
    // Check second invoke has no next (end of chain)
    QVERIFY(!secondInvoke.isEmpty());
    if (secondInvoke.contains("next")) {
        QJsonArray secondNext = secondInvoke["next"].toArray();
        QCOMPARE(secondNext.size(), 0);
    }
    
    // Check both values are in outputs
    QJsonObject outputs = event["outputs"].toObject();
    bool foundFirst = false, foundSecond = false;
    for (auto it = outputs.begin(); it != outputs.end(); ++it) {
        QJsonObject output = it.value().toObject();
        if (output["value"].toString() == "First message") foundFirst = true;
        if (output["value"].toString() == "Second message") foundSecond = true;
    }
    QVERIFY(foundFirst && foundSecond);
}

void tst_ScriptCompiler::testNodeValueProperty()
{
    // Test that the value property is properly stored and retrieved
    Node* node = new Node("test1", m_scripts);
    
    // Test empty value
    QVERIFY(node->value().isEmpty());
    
    // Test setting value
    node->setValue("Test Value");
    QCOMPARE(node->value(), "Test Value");
    
    // Test value change signal
    QSignalSpy spy(node, &Node::valueChanged);
    node->setValue("New Value");
    QCOMPARE(spy.count(), 1);
    QCOMPARE(node->value(), "New Value");
    
    // Test no signal on same value
    spy.clear();
    node->setValue("New Value");
    QCOMPARE(spy.count(), 0);
}

QTEST_MAIN(tst_ScriptCompiler)

#include "test_script_compiler.moc"