#include "ScriptSerializer.h"
#include "ScriptFunctionRegistry.h"
#include "Scripts.h"
#include "Node.h"
#include <QSet>

ScriptSerializer::ScriptSerializer()
    : m_functionRegistry(nullptr)
{
}

void ScriptSerializer::setFunctionRegistry(ScriptFunctionRegistry* registry)
{
    m_functionRegistry = registry;
}

QJsonDocument ScriptSerializer::serialize(const QMap<QString, ScriptInvokeBuilder::BuildContext>& eventContexts,
                                         Scripts* scripts)
{
    QJsonObject root;
    
    // Serialize each event
    for (auto it = eventContexts.constBegin(); it != eventContexts.constEnd(); ++it) {
        const QString& eventName = it.key();
        const ScriptInvokeBuilder::BuildContext& context = it.value();
        
        // Get the initial invokes for this event
        QList<QString> initialInvokes;
        for (const auto& invoke : context.invokes) {
            bool isInitial = true;
            // Check if this invoke is referenced by any other invoke
            for (const auto& other : context.invokes) {
                if (other.invokeId != invoke.invokeId && 
                    other.nextInvokes.contains(invoke.invokeId)) {
                    isInitial = false;
                    break;
                }
            }
            if (isInitial) {
                initialInvokes.append(invoke.invokeId);
            }
        }
        
        root[eventName] = serializeEventContext(context, initialInvokes, scripts);
    }
    
    return QJsonDocument(root);
}

QJsonObject ScriptSerializer::serializeEventContext(const ScriptInvokeBuilder::BuildContext& context,
                                                    const QList<QString>& initialInvokes,
                                                    Scripts* scripts)
{
    QJsonObject event;
    
    // Functions
    event["functions"] = serializeFunctions(context, scripts);
    
    // Outputs
    event["outputs"] = serializeOutputs(context);
    
    // Invokes
    event["invoke"] = serializeInvokes(context);
    
    // Next array (initial invokes)
    QJsonArray nextArray;
    for (const QString& invokeId : initialInvokes) {
        nextArray.append(invokeId);
    }
    event["next"] = nextArray;
    
    return event;
}

QJsonObject ScriptSerializer::serializeInvokes(const ScriptInvokeBuilder::BuildContext& context)
{
    QJsonObject invokes;
    
    for (auto it = context.invokes.constBegin(); it != context.invokes.constEnd(); ++it) {
        const QString& invokeId = it.key();
        const ScriptInvokeBuilder::InvokeData& invokeData = it.value();
        
        QJsonObject invoke;
        invoke["nodeId"] = invokeData.nodeId;
        invoke["function"] = invokeData.functionName;
        invoke["params"] = invokeData.params;
        
        if (!invokeData.nextInvokes.isEmpty()) {
            QJsonArray nextArray;
            for (const QString& nextId : invokeData.nextInvokes) {
                nextArray.append(nextId);
            }
            invoke["next"] = nextArray;
        }
        
        invokes[invokeId] = invoke;
    }
    
    return invokes;
}

QJsonObject ScriptSerializer::serializeOutputs(const ScriptInvokeBuilder::BuildContext& context)
{
    QJsonObject outputs;
    for (auto it = context.outputs.constBegin(); it != context.outputs.constEnd(); ++it) {
        outputs[it.key()] = it.value();
    }
    return outputs;
}

QJsonObject ScriptSerializer::serializeFunctions(const ScriptInvokeBuilder::BuildContext& context,
                                                Scripts* /*scripts*/)
{
    QJsonObject functions;
    
    if (!m_functionRegistry) {
        return functions;
    }
    
    // Get unique function names
    QSet<QString> functionNames = getUniqueFunctionNames(context);
    
    // Generate function code for each unique function
    for (const QString& functionName : functionNames) {
        // Find a node that uses this function to get its type
        Node* sampleNode = nullptr;
        for (const auto& invoke : context.invokes) {
            if (invoke.functionName == functionName) {
                // Use nodeReferences to find the node
                auto nodeRefIt = context.nodeReferences.find(invoke.nodeId);
                if (nodeRefIt != context.nodeReferences.end()) {
                    sampleNode = nodeRefIt->node;
                    break;
                }
            }
        }
        
        if (sampleNode) {
            QString functionCode = m_functionRegistry->getFunctionCode(sampleNode);
            functions[functionName] = functionCode;
        }
    }
    
    return functions;
}

QSet<QString> ScriptSerializer::getUniqueFunctionNames(const ScriptInvokeBuilder::BuildContext& context)
{
    QSet<QString> functionNames;
    
    for (const auto& invoke : context.invokes) {
        functionNames.insert(invoke.functionName);
    }
    
    return functionNames;
}