#include "ScriptInvokeBuilder.h"
#include "Scripts.h"
#include "Node.h"
#include "Edge.h"
#include <QDebug>

ScriptInvokeBuilder::ScriptInvokeBuilder()
{
}

ScriptInvokeBuilder::BuildContext ScriptInvokeBuilder::buildInvokes(Node* eventNode, Scripts* scripts)
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
            buildInvokesRecursive(targetNode, scripts, context);
        }
    }
    
    return context;
}

void ScriptInvokeBuilder::buildInvokesRecursive(Node* node, Scripts* scripts, 
                                                BuildContext& context, const QString& parentInvokeId)
{
    Q_UNUSED(parentInvokeId);
    if (!node || !scripts) {
        return;
    }
    
    // Skip event nodes - they don't generate invokes
    if (node->nodeType() == "Event") {
        return;
    }
    
    // Create invoke for this node
    InvokeData invoke;
    invoke.invokeId = generateInvokeId(context);
    invoke.nodeId = node->getId();
    invoke.functionName = getFunctionNameForNode(node);
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
            buildInvokesRecursive(nextNode, scripts, context, invoke.invokeId);
            return; // Already stored invoke
        }
    }
    
    // Store invoke if not already stored
    context.invokes[invoke.invokeId] = invoke;
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
            if (sourceNode && !sourceNode->value().isEmpty()) {
                // Create output for source node's value
                QString outputId = generateOutputId(context);
                QJsonObject output;
                output["type"] = "literal";
                output["value"] = sourceNode->value();
                output["sourceNodeId"] = sourceNode->getId();
                context.outputs[outputId] = output;
                
                // Reference this output in params
                QJsonObject param;
                param["output"] = outputId;
                params.append(param);
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
        if (edge->targetPortType() == "Data") {
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