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
    
    //          << "has" << outgoingEdges.size() << "outgoing Flow edges";
    
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
    
    // Skip param nodes - they only provide data, don't execute
    if (node->nodeType() == "Param") {
        // Still need to store node reference for data access
        NodeReference nodeRef;
        nodeRef.node = node;
        nodeRef.scripts = scripts;
        context.nodeReferences[node->getId()] = nodeRef;
        
        // Pre-create outputs for param nodes so their data is available
        // First, we need to build parameters for this param node
        QJsonArray paramNodeParams = createNodeParameters(node, scripts, context);
        
        // Store the param data in the context
        QString paramInvokeId = QString("param_%1").arg(context.invokeCounter++);
        InvokeData paramInvoke;
        paramInvoke.invokeId = paramInvokeId;
        paramInvoke.nodeId = node->getId();
        paramInvoke.functionName = getFunctionNameForNode(node);
        paramInvoke.params = paramNodeParams;
        paramInvoke.isAsync = false;
        paramInvoke.isParam = true;  // Mark as param
        context.invokes[paramInvokeId] = paramInvoke;
        
        // Create outputs for all data ports
        QList<Edge*> allOutgoingEdges = scripts->getOutgoingEdges(node->getId());
        for (Edge* edge : allOutgoingEdges) {
            if (edge->sourcePortType() != "Flow") {
                QString outputId = generateOutputId(context);
                QJsonObject output;
                output["type"] = "paramResult";
                output["sourceInvoke"] = paramInvokeId;
                output["sourceNodeId"] = node->getId();
                output["sourcePortIndex"] = edge->sourcePortIndex();
                output["sourcePortType"] = edge->sourcePortType();
                context.outputs[outputId] = output;
            }
        }
        return;
    }
    
    // Store node reference for function generation
    NodeReference nodeRef;
    nodeRef.node = node;
    nodeRef.scripts = scripts;
    context.nodeReferences[node->getId()] = nodeRef;
    
    // Special handling for ForEachLoop node
    if (node->nodeTitle() == "For Each Loop") {
        
        // Create a special invoke for the loop
        InvokeData invoke;
        invoke.invokeId = generateInvokeId(context);
        invoke.nodeId = node->getId();
        invoke.functionName = getFunctionNameForNode(node);
        invoke.params = createNodeParameters(node, scripts, context);
        invoke.isAsync = false;
        invoke.isLoop = true;  // Mark this as a loop invoke
        
        // Create outputs for the loop's data outputs (Array Element and Array Index) BEFORE processing loop body
        // These need to exist before we process the loop body so param nodes can reference them
        
        // Array Element output (port index 1)
        QString elementOutputId = generateOutputId(context);
        QJsonObject elementOutput;
        elementOutput["type"] = "invokeResult";
        elementOutput["sourceInvoke"] = invoke.invokeId;
        elementOutput["sourceNodeId"] = node->getId();
        elementOutput["sourcePortIndex"] = 1;  // arrayElement port
        elementOutput["sourcePortType"] = "Number";
        context.outputs[elementOutputId] = elementOutput;
        
        // Array Index output (port index 2)
        QString indexOutputId = generateOutputId(context);
        QJsonObject indexOutput;
        indexOutput["type"] = "invokeResult";
        indexOutput["sourceInvoke"] = invoke.invokeId;
        indexOutput["sourceNodeId"] = node->getId();
        indexOutput["sourcePortIndex"] = 2;  // arrayIndex port
        indexOutput["sourcePortType"] = "Number";
        context.outputs[indexOutputId] = indexOutput;
        
        // Store the loop invoke
        context.invokes[invoke.invokeId] = invoke;
        
        // Find the edges connected to the "Item" and "On Complete" flow outputs
        QList<Edge*> allOutgoingEdges = scripts->getOutgoingEdges(node->getId());
        QString itemInvokeId;
        QString onCompleteInvokeId;
        
        for (Edge* edge : allOutgoingEdges) {
            if (edge->sourcePortType() == "Flow") {
                Node* targetNode = scripts->getNode(edge->targetNodeId());
                if (targetNode) {
                    // Check which source port this edge is connected to
                    int sourcePortIndex = edge->sourcePortIndex();
                    
                    // Source port 0 is "Item", port 1 is "On Complete"
                    if (sourcePortIndex == 0) {
                        // This is the "Item" flow - build invokes for the loop body
                        itemInvokeId = generateInvokeId(context);
                        context.invokeCounter++;
                        buildInvokesRecursiveWithId(targetNode, scripts, context, elementModel, invoke.invokeId, itemInvokeId);
                    } else if (sourcePortIndex == 1) {
                        // This is the "On Complete" flow
                        onCompleteInvokeId = generateInvokeId(context);
                        context.invokeCounter++;
                        buildInvokesRecursiveWithId(targetNode, scripts, context, elementModel, invoke.invokeId, onCompleteInvokeId);
                    }
                }
            }
        }
        
        // Store loop-specific information in the invoke
        context.invokes[invoke.invokeId].loopBodyInvoke = itemInvokeId;
        context.invokes[invoke.invokeId].loopCompleteInvoke = onCompleteInvokeId;
        
        return; // Don't continue with normal flow processing
    }
    
    // Special handling for ComponentOnEditorLoadEvents node
    if (node->nodeTitle() == "Component On Editor Load Events") {
        
        // Get the parent element that owns these scripts
        QObject* scriptParent = scripts->parent();
        FrameComponentInstanceTemplate* frameInstance = qobject_cast<FrameComponentInstanceTemplate*>(scriptParent);
        TextComponentInstanceTemplate* textInstance = qobject_cast<TextComponentInstanceTemplate*>(scriptParent);
        
        Component* component = nullptr;
        QString componentId;
        
        // Check if the scripts belong to a component instance
        if (frameInstance) {
            componentId = frameInstance->componentId();
            //          << "instance of" << componentId;
            
            // Access the component directly from the instance
            component = frameInstance->sourceComponent();
        } else if (textInstance) {
            componentId = textInstance->componentId();
            //          << "instance of" << componentId;
            
            // Access the component directly from the instance
            component = textInstance->sourceComponent();
        } else {
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
            //          << "for instance" << (frameInstance ? frameInstance->getId() : textInstance->getId());
            
            // Ensure component scripts are compiled
            Scripts* componentScripts = component->scripts();
            //          << componentScripts->edgeCount() << "edges";
            if (!componentScripts->isCompiled()) {
                ScriptCompiler compiler;
                QString compiledJson = compiler.compile(componentScripts, elementModel);
                if (compiledJson.isEmpty()) {
                    qWarning() << "ScriptInvokeBuilder: Failed to compile component scripts for" << componentId;
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
                }
            }
        } else {
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
    invoke.isAsync = node->isAsync();
    
    // Pre-create outputs for nodes that have output ports BEFORE creating parameters
    // This ensures downstream nodes can reference these outputs
    QList<Edge*> allOutgoingEdges = scripts->getOutgoingEdges(node->getId());
    for (Edge* edge : allOutgoingEdges) {
        if (edge->sourcePortType() != "Flow") {
            // This node has a data output that's connected to another node
            // Create an output that will be populated by the result
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
    
    // NOW create parameters after outputs exist
    invoke.params = createNodeParameters(node, scripts, context);
    
    // Store this invoke
    context.invokes[invoke.invokeId] = invoke;
    
    // Create output for "Set Variable Value" nodes
    QString nodeType = node->nodeTitle().toLower().remove(' ');
    if (nodeType.startsWith("set") && nodeType.endsWith("value") && !node->sourceElementId().isEmpty()) {
        // This is a variable setter node - create a variable output
        QString outputId = generateOutputId(context);
        QJsonObject output;
        output["type"] = "variable";
        output["sourceInvoke"] = invoke.invokeId;
        output["targetId"] = node->sourceElementId(); // The variable to update
        context.outputs[outputId] = output;
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
    Q_UNUSED(parentInvokeId)
    if (!node || !scripts) {
        return;
    }
    
    // Skip event nodes - they don't generate invokes
    if (node->nodeType() == "Event") {
        return;
    }
    
    // Skip param nodes - they only provide data, don't execute
    if (node->nodeType() == "Param") {
        // Still need to store node reference for data access
        NodeReference nodeRef;
        nodeRef.node = node;
        nodeRef.scripts = scripts;
        context.nodeReferences[node->getId()] = nodeRef;
        
        // Pre-create outputs for param nodes so their data is available
        // First, we need to build parameters for this param node
        QJsonArray paramNodeParams = createNodeParameters(node, scripts, context);
        
        // Store the param data in the context
        QString paramInvokeId = QString("param_%1").arg(context.invokeCounter++);
        InvokeData paramInvoke;
        paramInvoke.invokeId = paramInvokeId;
        paramInvoke.nodeId = node->getId();
        paramInvoke.functionName = getFunctionNameForNode(node);
        paramInvoke.params = paramNodeParams;
        paramInvoke.isAsync = false;
        paramInvoke.isParam = true;  // Mark as param
        context.invokes[paramInvokeId] = paramInvoke;
        
        // Create outputs for all data ports
        QList<Edge*> allOutgoingEdges = scripts->getOutgoingEdges(node->getId());
        for (Edge* edge : allOutgoingEdges) {
            if (edge->sourcePortType() != "Flow") {
                QString outputId = generateOutputId(context);
                QJsonObject output;
                output["type"] = "paramResult";
                output["sourceInvoke"] = paramInvokeId;
                output["sourceNodeId"] = node->getId();
                output["sourcePortIndex"] = edge->sourcePortIndex();
                output["sourcePortType"] = edge->sourcePortType();
                context.outputs[outputId] = output;
            }
        }
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
    invoke.isAsync = node->isAsync();
    
    // Pre-create outputs for nodes that have output ports BEFORE creating parameters
    // This ensures downstream nodes can reference these outputs
    QList<Edge*> allOutgoingEdges = scripts->getOutgoingEdges(node->getId());
    for (Edge* edge : allOutgoingEdges) {
        if (edge->sourcePortType() != "Flow") {
            // This node has a data output that's connected to another node
            // Create an output that will be populated by the result
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
    
    // NOW create parameters after outputs exist
    invoke.params = createNodeParameters(node, scripts, context);
    
    // Store this invoke
    context.invokes[invoke.invokeId] = invoke;
    
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
    
    
    // Check if this is a dynamic node with multiple values
    if (node->isDynamicNode()) {
        // For dynamic nodes, create a parameter object with all port values
        QJsonObject paramObj;
        
        // Get all row configurations to find all value ports
        QVariantList rowConfigs = node->rowConfigurations();
        
        // For dynamic nodes, create an array of parameters in the "new format"
        // This will make params = [{value: "998"}, {value: "454"}] in JavaScript
        for (const QVariant& rowVar : rowConfigs) {
            QVariantMap row = rowVar.toMap();
            if (row["hasTarget"].toBool() && row["targetType"].toString() != "Flow") {
                int portIndex = row["targetPortIndex"].toInt();
                QString portValue = node->getPortValue(portIndex);
                
                // If no specific value, use default
                if (portValue.isEmpty()) {
                    portValue = "0";  // Default for number inputs
                } else {
                }
                
                // Create a separate parameter for each port value
                QJsonObject param;
                param["value"] = portValue;
                params.append(param);
            }
        }
        
    } else {
        // Original logic for non-dynamic nodes
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
                //          << "to" << node->nodeTitle() << "port" << edge->targetPortIndex();
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
                        //          << "port:" << output["sourcePortIndex"].toInt() << "type:" << output["type"].toString();
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
                        // Check if this is a Param node that needs to be evaluated
                        if (sourceNode->nodeType() == "Param") {
                            // Create an invoke for this Param node if not already created
                            QString paramInvokeId;
                            for (auto it = context.invokes.begin(); it != context.invokes.end(); ++it) {
                                if (it.value().nodeId == sourceNode->getId()) {
                                    paramInvokeId = it.key();
                                    break;
                                }
                            }
                            
                            if (paramInvokeId.isEmpty()) {
                                // Create a new invoke for this Param node
                                paramInvokeId = generateInvokeId(context);
                                
                                // Store node reference
                                NodeReference nodeRef;
                                nodeRef.node = sourceNode;
                                nodeRef.scripts = scripts;
                                context.nodeReferences[sourceNode->getId()] = nodeRef;
                                
                                // Create invoke
                                InvokeData invoke;
                                invoke.invokeId = paramInvokeId;
                                invoke.nodeId = sourceNode->getId();
                                invoke.functionName = getFunctionNameForNode(sourceNode);
                                invoke.params = createNodeParameters(sourceNode, scripts, context);
                                invoke.isAsync = false;
                                invoke.isParam = true;  // Mark as param since this is a Param node
                                
                                context.invokes[paramInvokeId] = invoke;
                            }
                            
                            // Create an output for this Param node's result
                            QString outputId = generateOutputId(context);
                            QJsonObject output;
                            output["type"] = "paramResult";  // Use paramResult to trigger param evaluation
                            output["sourceInvoke"] = paramInvokeId;
                            output["sourceNodeId"] = sourceNode->getId();
                            output["sourcePortIndex"] = edge->sourcePortIndex();
                            output["sourcePortType"] = edge->sourcePortType();
                            context.outputs[outputId] = output;
                            
                            // Reference this output in params
                            QJsonObject param;
                            param["output"] = outputId;
                            params.append(param);
                        } else {
                            // For non-async, non-param nodes, use the static value
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
    }
    }  // End of else block for non-dynamic nodes
    
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
    
    // Skip param nodes - they only provide data, don't execute
    if (node->nodeType() == "Param") {
        // Still need to store node reference for data access
        NodeReference nodeRef;
        nodeRef.node = node;
        nodeRef.scripts = scripts;
        context.nodeReferences[node->getId()] = nodeRef;
        
        // Pre-create outputs for param nodes so their data is available
        // First, we need to build parameters for this param node
        QJsonArray paramNodeParams = createNodeParameters(node, scripts, context);
        
        // Store the param data in the context
        QString paramInvokeId = QString("param_%1").arg(context.invokeCounter++);
        InvokeData paramInvoke;
        paramInvoke.invokeId = paramInvokeId;
        paramInvoke.nodeId = node->getId();
        paramInvoke.functionName = getFunctionNameForNode(node);
        paramInvoke.params = paramNodeParams;
        paramInvoke.isAsync = false;
        paramInvoke.isParam = true;  // Mark as param
        context.invokes[paramInvokeId] = paramInvoke;
        
        // Create outputs for all data ports
        QList<Edge*> allOutgoingEdges = scripts->getOutgoingEdges(node->getId());
        for (Edge* edge : allOutgoingEdges) {
            if (edge->sourcePortType() != "Flow") {
                QString outputId = generateOutputId(context);
                QJsonObject output;
                output["type"] = "paramResult";
                output["sourceInvoke"] = paramInvokeId;
                output["sourceNodeId"] = node->getId();
                output["sourcePortIndex"] = edge->sourcePortIndex();
                output["sourcePortType"] = edge->sourcePortType();
                context.outputs[outputId] = output;
            }
        }
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
    invoke.isAsync = node->isAsync();
    
    // Pre-create outputs for nodes that have output ports BEFORE creating parameters
    QList<Edge*> allOutgoingEdges = scripts->getOutgoingEdges(node->getId());
    for (Edge* edge : allOutgoingEdges) {
        if (edge->sourcePortType() != "Flow") {
            // This node has a data output that's connected to another node
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
    
    // NOW create parameters after outputs exist
    invoke.params = createNodeParameters(node, scripts, context);
    
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