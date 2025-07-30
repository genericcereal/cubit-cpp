#include "ScriptInvokeBuilder.h"
#include "Scripts.h"
#include "Node.h"
#include "Edge.h"
#include "ElementModel.h"
#include "Element.h"
#include "ComponentInstanceTemplate.h"
#include "ComponentInstance.h"
#include "Component.h"
#include "ScriptCompiler.h"
#include <QDebug>
#include <QJsonDocument>

ScriptInvokeBuilder::ScriptInvokeBuilder()
{
}

ScriptInvokeBuilder::BuildContext ScriptInvokeBuilder::buildInvokes(Node* eventNode, Scripts* scripts, ElementModel* elementModel)
{
    BuildContext context;
    
    if (!eventNode || !scripts) {
        return context;
    }
    
    // Start building from the event node's outgoing connections
    QList<Edge*> outgoingEdges = getOutgoingFlowEdges(eventNode, scripts);
    
    for (Edge* edge : outgoingEdges) {
        Node* targetNode = scripts->getNode(edge->targetNodeId());
        if (targetNode) {
            buildInvokesRecursive(targetNode, scripts, context, elementModel);
        }
    }
    
    return context;
}

void ScriptInvokeBuilder::buildInvokesRecursive(Node* node, Scripts* scripts, 
                                                BuildContext& context, ElementModel* elementModel,
                                                const QString& parentInvokeId)
{
    Q_UNUSED(parentInvokeId);
    if (!node || !scripts) {
        return;
    }
    
    // Skip event nodes - they don't generate invokes
    if (node->nodeType() == "Event") {
        return;
    }
    
    // Store node reference for function generation
    NodeReference nodeRef;
    nodeRef.node = node;
    nodeRef.scripts = scripts;
    context.nodeReferences[node->getId()] = nodeRef;
    
    // Special handling for ComponentOnEditorLoadEvents node
    if (node->nodeTitle() == "Component On Editor Load Events") {
        qDebug() << "ScriptInvokeBuilder: Processing ComponentOnEditorLoadEvents node";
        
        // Get the parent element that owns these scripts
        QObject* scriptParent = scripts->parent();
        FrameComponentInstanceTemplate* frameInstance = qobject_cast<FrameComponentInstanceTemplate*>(scriptParent);
        TextComponentInstanceTemplate* textInstance = qobject_cast<TextComponentInstanceTemplate*>(scriptParent);
        
        Component* component = nullptr;
        QString componentId;
        
        // Check if the scripts belong to a component instance
        if (frameInstance) {
            componentId = frameInstance->componentId();
            qDebug() << "ScriptInvokeBuilder: Scripts belong to FrameComponentInstanceTemplate" << frameInstance->getId() 
                     << "instance of" << componentId;
            
            // Access the component directly from the instance
            component = frameInstance->sourceComponent();
        } else if (textInstance) {
            componentId = textInstance->componentId();
            qDebug() << "ScriptInvokeBuilder: Scripts belong to TextComponentInstanceTemplate" << textInstance->getId() 
                     << "instance of" << componentId;
            
            // Access the component directly from the instance
            component = textInstance->sourceComponent();
        } else {
            qDebug() << "ScriptInvokeBuilder: ComponentOnEditorLoadEvents node not in a component instance context";
            // Continue with the normal flow
            QList<Edge*> outgoingEdges = getOutgoingFlowEdges(node, scripts);
            for (Edge* edge : outgoingEdges) {
                Node* nextNode = scripts->getNode(edge->targetNodeId());
                if (nextNode) {
                    buildInvokesRecursive(nextNode, scripts, context, elementModel, parentInvokeId);
                }
            }
            return;
        }
        
        // Check if we have the component (either from frame or text instance)
        if (component && component->scripts()) {
            qDebug() << "ScriptInvokeBuilder: Found component" << componentId 
                     << "for instance" << (frameInstance ? frameInstance->getId() : textInstance->getId());
            
            // Ensure component scripts are compiled
            Scripts* componentScripts = component->scripts();
            qDebug() << "ScriptInvokeBuilder: Component has" << componentScripts->nodeCount() << "nodes and" 
                     << componentScripts->edgeCount() << "edges";
            if (!componentScripts->isCompiled()) {
                ScriptCompiler compiler;
                QString compiledJson = compiler.compile(componentScripts, elementModel);
                if (compiledJson.isEmpty()) {
                    qDebug() << "ScriptInvokeBuilder: Failed to compile component scripts for" << componentId;
                } else {
                    componentScripts->setCompiledScript(compiledJson);
                    componentScripts->setIsCompiled(true);
                }
            }
            
            if (componentScripts->isCompiled()) {
                // Find the onEditorLoad event node in the component's scripts
                QList<Node*> componentNodes = componentScripts->getAllNodes();
                Node* onEditorLoadNode = nullptr;
                
                for (Node* compNode : componentNodes) {
                    if (compNode && compNode->nodeType() == "Event" && 
                        compNode->nodeTitle() == "On Editor Load") {
                        onEditorLoadNode = compNode;
                        break;
                    }
                }
                
                if (onEditorLoadNode) {
                    qDebug() << "ScriptInvokeBuilder: Building invokes for component's onEditorLoad event";
                    
                    // Get outgoing edges from the onEditorLoad event
                    QList<Edge*> componentEdges = getOutgoingFlowEdges(onEditorLoadNode, componentScripts);
                    
                    for (Edge* edge : componentEdges) {
                        Node* targetNode = componentScripts->getNode(edge->targetNodeId());
                        if (targetNode) {
                            // Build invokes for the component's nodes, inserting them into our context
                            buildComponentInvokesRecursive(targetNode, componentScripts, context, elementModel);
                        }
                    }
                } else {
                    qDebug() << "ScriptInvokeBuilder: No onEditorLoad event found in component scripts";
                }
            }
        } else {
            qDebug() << "ScriptInvokeBuilder: Component" << componentId << "not found or has no scripts";
        }
        
        // Continue with the next nodes after ComponentOnEditorLoadEvents
        QList<Edge*> outgoingEdges = getOutgoingFlowEdges(node, scripts);
        for (Edge* edge : outgoingEdges) {
            Node* nextNode = scripts->getNode(edge->targetNodeId());
            if (nextNode) {
                buildInvokesRecursive(nextNode, scripts, context, elementModel, parentInvokeId);
            }
        }
        
        return; // Don't create an invoke for the ComponentOnEditorLoadEvents node itself
    }
    
    // Create invoke for this node
    InvokeData invoke;
    invoke.invokeId = generateInvokeId(context);
    invoke.nodeId = node->getId();
    invoke.functionName = getFunctionNameForNode(node);
    invoke.params = createNodeParameters(node, scripts, context);
    invoke.isAsync = node->isAsync();
    
    // Store this invoke first before processing next nodes
    context.invokes[invoke.invokeId] = invoke;
    
    // Create outputs for nodes that have output ports (source ports)
    // This is especially important for async nodes like AI Prompt
    if (node->isAsync() || node->nodeTitle().contains("AI", Qt::CaseInsensitive)) {
        // Check if this node has any outgoing data edges (not flow edges)
        QList<Edge*> allOutgoingEdges = scripts->getOutgoingEdges(node->getId());
        for (Edge* edge : allOutgoingEdges) {
            if (edge->sourcePortType() != "Flow") {
                // This node has a data output that's connected to another node
                // Create an output that will be populated by the async result
                QString outputId = generateOutputId(context);
                QJsonObject output;
                output["type"] = "invokeResult";
                output["sourceInvoke"] = invoke.invokeId;
                output["sourceNodeId"] = node->getId();
                output["sourcePortIndex"] = edge->sourcePortIndex();
                output["sourcePortType"] = edge->sourcePortType();
                context.outputs[outputId] = output;
                
            }
        }
    }
    
    // Find next nodes to invoke
    QList<Edge*> outgoingEdges = getOutgoingFlowEdges(node, scripts);
    QStringList nextInvokeIds;
    
    // First pass: collect all next nodes and pre-generate their invoke IDs
    QList<QPair<Node*, QString>> nextNodesWithIds;
    for (Edge* edge : outgoingEdges) {
        Node* nextNode = scripts->getNode(edge->targetNodeId());
        if (nextNode) {
            // Check if we already have an invoke for this node
            QString existingInvokeId;
            for (auto it = context.invokes.begin(); it != context.invokes.end(); ++it) {
                if (it.value().nodeId == nextNode->getId()) {
                    existingInvokeId = it.key();
                    break;
                }
            }
            
            if (!existingInvokeId.isEmpty()) {
                nextInvokeIds.append(existingInvokeId);
            } else {
                QString nextInvokeId = QString("invoke_%1").arg(context.invokeCounter);
                nextInvokeIds.append(nextInvokeId);
                nextNodesWithIds.append(qMakePair(nextNode, nextInvokeId));
            }
        }
    }
    
    // Update the current invoke with the next invoke IDs
    if (!nextInvokeIds.isEmpty()) {
        context.invokes[invoke.invokeId].nextInvokes = nextInvokeIds;
    }
    
    // Second pass: process the next nodes with their pre-assigned IDs
    for (const auto& pair : nextNodesWithIds) {
        Node* nextNode = pair.first;
        QString preAssignedId = pair.second;
        // Increment counter for the pre-assigned ID
        context.invokeCounter++;
        buildInvokesRecursiveWithId(nextNode, scripts, context, elementModel, invoke.invokeId, preAssignedId);
    }
}

