#include "CanvasController.h"
#include "Element.h"
#include "CanvasElement.h"
#include "Frame.h"
#include "Text.h"
#include "Html.h"
#include "Variable.h"
#include "Node.h"
#include "Edge.h"
#include "ElementModel.h"
#include "SelectionManager.h"
#include "Config.h"
#include "HandleType.h"
#include "UniqueIdGenerator.h"
#include <QDebug>
#include <QtMath>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QJsonValue>

CanvasController::CanvasController(QObject *parent)
    : QObject(parent)
    , m_mode("select")
    , m_canvasType("design")
    , m_elementModel(nullptr)
    , m_selectionManager(nullptr)
    , m_isDragging(false)
    , m_dragElement(nullptr)
    , m_hasDraggedMinDistance(false)
{
}

void CanvasController::setMode(const QString &mode)
{
    qDebug() << "CanvasController::setMode called - current:" << m_mode << "new:" << mode;
    if (m_mode != mode) {
        m_mode = mode;
        qDebug() << "Mode changed, emitting signal";
        emit modeChanged();
    }
}

void CanvasController::setCanvasType(const QString &type)
{
    if (m_canvasType != type) {
        m_canvasType = type;
        emit canvasTypeChanged();
    }
}

void CanvasController::handleMousePress(qreal x, qreal y)
{
    if (!m_elementModel || !m_selectionManager) return;
    
    if (m_mode == "select") {
        Element *element = hitTest(x, y);
        if (element) {
            // Don't change selection on press - wait to see if it's a click or drag
            // Just prepare for potential drag
            startDrag(x, y);
        } else {
            m_selectionManager->clearSelection();
        }
    } else {
        // Start element creation
        m_dragStartPos = QPointF(x, y);
        m_isDragging = true;
        
        // Create element immediately for frame mode
        if (m_mode == "frame" || m_mode == "text" || m_mode == "html") {
            createElement(m_mode, x, y, 1, 1);
            // The last created element becomes the drag element
            if (m_elementModel && m_elementModel->rowCount() > 0) {
                m_dragElement = m_elementModel->elementAt(m_elementModel->rowCount() - 1);
                qDebug() << "Frame created, dragElement:" << m_dragElement << "isDragging:" << m_isDragging;
            }
        }
    }
}

void CanvasController::handleMouseMove(qreal x, qreal y)
{
    if (!m_isDragging) return;
    
    if (m_dragElement) {
        if (m_mode == "select") {
            updateDrag(x, y);
        } else if (m_mode == "frame" || m_mode == "text" || m_mode == "html") {
            // Update frame size during creation
            qreal width = qAbs(x - m_dragStartPos.x());
            qreal height = qAbs(y - m_dragStartPos.y());
            qreal left = qMin(x, m_dragStartPos.x());
            qreal top = qMin(y, m_dragStartPos.y());
            
            if (m_dragElement->isVisual()) {
                CanvasElement* canvasElement = qobject_cast<CanvasElement*>(m_dragElement);
                if (canvasElement) {
                    canvasElement->setX(left);
                    canvasElement->setY(top);
                    canvasElement->setWidth(qMax(width, 1.0));
                    canvasElement->setHeight(qMax(height, 1.0));
                }
            }
            
            qDebug() << "Frame resize:" << left << top << width << height;
        }
    } else if (m_mode != "select") {
        m_dragCurrentPos = QPointF(x, y);
    }
}

void CanvasController::handleMouseRelease(qreal x, qreal y)
{
    if (!m_isDragging) return;
    
    if (m_mode == "select") {
        // Check if this was a click (no significant movement) vs a drag
        if (!m_hasDraggedMinDistance && m_dragElement) {
            // This was a click - update selection
            if (m_selectionManager->selectionCount() > 1 && m_dragElement->isSelected()) {
                // Multiple elements were selected and user clicked on one of them
                // Select only the clicked element
                m_selectionManager->selectOnly(m_dragElement);
            } else if (!m_dragElement->isSelected()) {
                // Clicked on an unselected element
                m_selectionManager->selectOnly(m_dragElement);
            }
        }
        endDrag();
    } else if (m_mode == "frame" || m_mode == "text" || m_mode == "html") {
        // Finalize frame creation
        if (m_dragElement) {
            // Ensure minimum size
            if (m_dragElement->isVisual()) {
                CanvasElement* canvasElement = qobject_cast<CanvasElement*>(m_dragElement);
                if (canvasElement) {
                    if (canvasElement->width() < 10) canvasElement->setWidth(Config::DEFAULT_ELEMENT_WIDTH);
                    if (canvasElement->height() < 10) canvasElement->setHeight(Config::DEFAULT_ELEMENT_HEIGHT);
                }
            }
            
            // Select the newly created frame
            if (m_selectionManager) {
                m_selectionManager->selectOnly(m_dragElement);
            }
            
            // Switch back to select mode
            setMode("select");
        }
    } else {
        // Create other element types on release
        qreal width = qAbs(x - m_dragStartPos.x());
        qreal height = qAbs(y - m_dragStartPos.y());
        qreal left = qMin(x, m_dragStartPos.x());
        qreal top = qMin(y, m_dragStartPos.y());
        
        if (width < 10) width = Config::DEFAULT_ELEMENT_WIDTH;
        if (height < 10) height = Config::DEFAULT_ELEMENT_HEIGHT;
        
        createElement(m_mode, left, top, width, height);
    }
    
    m_isDragging = false;
    m_dragElement = nullptr;
}

