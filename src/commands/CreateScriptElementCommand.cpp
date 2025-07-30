#include "CreateScriptElementCommand.h"
#include "ScriptElement.h"
#include "Node.h"
#include "Edge.h"
#include "ElementModel.h"
#include "SelectionManager.h"
#include "Scripts.h"
#include "HandleType.h"
#include "../Application.h"
#include "../Project.h"
#include "../ProjectApiClient.h"
#include <QJsonObject>
#include <QDebug>

CreateScriptElementCommand::CreateScriptElementCommand(ElementModel* model, SelectionManager* selectionManager,
                                                     Scripts* scripts, const QString& elementType, 
                                                     const QRectF& rect, const QVariant& initialPayload, 
                                                     QObject *parent)
    : Command(parent)
    , m_elementModel(model)
    , m_selectionManager(selectionManager)
    , m_scripts(scripts)
    , m_elementType(elementType)
    , m_rect(rect)
    , m_initialPayload(initialPayload)
    , m_element(nullptr)
{
    QString displayName = elementType;
    if (elementType == "node") {
        displayName = "Node";
    } else if (elementType == "edge") {
        displayName = "Edge";
    }
    
    setDescription(QString("Create %1")
                   .arg(displayName));
}

CreateScriptElementCommand::~CreateScriptElementCommand()
{
    // QPointer will automatically be null if the objects were deleted
    // Only delete elements if they exist and the model is still valid
    if (m_elementModel && m_element) {
        // Check if element is not in the model (undone state)
        auto allElements = m_elementModel->getAllElements();
        
        if (!allElements.contains(m_element)) {
            delete m_element;
        }
    }
}

void CreateScriptElementCommand::execute()
{
    if (!m_elementModel || !m_scripts) return;

    // Create element on first execution
    if (!m_element) {
        m_elementId = m_elementModel->generateId();
        
        // Create the appropriate script element
        if (m_elementType == "node") {
            Node* node = new Node(m_elementId);
            
            // Apply initial payload if provided
            bool hasInputPorts = false;
            bool hasOutputPorts = false;
            
            if (m_initialPayload.isValid() && m_initialPayload.canConvert<QVariantMap>()) {
                QVariantMap payload = m_initialPayload.toMap();
                
                if (payload.contains("nodeTitle")) {
                    node->setNodeTitle(payload.value("nodeTitle").toString());
                }
                if (payload.contains("nodeType")) {
                    node->setNodeType(payload.value("nodeType").toString());
                }
                if (payload.contains("inputPorts")) {
                    node->setInputPorts(payload.value("inputPorts").toStringList());
                    hasInputPorts = true;
                }
                if (payload.contains("outputPorts")) {
                    node->setOutputPorts(payload.value("outputPorts").toStringList());
                    hasOutputPorts = true;
                }
                if (payload.contains("nodeColor")) {
                    node->setNodeColor(QColor(payload.value("nodeColor").toString()));
                }
            }
            
            // Set position and size
            node->setRect(m_rect);
            
            // Setup default ports if none were provided
            if (!hasInputPorts || !hasOutputPorts) {
                setupDefaultNodePorts(node);
            }
            
            m_element = node;
        } else if (m_elementType == "edge") {
            Edge* edge = new Edge(m_elementId);
            
            // Apply initial payload if provided
            if (m_initialPayload.isValid() && m_initialPayload.canConvert<QVariantMap>()) {
                QVariantMap payload = m_initialPayload.toMap();
                
                if (payload.contains("sourceNodeId")) {
                    edge->setSourceNodeId(payload.value("sourceNodeId").toString());
                }
                if (payload.contains("targetNodeId")) {
                    edge->setTargetNodeId(payload.value("targetNodeId").toString());
                }
                if (payload.contains("sourcePortIndex")) {
                    edge->setSourcePortIndex(payload.value("sourcePortIndex").toInt());
                }
                if (payload.contains("targetPortIndex")) {
                    edge->setTargetPortIndex(payload.value("targetPortIndex").toInt());
                }
                if (payload.contains("edgeColor")) {
                    edge->setEdgeColor(QColor(payload.value("edgeColor").toString()));
                }
                if (payload.contains("sourcePortType")) {
                    edge->setSourcePortType(payload.value("sourcePortType").toString());
                }
                if (payload.contains("targetPortType")) {
                    edge->setTargetPortType(payload.value("targetPortType").toString());
                }
                
                // Calculate edge points based on connected nodes
                if (payload.contains("sourceNodeId") && payload.contains("targetNodeId")) {
                    calculateEdgePoints(edge);
                }
            }
            
            m_element = edge;
        } else {
            qWarning() << "Unknown script element type:" << m_elementType;
            return;
        }
    }

    // Add element to Scripts
    if (m_elementType == "node") {
        if (Node* node = qobject_cast<Node*>(m_element)) {
            m_scripts->addNode(node);
        }
    } else if (m_elementType == "edge") {
        if (Edge* edge = qobject_cast<Edge*>(m_element)) {
            m_scripts->addEdge(edge);
        }
    }
    
    // Add element to model for visibility
    m_elementModel->addElement(m_element);

    // Select the newly created element (for nodes only, edges typically aren't selected)
    if (m_selectionManager && m_element && m_elementType == "node") {
        m_selectionManager->selectOnly(m_element);
    }
    
    // Sync with API
    syncWithApi();
}

