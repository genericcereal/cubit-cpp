#include "CreationManager.h"
#include "Element.h"
#include "CanvasElement.h"
#include "Frame.h"
#include "Text.h"
#include "Variable.h"
#include "Node.h"
#include "Edge.h"
#include "ElementModel.h"
#include "SelectionManager.h"
#include "Config.h"
#include "HandleType.h"
#include "Project.h"
#include "Component.h"
#include "ElementTypeRegistry.h"
#include "PlatformConfig.h"
#include <QDebug>
#include <QtMath>

CreationManager::CreationManager(QObject *parent)
    : QObject(parent)
{
}

Element* CreationManager::createElement(const QString& type, qreal x, qreal y, qreal width, qreal height)
{
    qDebug() << "CreationManager::createElement() called - Type:" << type << "Pos:" << x << y << "Size:" << width << height;
    if (!m_elementModel) return nullptr;
    
    // Only create elements for design canvas
    if (m_canvasType != CanvasType::Design) {
        qDebug() << "Cannot create elements on script canvas";
        return nullptr;
    }
    
    // Check if we're in variant mode or globalElements mode
    bool isVariantMode = m_project && m_project->viewMode() == "variant";
    bool isGlobalElementsMode = m_project && m_project->viewMode() == "globalElements";
    Component* editingComponent = nullptr;
    PlatformConfig* editingPlatform = nullptr;
    
    if (isVariantMode && m_project) {
        QObject* editingElement = m_project->editingElement();
        editingComponent = qobject_cast<Component*>(editingElement);
        if (!editingComponent) {
            qWarning() << "CreationManager: In variant mode but editing element is not a Component";
        }
    } else if (isGlobalElementsMode && m_project) {
        QObject* editingElement = m_project->editingElement();
        editingPlatform = qobject_cast<PlatformConfig*>(editingElement);
        if (!editingPlatform) {
            qWarning() << "CreationManager: In globalElements mode but editing element is not a PlatformConfig";
        }
    }
    
    QString id = m_elementModel->generateId();
    Element *element = nullptr;
    
    qDebug() << "Creating element with unique ID:" << id;
    
    // Use the ElementTypeRegistry for design elements
    ElementTypeRegistry& registry = ElementTypeRegistry::instance();
    
    if (registry.hasType(type)) {
        // Create using the registry
        element = registry.createElement(type, id);
    } else if (type == "text") {
        // Special handling for text - create a Frame for preview
        // The actual Text element will be created by the command
        element = registry.createElement("frame", id);
    } else if (type == "variable") {
        // Variables are not design elements, handle separately
        element = new Variable(id);
    } else {
        qWarning() << "Unknown element type:" << type;
        return nullptr;
    }
    
    if (element) {
        // Set position and size for visual elements
        if (element->isVisual()) {
            CanvasElement* canvasElement = qobject_cast<CanvasElement*>(element);
            if (canvasElement) {
                canvasElement->setRect(QRectF(x, y, width, height));
            }
        }
        
        // Add to appropriate container
        if (isVariantMode && editingComponent) {
            // In variant mode, add to the Component's variants array
            editingComponent->addVariant(element);
            // Still add to model for visibility
            m_elementModel->addElement(element);
        } else if (isGlobalElementsMode && editingPlatform) {
            // In globalElements mode, add to the PlatformConfig's globalElements
            ElementModel* globalElements = editingPlatform->globalElements();
            if (globalElements) {
                globalElements->addElement(element);
                // Also add to main model for visibility during editing
                // The Project class manages loading/clearing these when switching modes
                m_elementModel->addElement(element);
            } else {
                qWarning() << "CreationManager: Platform has no globalElements model";
                delete element;
                return nullptr;
            }
        } else {
            // Normal mode, just add to model
            m_elementModel->addElement(element);
        }
        
        emit elementCreated(element);
        
        // Select the newly created element
        if (m_selectionManager) {
            m_selectionManager->selectOnly(element);
        }
    }
    
    return element;
}

Node* CreationManager::createNode(qreal x, qreal y, const QString& title, const QString& color)
{
    if (!m_elementModel) return nullptr;
    
    // Only create nodes for script canvas
    if (m_canvasType != CanvasType::Script) {
        qDebug() << "Cannot create nodes on design canvas";
        return nullptr;
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
        
        setupDefaultNodePorts(node);
        
        qDebug() << "Created node with ID:" << node->getId() << "title:" << title;
        
        m_elementModel->addElement(node);
        emit elementCreated(node);
        emit nodeCreated(node);
    }
    
    return node;
}

Edge* CreationManager::createEdge(const QString& sourceNodeId, const QString& targetNodeId, 
                                  const QString& sourceHandleType, const QString& targetHandleType,
                                  int sourcePortIndex, int targetPortIndex)
{
    if (!m_elementModel) return nullptr;
    
    // Only create edges for script canvas
    if (m_canvasType != CanvasType::Script) {
        qDebug() << "Cannot create edges on design canvas";
        return nullptr;
    }
    
    // Find the source and target nodes
    Element *sourceNode = m_elementModel->getElementById(sourceNodeId);
    Element *targetNode = m_elementModel->getElementById(targetNodeId);
    
    if (!sourceNode || !targetNode) {
        qDebug() << "Cannot create edge: source or target node not found";
        return nullptr;
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
            return nullptr;
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
        
        // Calculate connection points
        calculateEdgePoints(edge, sourceNode, targetNode, sourceHandleType, targetHandleType, 
                          sourcePortIndex, targetPortIndex);
        
        m_elementModel->addElement(edge);
        emit elementCreated(edge);
        emit edgeCreated(edge);
        
        qDebug() << "Created edge from node" << sourceNodeId << "port" << sourcePortIndex
                 << "to node" << targetNodeId << "port" << targetPortIndex;
    }
    
    return edge;
}