void ScriptInvokeBuilder::buildInvokesRecursiveWithId(Node* node, Scripts* scripts, BuildContext& context,
                                                       ElementModel* elementModel,
                                                       const QString& parentInvokeId,
                                                       const QString& preAssignedId)
{
    if (!node || !scripts) {
        return;
    }
    
    // Skip event nodes - they don't generate invokes
    if (node->nodeType() == "Event") {
        return;
    }
    
    // Store node reference for function generation
    NodeReference nodeRef;
    nodeRef.node = node;
    nodeRef.scripts = scripts;
    context.nodeReferences[node->getId()] = nodeRef;
    
    // Special handling for ComponentOnEditorLoadEvents node
    if (node->nodeTitle() == "Component On Editor Load Events") {
        // Handle ComponentOnEditorLoadEvents (code omitted for brevity - same as buildInvokesRecursive)
        return;
    }
    
    // Create invoke for this node with the pre-assigned ID
    InvokeData invoke;
    invoke.invokeId = preAssignedId;
    invoke.nodeId = node->getId();
    invoke.functionName = getFunctionNameForNode(node);
    invoke.params = createNodeParameters(node, scripts, context);
    invoke.isAsync = node->isAsync();
    
    // Store this invoke first before processing next nodes
    context.invokes[invoke.invokeId] = invoke;
    
    // Create outputs for nodes that have output ports (source ports)
    // This is especially important for async nodes like AI Prompt
    if (node->isAsync() || node->nodeTitle().contains("AI", Qt::CaseInsensitive)) {
        // Check if this node has any outgoing data edges (not flow edges)
        QList<Edge*> allOutgoingEdges = scripts->getOutgoingEdges(node->getId());
        for (Edge* edge : allOutgoingEdges) {
            if (edge->sourcePortType() != "Flow") {
                // This node has a data output that's connected to another node
                // Create an output that will be populated by the async result
                QString outputId = generateOutputId(context);
                QJsonObject output;
                output["type"] = "invokeResult";
                output["sourceInvoke"] = invoke.invokeId;
                output["sourceNodeId"] = node->getId();
                output["sourcePortIndex"] = edge->sourcePortIndex();
                output["sourcePortType"] = edge->sourcePortType();
                context.outputs[outputId] = output;
                
            }
        }
    }
    
    // Find next nodes to invoke
    QList<Edge*> outgoingEdges = getOutgoingFlowEdges(node, scripts);
    QStringList nextInvokeIds;
    
    // First pass: collect all next nodes and pre-generate their invoke IDs
    QList<QPair<Node*, QString>> nextNodesWithIds;
    for (Edge* edge : outgoingEdges) {
        Node* nextNode = scripts->getNode(edge->targetNodeId());
        if (nextNode) {
            // Check if we already have an invoke for this node
            QString existingInvokeId;
            for (auto it = context.invokes.begin(); it != context.invokes.end(); ++it) {
                if (it.value().nodeId == nextNode->getId()) {
                    existingInvokeId = it.key();
                    break;
                }
            }
            
            if (!existingInvokeId.isEmpty()) {
                nextInvokeIds.append(existingInvokeId);
            } else {
                QString nextInvokeId = QString("invoke_%1").arg(context.invokeCounter);
                nextInvokeIds.append(nextInvokeId);
                nextNodesWithIds.append(qMakePair(nextNode, nextInvokeId));
            }
        }
    }
    
    // Update the current invoke with the next invoke IDs
    if (!nextInvokeIds.isEmpty()) {
        context.invokes[invoke.invokeId].nextInvokes = nextInvokeIds;
    }
    
    // Second pass: process the next nodes with their pre-assigned IDs
    for (const auto& pair : nextNodesWithIds) {
        Node* nextNode = pair.first;
        QString nextId = pair.second;
        // Increment counter for the pre-assigned ID
        context.invokeCounter++;
        buildInvokesRecursiveWithId(nextNode, scripts, context, elementModel, invoke.invokeId, nextId);
    }
}

