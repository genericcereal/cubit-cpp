#include "JsonImporter.h"
#include "Node.h"
#include "CreationManager.h"
#include "ElementModel.h"
#include "HandleType.h"
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QJsonValue>
#include <QDebug>

JsonImporter::JsonImporter(QObject *parent)
    : QObject(parent)
{
}

QString JsonImporter::createNodeFromJson(const QString& jsonData)
{
    if (!m_creationManager || !m_elementModel) {
        emit importError("JsonImporter not properly initialized");
        return QString();
    }
    
    QJsonDocument doc = QJsonDocument::fromJson(jsonData.toUtf8());
    if (!doc.isObject()) {
        emit importError("Invalid JSON format - expected object");
        return QString();
    }
    
    QJsonObject nodeObj = doc.object();
    
    // Extract required fields
    QString nodeId = nodeObj.value("id").toString();
    if (nodeId.isEmpty()) {
        nodeId = m_elementModel->generateId();
    }
    
    QString name = nodeObj.value("name").toString("Node");
    QString nodeType = nodeObj.value("type").toString("Operation");
    qreal x = nodeObj.value("x").toDouble(0);
    qreal y = nodeObj.value("y").toDouble(0);
    
    // Create the node
    Node *node = new Node(nodeId);
    node->setNodeType(nodeType);
    node->setX(x);
    node->setY(y);
    node->setNodeTitle(name);
    
    // Parse targets (input ports)
    QJsonArray targets = nodeObj.value("targets").toArray();
    QStringList inputPorts;
    QList<Node::RowConfig> rowConfigs;
    
    for (int i = 0; i < targets.size(); ++i) {
        QJsonObject target = targets[i].toObject();
        QString targetId = target.value("id").toString();
        QString rawType = target.value("type").toString("Variable");
        QString targetType = PortType::migrateVariableType(rawType);
        QString targetLabel = target.value("label").toString(targetId);
        
        if (!targetId.isEmpty()) {
            inputPorts.append(targetId);
            node->setInputPortType(i, targetType);
            
            // Add to row configuration
            Node::RowConfig config;
            config.hasTarget = true;
            config.targetLabel = targetLabel;
            config.targetType = targetType;
            config.targetPortIndex = i;
            config.hasSource = false;
            rowConfigs.append(config);
        }
    }
    
    // Parse sources (output ports)
    QJsonArray sources = nodeObj.value("sources").toArray();
    QStringList outputPorts;
    
    for (int i = 0; i < sources.size(); ++i) {
        QJsonObject source = sources[i].toObject();
        QString sourceId = source.value("id").toString();
        QString rawType = source.value("type").toString("Variable");
        QString sourceType = PortType::migrateVariableType(rawType);
        QString sourceLabel = source.value("label").toString(sourceId);
        
        if (!sourceId.isEmpty()) {
            outputPorts.append(sourceId);
            node->setOutputPortType(i, sourceType);
            
            // Check if we can add to existing row or need new row
            if (i < rowConfigs.size()) {
                rowConfigs[i].hasSource = true;
                rowConfigs[i].sourceLabel = sourceLabel;
                rowConfigs[i].sourceType = sourceType;
                rowConfigs[i].sourcePortIndex = i;
            } else {
                Node::RowConfig config;
                config.hasTarget = false;
                config.hasSource = true;
                config.sourceLabel = sourceLabel;
                config.sourceType = sourceType;
                config.sourcePortIndex = i;
                rowConfigs.append(config);
            }
        }
    }
    
    // Set the ports
    node->setInputPorts(inputPorts);
    node->setOutputPorts(outputPorts);
    
    // Add rows
    for (const auto &config : rowConfigs) {
        node->addRow(config);
    }
    
    // Add to model
    m_elementModel->addElement(node);
    
    qDebug() << "Created node from JSON:" << nodeId << "at" << x << "," << y;
    emit nodeImported(nodeId);
    return nodeId;
}