void CanvasController::createElement(const QString &type, qreal x, qreal y, qreal width, qreal height)
{
    if (!m_elementModel) return;
    
    // Only create elements for design canvas
    if (m_canvasType != "design") {
        qDebug() << "Cannot create elements on" << m_canvasType << "canvas";
        return;
    }
    
    QString id = m_elementModel->generateId();
    Element *element = nullptr;
    
    qDebug() << "Creating element with unique ID:" << id;
    
    if (type == "frame") {
        element = new Frame(id);
    } else if (type == "text") {
        // Create frame with text
        element = new Text(id);
    } else if (type == "html") {
        element = new Frame(id);
    } else if (type == "variable") {
        element = new Variable(id);
    }
    
    if (element) {
        // Set position and size for visual elements
        if (element->isVisual()) {
            CanvasElement* canvasElement = qobject_cast<CanvasElement*>(element);
            if (canvasElement) {
                canvasElement->setRect(QRectF(x, y, width, height));
            }
        }
        m_elementModel->addElement(element);
        emit elementCreated(element);
        
        // Select the newly created element
        if (m_selectionManager) {
            m_selectionManager->selectOnly(element);
        }
    }
}

Element* CanvasController::hitTest(qreal x, qreal y)
{
    if (!m_elementModel) return nullptr;
    
    // Test in reverse order (top to bottom) for all canvas types
    QList<Element*> elements = m_elementModel->getAllElements();
    for (int i = elements.size() - 1; i >= 0; --i) {
        Element *element = elements[i];
        
        // For script canvas, test nodes and edges
        if (m_canvasType == "script") {
            if (element->getType() != Element::NodeType && element->getType() != Element::EdgeType) {
                continue;
            }
        }
        // For design canvas, skip nodes
        else if (m_canvasType == "design") {
            if (element->getType() == Element::NodeType || element->getType() == Element::EdgeType) {
                continue;
            }
        }
        
        // Only visual elements can be hit tested
        if (element->isVisual()) {
            CanvasElement* canvasElement = qobject_cast<CanvasElement*>(element);
            if (canvasElement && canvasElement->containsPoint(QPointF(x, y))) {
                return element;
            }
        }
    }
    
    return nullptr;
}

void CanvasController::startDrag(qreal x, qreal y)
{
    m_dragElement = hitTest(x, y);
    if (m_dragElement) {
        m_isDragging = true;
        emit isDraggingChanged();
        m_dragStartPos = QPointF(x, y);
        m_hasDraggedMinDistance = false;
        
        // Clear previous drag states
        m_draggedElements.clear();
        
        // Get all selected elements
        QList<Element*> selectedElements = m_selectionManager->selectedElements();
        
        // If the clicked element is not selected, we'll only drag it alone
        // Otherwise, drag all selected elements
        if (m_dragElement->isSelected()) {
            // Store initial positions of all selected elements
            for (Element *element : selectedElements) {
                // Only visual elements can be dragged
                if (element->isVisual()) {
                    CanvasElement* canvasElement = qobject_cast<CanvasElement*>(element);
                    if (canvasElement) {
                        ElementDragState state;
                        state.element = element;
                        state.originalPosition = QPointF(canvasElement->x(), canvasElement->y());
                        m_draggedElements.append(state);
                    }
                }
            }
        } else {
            // Only drag the clicked element (it's not selected)
            if (m_dragElement->isVisual()) {
                CanvasElement* canvasElement = qobject_cast<CanvasElement*>(m_dragElement);
                if (canvasElement) {
                    ElementDragState state;
                    state.element = m_dragElement;
                    state.originalPosition = QPointF(canvasElement->x(), canvasElement->y());
                    m_draggedElements.append(state);
                }
            }
        }
    }
}