Edge* CreationManager::createEdgeByPortId(const QString& sourceNodeId, const QString& targetNodeId,
                                         const QString& sourcePortId, const QString& targetPortId)
{
    if (!m_elementModel) return nullptr;
    
    // Only create edges for script canvas
    if (m_canvasType != CanvasType::Script) {
        qDebug() << "Cannot create edges on design canvas";
        return nullptr;
    }
    
    // Find the source and target nodes
    Element *sourceElement = m_elementModel->getElementById(sourceNodeId);
    Element *targetElement = m_elementModel->getElementById(targetNodeId);
    
    if (!sourceElement || !targetElement) {
        qDebug() << "Cannot create edge: source or target node not found";
        return nullptr;
    }
    
    // Cast to Node to get port information
    Node *srcNode = qobject_cast<Node*>(sourceElement);
    Node *tgtNode = qobject_cast<Node*>(targetElement);
    
    if (!srcNode || !tgtNode) {
        qDebug() << "Cannot create edge: elements are not nodes";
        return nullptr;
    }
    
    // Find port indices from port IDs
    int sourcePortIndex = srcNode->getOutputPortIndex(sourcePortId);
    int targetPortIndex = tgtNode->getInputPortIndex(targetPortId);
    
    if (sourcePortIndex == -1 || targetPortIndex == -1) {
        qDebug() << "Cannot create edge: port not found -"
                 << "sourcePortId:" << sourcePortId << "index:" << sourcePortIndex
                 << "targetPortId:" << targetPortId << "index:" << targetPortIndex;
        return nullptr;
    }
    
    // Call the original createEdge method with the resolved indices
    return createEdge(sourceNodeId, targetNodeId, "right", "left", sourcePortIndex, targetPortIndex);
}

void CreationManager::setupDefaultNodePorts(Node* node)
{
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
}

void CreationManager::calculateEdgePoints(Edge* edge, Element* sourceNode, Element* targetNode,
                                        const QString& sourceHandleType, const QString& targetHandleType,
                                        int sourcePortIndex, int targetPortIndex) const
{
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
}

Element* CreationManager::startDragCreation(const QString& type, const QPointF& startPos)
{
    qDebug() << "WARNING: startDragCreation() called - this should not happen with new creation flow!";
    qDebug() << "Type:" << type << "StartPos:" << startPos;
    if (!m_elementModel) return nullptr;
    
    // Only create elements for design canvas
    if (m_canvasType != CanvasType::Design) {
        qDebug() << "Cannot create elements on script canvas via drag";
        return nullptr;
    }
    
    // Cancel any existing drag creation
    if (m_dragCreationElement) {
        cancelDragCreation();
    }
    
    m_dragCreationType = type;
    m_dragCreationStartPos = startPos;
    
    // Create element at start position with minimal size
    m_dragCreationElement = createElement(type, startPos.x(), startPos.y(), 1, 1);
    
    return m_dragCreationElement;
}

void CreationManager::updateDragCreation(const QPointF& currentPos)
{
    if (!m_dragCreationElement || !m_dragCreationElement->isVisual()) return;
    
    CanvasElement* canvasElement = qobject_cast<CanvasElement*>(m_dragCreationElement);
    if (!canvasElement) return;
    
    // Calculate dimensions based on drag
    qreal width = qAbs(currentPos.x() - m_dragCreationStartPos.x());
    qreal height = qAbs(currentPos.y() - m_dragCreationStartPos.y());
    qreal left = qMin(currentPos.x(), m_dragCreationStartPos.x());
    qreal top = qMin(currentPos.y(), m_dragCreationStartPos.y());
    
    // Update element size and position
    canvasElement->setX(left);
    canvasElement->setY(top);
    canvasElement->setWidth(qMax(width, 1.0));
    canvasElement->setHeight(qMax(height, 1.0));
}

Element* CreationManager::finishDragCreation()
{
    if (!m_dragCreationElement || !m_dragCreationElement->isVisual()) {
        m_dragCreationElement = nullptr;
        return nullptr;
    }
    
    CanvasElement* canvasElement = qobject_cast<CanvasElement*>(m_dragCreationElement);
    if (!canvasElement) {
        m_dragCreationElement = nullptr;
        return nullptr;
    }
    
    // Ensure minimum size
    if (canvasElement->width() < 10) {
        canvasElement->setWidth(Config::DEFAULT_ELEMENT_WIDTH);
    }
    if (canvasElement->height() < 10) {
        canvasElement->setHeight(Config::DEFAULT_ELEMENT_HEIGHT);
    }
    
    // Select the newly created element
    if (m_selectionManager) {
        m_selectionManager->selectOnly(m_dragCreationElement);
    }
    
    Element* createdElement = m_dragCreationElement;
    m_dragCreationElement = nullptr;
    m_dragCreationType.clear();
    
    return createdElement;
}

void CreationManager::cancelDragCreation()
{
    if (m_dragCreationElement && m_elementModel) {
        m_elementModel->removeElement(m_dragCreationElement->getId());
        m_dragCreationElement = nullptr;
        m_dragCreationType.clear();
    }
}