void CreateScriptElementCommand::syncWithApi()
{
    if (!m_element || !m_elementModel) return;
    
    // Get the project from the element model
    Project* project = qobject_cast<Project*>(m_elementModel->parent());
    if (!project) return;
    
    // Get the Application instance
    Application* app = qobject_cast<Application*>(project->parent());
    if (!app) return;
    
    // Get the ProjectApiClient
    ProjectApiClient* apiClient = app->projectApiClient();
    if (!apiClient) return;
    
    // Create element data for API sync
    QJsonObject elementData;
    elementData["elementId"] = m_element->getId();
    elementData["elementType"] = m_elementType;
    
    if (m_elementType == "node") {
        if (Node* node = qobject_cast<Node*>(m_element)) {
            elementData["nodeTitle"] = node->nodeTitle();
            elementData["nodeType"] = node->nodeType();
            elementData["x"] = node->x();
            elementData["y"] = node->y();
            elementData["width"] = node->width();
            elementData["height"] = node->height();
        }
    } else if (m_elementType == "edge") {
        if (Edge* edge = qobject_cast<Edge*>(m_element)) {
            elementData["sourceNodeId"] = edge->sourceNodeId();
            elementData["targetNodeId"] = edge->targetNodeId();
            elementData["sourcePortIndex"] = edge->sourcePortIndex();
            elementData["targetPortIndex"] = edge->targetPortIndex();
        }
    }
    
    // Use the project's ID as the API project ID
    QString apiProjectId = project->id();
    
    // Sync the element creation with the API
    apiClient->syncCreateElement(apiProjectId, elementData);
}

void CreateScriptElementCommand::undo()
{
    if (!m_elementModel || !m_scripts) return;

    // Clear selection if the created element is selected
    if (m_selectionManager && m_selectionManager->hasSelection() && m_element) {
        auto selectedElements = m_selectionManager->selectedElements();
        if (selectedElements.contains(m_element)) {
            m_selectionManager->clearSelection();
        }
    }

    // Remove element from Scripts
    if (m_elementType == "node") {
        if (Node* node = qobject_cast<Node*>(m_element)) {
            m_scripts->removeNode(node);
        }
    } else if (m_elementType == "edge") {
        if (Edge* edge = qobject_cast<Edge*>(m_element)) {
            m_scripts->removeEdge(edge);
        }
    }
    
    // Remove element from model
    if (m_element) {
        m_elementModel->removeElement(m_element->getId());
    }
}

void CreateScriptElementCommand::calculateEdgePoints(Edge* edge)
{
    if (!edge || !m_elementModel) return;
    
    // Find source and target nodes
    Element* sourceElement = m_elementModel->getElementById(edge->sourceNodeId());
    Element* targetElement = m_elementModel->getElementById(edge->targetNodeId());
    
    if (!sourceElement || !targetElement) return;
    
    // Cast to CanvasElement to access position
    CanvasElement* sourceCanvas = qobject_cast<CanvasElement*>(sourceElement);
    CanvasElement* targetCanvas = qobject_cast<CanvasElement*>(targetElement);
    
    if (!sourceCanvas || !targetCanvas) return;
    
    // Calculate source point
    qreal sourceX = sourceCanvas->x() + sourceCanvas->width();  // Right edge
    qreal sourceY = sourceCanvas->y() + 60 + 15 + (edge->sourcePortIndex() * 40);  // Title height + spacing + port offset
    
    // Calculate target point
    qreal targetX = targetCanvas->x();  // Left edge
    qreal targetY = targetCanvas->y() + 60 + 15 + (edge->targetPortIndex() * 40);
    
    // Set the points
    edge->setSourcePoint(QPointF(sourceX, sourceY));
    edge->setTargetPoint(QPointF(targetX, targetY));
    
    // Update edge geometry will be called automatically by setSourcePoint/setTargetPoint
}

void CreateScriptElementCommand::setupDefaultNodePorts(Node* node)
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