void CanvasController::updateDrag(qreal x, qreal y)
{
    if (m_isDragging && !m_draggedElements.isEmpty()) {
        // Calculate the delta from the drag start position
        qreal deltaX = x - m_dragStartPos.x();
        qreal deltaY = y - m_dragStartPos.y();
        
        // Check if we've moved enough to consider this a drag (3 pixel threshold)
        if (!m_hasDraggedMinDistance) {
            qreal distance = qSqrt(deltaX * deltaX + deltaY * deltaY);
            if (distance > 3.0) {
                m_hasDraggedMinDistance = true;
            }
        }
        
        // Only move elements if we've established this is a drag
        if (m_hasDraggedMinDistance) {
            // Move all selected elements by the same delta
            for (const ElementDragState &state : m_draggedElements) {
                if (state.element->isVisual()) {
                    CanvasElement* canvasElement = qobject_cast<CanvasElement*>(state.element);
                    if (canvasElement) {
                        canvasElement->setX(state.originalPosition.x() + deltaX);
                        canvasElement->setY(state.originalPosition.y() + deltaY);
                    }
                }
            }
        }
    }
}

void CanvasController::endDrag()
{
    m_isDragging = false;
    emit isDraggingChanged();
    m_dragElement = nullptr;
    m_draggedElements.clear();
    m_hasDraggedMinDistance = false;
}

void CanvasController::selectElementsInRect(const QRectF &rect)
{
    if (!m_elementModel || !m_selectionManager) return;
    
    QList<Element*> elementsToSelect;
    QList<Element*> allElements = m_elementModel->getAllElements();
    
    for (Element *element : allElements) {
        // For script canvas, only select nodes
        if (m_canvasType == "script") {
            if (element->getType() != Element::NodeType) {
                continue;
            }
        }
        // For design canvas, skip nodes and edges
        else if (m_canvasType == "design") {
            if (element->getType() == Element::NodeType || element->getType() == Element::EdgeType) {
                continue;
            }
        }
        
        // Only visual elements can be selected by rectangle
        if (element->isVisual()) {
            CanvasElement* canvasElement = qobject_cast<CanvasElement*>(element);
            if (canvasElement && rect.intersects(canvasElement->rect())) {
                elementsToSelect.append(element);
            }
        }
    }
    
    if (!elementsToSelect.isEmpty()) {
        m_selectionManager->selectAll(elementsToSelect);
    } else {
        // Clear selection if no elements are in the rect
        m_selectionManager->clearSelection();
    }
}

void CanvasController::selectAll()
{
    if (!m_elementModel || !m_selectionManager) return;
    
    m_selectionManager->selectAll(m_elementModel->getAllElements());
}

void CanvasController::deleteSelectedElements()
{
    if (!m_elementModel || !m_selectionManager) return;
    
    QList<Element*> selectedElements = m_selectionManager->selectedElements();
    
    // Clear selection first
    m_selectionManager->clearSelection();
    
    // Delete elements
    for (Element *element : selectedElements) {
        m_elementModel->removeElement(element->getId());
    }
}

void CanvasController::createNode(qreal x, qreal y, const QString &title, const QString &color)
{
    if (!m_elementModel) return;
    
    // Only create nodes for script canvas
    if (m_canvasType != "script") {
        qDebug() << "Cannot create nodes on" << m_canvasType << "canvas";
        return;
    }
    
    QString id = m_elementModel->generateId();
    Node *node = new Node(id);
    
    if (node) {
        node->setX(x);
        node->setY(y);
        node->setNodeTitle(title);
        if (!color.isEmpty()) {
            node->setNodeColor(QColor(color));
        }
        // If no color specified, it will use the default from Config
        
        // Set up default ports and rows
        // Configure port types
        node->setInputPortType(0, PortType::Flow);      // Flow In
        node->setInputPortType(1, PortType::String);    // Value (defaulting to String for now)
        node->setOutputPortType(1, PortType::String);   // Result (defaulting to String for now)
        node->setOutputPortType(2, PortType::Flow);     // Flow Out
        
        // Configure rows
        Node::RowConfig row0;
        row0.hasTarget = true;
        row0.targetLabel = "Flow In";
        row0.targetType = PortType::Flow;
        row0.targetPortIndex = 0;
        row0.hasSource = true;
        row0.sourceLabel = "Flow Out";
        row0.sourceType = PortType::Flow;
        row0.sourcePortIndex = 2;
        
        Node::RowConfig row1;
        row1.hasTarget = true;
        row1.targetLabel = "Value";
        row1.targetType = PortType::String;  // defaulting to String for now
        row1.targetPortIndex = 1;
        row1.hasSource = true;
        row1.sourceLabel = "Result";
        row1.sourceType = PortType::String;  // defaulting to String for now
        row1.sourcePortIndex = 1;
        
        node->addRow(row0);
        node->addRow(row1);
        
        qDebug() << "Created node with ID:" << node->getId() << "title:" << title;
        
        m_elementModel->addElement(node);
        emit elementCreated(node);
    }
}