QStringList JsonImporter::createNodesFromJson(const QString& jsonData)
{
    QStringList createdNodeIds;
    
    if (!m_creationManager || !m_elementModel) {
        emit importError("JsonImporter not properly initialized");
        return createdNodeIds;
    }
    
    QJsonDocument doc = QJsonDocument::fromJson(jsonData.toUtf8());
    if (!doc.isArray()) {
        emit importError("Invalid JSON format - expected array");
        return createdNodeIds;
    }
    
    QJsonArray nodesArray = doc.array();
    
    for (const QJsonValue &value : nodesArray) {
        if (value.isObject()) {
            QJsonDocument nodeDoc(value.toObject());
            QString nodeId = createNodeFromJson(nodeDoc.toJson(QJsonDocument::Compact));
            if (!nodeId.isEmpty()) {
                createdNodeIds.append(nodeId);
            }
        }
    }
    
    qDebug() << "Created" << createdNodeIds.size() << "nodes from JSON array";
    emit nodesImported(createdNodeIds);
    return createdNodeIds;
}

void JsonImporter::createGraphFromJson(const QString& jsonData)
{
    if (!m_creationManager || !m_elementModel) {
        emit importError("JsonImporter not properly initialized");
        return;
    }
    
    QJsonDocument doc = QJsonDocument::fromJson(jsonData.toUtf8());
    if (!doc.isObject()) {
        emit importError("Invalid JSON format - expected object");
        return;
    }
    
    QJsonObject graphObj = doc.object();
    
    // Create nodes first
    QJsonArray nodes = graphObj.value("nodes").toArray();
    QMap<QString, QString> idMap; // Map original IDs to generated IDs
    
    for (const QJsonValue &value : nodes) {
        if (value.isObject()) {
            QJsonObject nodeObj = value.toObject();
            QString originalId = nodeObj.value("id").toString();
            
            QJsonDocument nodeDoc(nodeObj);
            QString newId = createNodeFromJson(nodeDoc.toJson(QJsonDocument::Compact));
            
            if (!originalId.isEmpty() && !newId.isEmpty()) {
                idMap[originalId] = newId;
            }
        }
    }
    
    // Create edges
    QJsonArray edges = graphObj.value("edges").toArray();
    int edgeCount = 0;
    
    for (const QJsonValue &value : edges) {
        if (value.isObject()) {
            QJsonObject edgeObj = value.toObject();
            
            QString sourceNodeId = edgeObj.value("sourceNodeId").toString();
            QString targetNodeId = edgeObj.value("targetNodeId").toString();
            
            // Map IDs if necessary
            if (idMap.contains(sourceNodeId)) {
                sourceNodeId = idMap[sourceNodeId];
            }
            if (idMap.contains(targetNodeId)) {
                targetNodeId = idMap[targetNodeId];
            }
            
            // Check if the edge uses port IDs or indices
            if (edgeObj.contains("sourcePortId") && edgeObj.contains("targetPortId")) {
                // New format: use port IDs
                QString sourcePortId = edgeObj.value("sourcePortId").toString();
                QString targetPortId = edgeObj.value("targetPortId").toString();
                
                if (!sourceNodeId.isEmpty() && !targetNodeId.isEmpty() && 
                    !sourcePortId.isEmpty() && !targetPortId.isEmpty()) {
                    if (m_creationManager->createEdgeByPortId(sourceNodeId, targetNodeId, sourcePortId, targetPortId)) {
                        edgeCount++;
                    }
                }
            } else {
                // Legacy format: use port indices
                int sourcePortIndex = edgeObj.value("sourcePortIndex").toInt(0);
                int targetPortIndex = edgeObj.value("targetPortIndex").toInt(0);
                
                if (!sourceNodeId.isEmpty() && !targetNodeId.isEmpty()) {
                    if (m_creationManager->createEdge(sourceNodeId, targetNodeId, "right", "left", 
                                                     sourcePortIndex, targetPortIndex)) {
                        edgeCount++;
                    }
                }
            }
        }
    }
    
    qDebug() << "Created graph from JSON with" << idMap.size() << "nodes and" << edgeCount << "edges";
    emit graphImported(idMap.size(), edgeCount);
}