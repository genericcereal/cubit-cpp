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
    
    // Check outputs array contains the node value
    QJsonArray outputs = event["outputs"].toArray();
    QVERIFY(outputs.size() > 0);
    
    bool foundValue = false;
    for (const QJsonValue& val : outputs) {
        QJsonObject output = val.toObject();
        if (output["type"].toString() == "literal" && 
            output["value"].toString() == "Hello from test!") {
            foundValue = true;
            break;
        }
    }
    QVERIFY2(foundValue, "Node value not found in outputs");
    
    // Check invoke array references the output
    QJsonArray invokes = event["invoke"].toArray();
    QCOMPARE(invokes.size(), 1);
    
    QJsonObject invoke = invokes[0].toObject();
    QCOMPARE(invoke["id"].toString(), consoleNode->getId());
    
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
    QJsonArray outputs = event["outputs"].toArray();
    bool foundDataValue = false;
    for (const QJsonValue& val : outputs) {
        QJsonObject output = val.toObject();
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
    
    // Check both consoles are in invoke array
    QJsonArray invokes = event["invoke"].toArray();
    QCOMPARE(invokes.size(), 2);
    
    // Check both values are in outputs
    QJsonArray outputs = event["outputs"].toArray();
    bool foundFirst = false, foundSecond = false;
    for (const QJsonValue& val : outputs) {
        QJsonObject output = val.toObject();
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