void CanvasController::createEdgeByPortId(const QString &sourceNodeId, const QString &targetNodeId,
                                          const QString &sourcePortId, const QString &targetPortId)
{
    if (!m_elementModel) return;
    
    // Only create edges for script canvas
    if (m_canvasType != "script") {
        qDebug() << "Cannot create edges on" << m_canvasType << "canvas";
        return;
    }
    
    // Find the source and target nodes
    Element *sourceElement = m_elementModel->getElementById(sourceNodeId);
    Element *targetElement = m_elementModel->getElementById(targetNodeId);
    
    if (!sourceElement || !targetElement) {
        qDebug() << "Cannot create edge: source or target node not found";
        return;
    }
    
    // Cast to Node to get port information
    Node *srcNode = qobject_cast<Node*>(sourceElement);
    Node *tgtNode = qobject_cast<Node*>(targetElement);
    
    if (!srcNode || !tgtNode) {
        qDebug() << "Cannot create edge: elements are not nodes";
        return;
    }
    
    // Find port indices from port IDs
    int sourcePortIndex = srcNode->getOutputPortIndex(sourcePortId);
    int targetPortIndex = tgtNode->getInputPortIndex(targetPortId);
    
    if (sourcePortIndex == -1 || targetPortIndex == -1) {
        qDebug() << "Cannot create edge: port not found -"
                 << "sourcePortId:" << sourcePortId << "index:" << sourcePortIndex
                 << "targetPortId:" << targetPortId << "index:" << targetPortIndex;
        return;
    }
    
    // Call the original createEdge method with the resolved indices
    createEdge(sourceNodeId, targetNodeId, "right", "left", sourcePortIndex, targetPortIndex);
}