QJsonArray ScriptInvokeBuilder::createNodeParameters(Node* node, Scripts* scripts, BuildContext& context)
{
    QJsonArray params;
    
    // Check if node has a direct value
    QString nodeValue = node->value();
    
    // Look for incoming data edges
    QList<Edge*> dataEdges = getIncomingDataEdges(node, scripts);
    
    if (!nodeValue.isEmpty()) {
        // Node has its own value - create a literal output
        QString outputId = generateOutputId(context);
        QJsonObject output;
        output["type"] = "literal";
        output["value"] = nodeValue;
        context.outputs[outputId] = output;
        
        // Reference this output in params
        QJsonObject param;
        param["output"] = outputId;
        params.append(param);
        
    } else if (!dataEdges.isEmpty()) {
        // Node gets data from connected edges
        for (Edge* edge : dataEdges) {
            Node* sourceNode = scripts->getNode(edge->sourceNodeId());
            if (sourceNode) {
                // Check if the source node is an event node
                if (sourceNode->nodeType() == "Event") {
                    // For event nodes, create an output that references event data
                    QString outputId = generateOutputId(context);
                    QJsonObject output;
                    output["type"] = "eventData";
                    output["sourceNodeId"] = sourceNode->getId();
                    output["sourcePortIndex"] = edge->sourcePortIndex();
                    context.outputs[outputId] = output;
                    
                    // Reference this output in params
                    QJsonObject param;
                    param["output"] = outputId;
                    params.append(param);
                } else {
                    // For regular nodes, check if we already have an output for this source
                    // This happens for async nodes where we pre-created outputs
                    QString existingOutputId;
                    for (auto it = context.outputs.begin(); it != context.outputs.end(); ++it) {
                        QJsonObject output = it.value();
                        if (output["sourceNodeId"].toString() == sourceNode->getId() &&
                            output["sourcePortIndex"].toInt() == edge->sourcePortIndex() &&
                            output["type"].toString() == "invokeResult") {
                            existingOutputId = it.key();
                            break;
                        }
                    }
                    
                    if (!existingOutputId.isEmpty()) {
                        // Use the existing output created for async nodes
                        QJsonObject param;
                        param["output"] = existingOutputId;
                        params.append(param);
                    } else {
                        // For non-async nodes, use the static value
                        QString sourceValue = sourceNode->value();
                        
                        if (!sourceValue.isEmpty()) {
                            // Create output for source node's value
                            QString outputId = generateOutputId(context);
                            QJsonObject output;
                            output["type"] = "literal";
                            output["value"] = sourceValue;
                            output["sourceNodeId"] = sourceNode->getId();
                            context.outputs[outputId] = output;
                            
                            // Reference this output in params
                            QJsonObject param;
                            param["output"] = outputId;
                            params.append(param);
                        }
                    }
                }
            }
        }
    }
    
    // If no params were added, add an empty value
    if (params.isEmpty()) {
        QJsonObject param;
        param["value"] = "";
        params.append(param);
    }
    
    return params;
}