void CanvasController::createEdge(const QString &sourceNodeId, const QString &targetNodeId, 
                                  const QString &sourceHandleType, const QString &targetHandleType,
                                  int sourcePortIndex, int targetPortIndex)
{
    if (!m_elementModel) return;
    
    // Only create edges for script canvas
    if (m_canvasType != "script") {
        qDebug() << "Cannot create edges on" << m_canvasType << "canvas";
        return;
    }
    
    // Find the source and target nodes
    Element *sourceNode = m_elementModel->getElementById(sourceNodeId);
    Element *targetNode = m_elementModel->getElementById(targetNodeId);
    
    if (!sourceNode || !targetNode) {
        qDebug() << "Cannot create edge: source or target node not found";
        return;
    }
    
    // Check if nodes are the correct type and get port types
    Node *srcNode = qobject_cast<Node*>(sourceNode);
    Node *tgtNode = qobject_cast<Node*>(targetNode);
    
    if (srcNode && tgtNode) {
        // Get port types
        QString sourcePortType = srcNode->getOutputPortType(sourcePortIndex);
        QString targetPortType = tgtNode->getInputPortType(targetPortIndex);
        
        qDebug() << "Edge validation - source port" << sourcePortIndex << "type:" << sourcePortType
                 << "target port" << targetPortIndex << "type:" << targetPortType;
        
        // Validate that port types can connect
        if (!PortType::canConnect(sourcePortType, targetPortType)) {
            qDebug() << "Cannot create edge: port types don't match -"
                     << "source:" << sourcePortType 
                     << "target:" << targetPortType;
            return;
        }
        
        qDebug() << "Port types match, creating edge";
    }
    
    QString id = m_elementModel->generateId();
    Edge *edge = new Edge(id);
    
    if (edge) {
        qDebug() << "Creating edge with ID:" << id;
        
        // Set connections
        edge->setSourceNodeId(sourceNodeId);
        edge->setTargetNodeId(targetNodeId);
        edge->setSourceHandleType(sourceHandleType);
        edge->setTargetHandleType(targetHandleType);
        edge->setSourcePortIndex(sourcePortIndex);
        edge->setTargetPortIndex(targetPortIndex);
        
        // Set port types
        if (srcNode && tgtNode) {
            edge->setSourcePortType(srcNode->getOutputPortType(sourcePortIndex));
            edge->setTargetPortType(tgtNode->getInputPortType(targetPortIndex));
        }
        
        // Calculate connection points based on port indices
        qreal sourceX = 0, sourceY = 0, targetX = 0, targetY = 0;
        
        // Cast to CanvasElement to access position
        CanvasElement* sourceCanvas = qobject_cast<CanvasElement*>(sourceNode);
        CanvasElement* targetCanvas = qobject_cast<CanvasElement*>(targetNode);
        
        if (sourceCanvas && targetCanvas) {
            // Source point calculation
            if (sourceHandleType == "right") {
                sourceX = sourceCanvas->x() + sourceCanvas->width();
            } else {
                sourceX = sourceCanvas->x();
            }
            // Calculate Y based on port index (title height + row offset)
            sourceY = sourceCanvas->y() + 60 + 15 + (sourcePortIndex * 40);
            
            // Target point calculation  
            if (targetHandleType == "left") {
                targetX = targetCanvas->x();
            } else {
                targetX = targetCanvas->x() + targetCanvas->width();
            }
            // Calculate Y based on port index
            targetY = targetCanvas->y() + 60 + 15 + (targetPortIndex * 40);
        }
        
        edge->setSourcePoint(QPointF(sourceX, sourceY));
        edge->setTargetPoint(QPointF(targetX, targetY));
        
        qDebug() << "Edge points - Source:" << QPointF(sourceX, sourceY) 
                 << "Target:" << QPointF(targetX, targetY);
        qDebug() << "Edge bounds - Position:" << edge->x() << "," << edge->y()
                 << "Size:" << edge->width() << "x" << edge->height();
        
        m_elementModel->addElement(edge);
        emit elementCreated(edge);
        qDebug() << "Created edge from node" << sourceNodeId << "port" << sourcePortIndex
                 << "to node" << targetNodeId << "port" << targetPortIndex;
    }
}

QString CanvasController::createNodeFromJson(const QString &jsonData)
{
    if (m_canvasType != "script") {
        qWarning() << "createNodeFromJson can only be used on script canvas";
        return QString();
    }
    
    if (!m_elementModel) {
        qWarning() << "No element model set";
        return QString();
    }
    
    QJsonDocument doc = QJsonDocument::fromJson(jsonData.toUtf8());
    if (!doc.isObject()) {
        qWarning() << "Invalid JSON format - expected object";
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
    emit elementCreated(node);
    
    qDebug() << "Created node from JSON:" << nodeId << "at" << x << "," << y;
    return nodeId;
}

void CanvasController::createNodesFromJson(const QString &jsonData)
{
    if (m_canvasType != "script") {
        qWarning() << "createNodesFromJson can only be used on script canvas";
        return;
    }
    
    QJsonDocument doc = QJsonDocument::fromJson(jsonData.toUtf8());
    if (!doc.isArray()) {
        qWarning() << "Invalid JSON format - expected array";
        return;
    }
    
    QJsonArray nodesArray = doc.array();
    QStringList createdNodeIds;
    
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
}

void CanvasController::createGraphFromJson(const QString &jsonData)
{
    if (m_canvasType != "script") {
        qWarning() << "createGraphFromJson can only be used on script canvas";
        return;
    }
    
    QJsonDocument doc = QJsonDocument::fromJson(jsonData.toUtf8());
    if (!doc.isObject()) {
        qWarning() << "Invalid JSON format - expected object";
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
                    createEdgeByPortId(sourceNodeId, targetNodeId, sourcePortId, targetPortId);
                }
            } else {
                // Legacy format: use port indices
                int sourcePortIndex = edgeObj.value("sourcePortIndex").toInt(0);
                int targetPortIndex = edgeObj.value("targetPortIndex").toInt(0);
                
                if (!sourceNodeId.isEmpty() && !targetNodeId.isEmpty()) {
                    createEdge(sourceNodeId, targetNodeId, "right", "left", 
                              sourcePortIndex, targetPortIndex);
                }
            }
        }
    }
    
    qDebug() << "Created graph from JSON with" << idMap.size() << "nodes and" << edges.size() << "edges";
}