QString ScriptInvokeBuilder::getFunctionNameForNode(Node* node) const
{
    QString title = node->nodeTitle().toLower().remove(' ');
    
    // Map common node titles to function names
    if (title == "consolelog") {
        return "consoleLog";
    }
    
    // Default: use the node title as function name
    return title;
}

QList<Edge*> ScriptInvokeBuilder::getIncomingDataEdges(Node* node, Scripts* scripts)
{
    QList<Edge*> result;
    QList<Edge*> edges = scripts->getIncomingEdges(node->getId());
    
    for (Edge* edge : edges) {
        // Check for non-Flow edges (Data, String, Number, Boolean, etc.)
        if (edge->targetPortType() != "Flow") {
            result.append(edge);
        }
    }
    
    return result;
}

QList<Edge*> ScriptInvokeBuilder::getOutgoingFlowEdges(Node* node, Scripts* scripts)
{
    QList<Edge*> result;
    QList<Edge*> edges = scripts->getOutgoingEdges(node->getId());
    
    for (Edge* edge : edges) {
        if (edge->sourcePortType() == "Flow") {
            result.append(edge);
        }
    }
    
    return result;
}

QString ScriptInvokeBuilder::generateInvokeId(BuildContext& context)
{
    return QString("invoke_%1").arg(context.invokeCounter++);
}

QString ScriptInvokeBuilder::generateOutputId(BuildContext& context)
{
    return QString("output_%1").arg(context.outputCounter++);
}

void ScriptInvokeBuilder::buildComponentInvokesRecursive(Node* node, Scripts* scripts, 
                                                         BuildContext& context, ElementModel* elementModel)
{
    if (!node || !scripts) {
        return;
    }
    
    // Skip event nodes - they don't generate invokes
    if (node->nodeType() == "Event") {
        return;
    }
    
    // Store node reference for function generation
    NodeReference nodeRef;
    nodeRef.node = node;
    nodeRef.scripts = scripts;
    context.nodeReferences[node->getId()] = nodeRef;
    
    // Create invoke for this node
    InvokeData invoke;
    invoke.invokeId = generateInvokeId(context);
    invoke.nodeId = node->getId();
    invoke.functionName = getFunctionNameForNode(node);
    invoke.params = createNodeParameters(node, scripts, context);
    invoke.isAsync = node->isAsync();
    
    // Find next nodes to invoke
    QList<Edge*> outgoingEdges = getOutgoingFlowEdges(node, scripts);
    for (Edge* edge : outgoingEdges) {
        Node* nextNode = scripts->getNode(edge->targetNodeId());
        if (nextNode) {
            QString nextInvokeId = generateInvokeId(context);
            invoke.nextInvokes.append(nextInvokeId);
            
            // Store this invoke before processing next
            context.invokes[invoke.invokeId] = invoke;
            
            // Process next node
            buildComponentInvokesRecursive(nextNode, scripts, context, elementModel);
            return; // Already stored invoke
        }
    }
    
    // Store invoke if not already stored
    context.invokes[invoke.invokeId] = invoke;
}