#include "ScriptCompiler.h"
#include "Scripts.h"
#include "Node.h"
#include "Edge.h"
#include <QJsonDocument>
#include <QDebug>

ScriptCompiler::ScriptCompiler(QObject *parent)
    : QObject(parent)
{
}

ScriptCompiler::~ScriptCompiler() = default;

QString ScriptCompiler::compile(Scripts* scripts) {
    if (!scripts) {
        m_lastError = "Scripts object is null";
        return QString();
    }
    
    m_lastError.clear();
    m_lastCompiled = QJsonObject();
    
    // Validate the graph structure
    if (!validateGraph(scripts)) {
        return QString();
    }
    
    CompilationContext context;
    
    // Find all event nodes (starting points)
    QList<Node*> eventNodes;
    for (Node* node : scripts->getAllNodes()) {
        if (node->nodeType() == "Event") {
            eventNodes.append(node);
        }
    }
    
    if (eventNodes.isEmpty()) {
        m_lastError = "No event nodes found in the graph";
        return QString();
    }
    
    // Build the compiled structure
    QJsonObject result;
    
    // Process each event node and its connected graph
    for (Node* eventNode : eventNodes) {
        QString eventName = eventNode->nodeTitle().toLower().remove(" ");
        
        // Reset context for each event
        context = CompilationContext();
        
        // Process the event node and all connected nodes
        processNode(eventNode, context, scripts);
        
        // Build function definitions
        buildFunctionDefinitions(context);
        
        // Convert node IDs to invoke IDs in all next arrays
        convertNodeIdsToInvokeIds(context);
        
        // Get the nodes directly connected to this event node and convert to invoke IDs
        QStringList nextNodeIds = getNextNodeIds(eventNode, scripts);
        QJsonArray nextArray;
        for (const QString& nodeId : nextNodeIds) {
            if (context.nodeToInvokeMap.contains(nodeId)) {
                nextArray.append(context.nodeToInvokeMap[nodeId]);
            }
        }
        
        // Create the event object
        QJsonObject eventObj;
        eventObj["functions"] = context.functions;
        eventObj["outputs"] = context.outputs;
        eventObj["invoke"] = context.invokes;
        eventObj["next"] = nextArray;
        
        result[eventName] = eventObj;
    }
    
    m_lastCompiled = result;
    
    // Convert to formatted JSON string
    QJsonDocument doc(result);
    return doc.toJson(QJsonDocument::Indented);
}

void ScriptCompiler::processNode(Node* node, CompilationContext& context, Scripts* scripts) {
    if (!node || context.processedNodes.contains(node->getId())) {
        return;
    }
    
    context.processedNodes.insert(node->getId());
    
    // Skip event nodes in invoke generation (they're just triggers)
    if (node->nodeType() != "Event") {
        QJsonObject invokeObj = createInvokeObject(node, context, scripts);
        if (!invokeObj.isEmpty()) {
            QString invokeId = "invoke_" + QString::number(context.invokeCounter++);
            context.invokes[invokeId] = invokeObj;
            context.nodeToInvokeMap[node->getId()] = invokeId;
        }
    }
    
    // Process all nodes connected to this node's outputs
    QStringList nextNodeIds = getNextNodeIds(node, scripts);
    for (const QString& nextId : nextNodeIds) {
        Node* nextNode = scripts->getNode(nextId);
        if (nextNode) {
            processNode(nextNode, context, scripts);
        }
    }
}

QJsonObject ScriptCompiler::createInvokeObject(Node* node, CompilationContext& context, Scripts* scripts) {
    QJsonObject invoke;
    
    invoke["nodeId"] = node->getId();
    
    // Determine the function type based on node name
    QString nodeName = node->nodeTitle();
    QString functionType;
    
    if (nodeName.contains("Console", Qt::CaseInsensitive)) {
        functionType = "consoleLog";
    } else if (nodeName.contains("Display", Qt::CaseInsensitive)) {
        functionType = "displayOutput";
    } else if (nodeName.contains("HTTP", Qt::CaseInsensitive)) {
        functionType = "httpRequest";
    } else if (nodeName == "Add" || nodeName == "Subtract" || 
               nodeName == "Multiply" || nodeName == "Divide") {
        functionType = "math_" + nodeName.toLower();
    } else if (nodeName.contains("Variable", Qt::CaseInsensitive)) {
        if (nodeName.contains("Set")) {
            functionType = "setVariable";
        } else {
            functionType = "getVariable";
        }
    } else {
        functionType = nodeName.toLower().remove(" ");
    }
    
    // Get or create function reference
    QString functionId = getOrCreateFunction(functionType, context);
    invoke["function"] = functionId;
    
    // Handle parameters and create outputs for input values
    QJsonArray params;
    
    // Get incoming edges to determine parameters
    QList<Edge*> incomingEdges = scripts->getIncomingEdges(node->getId());
    
    // Count non-flow incoming edges
    int dataEdgeCount = 0;
    for (Edge* edge : incomingEdges) {
        if (edge->sourcePortType() != "Flow") {
            dataEdgeCount++;
        }
    }
    
    // Check if node has a value property (for nodes with input fields)
    if (!node->value().isEmpty()) {
        // Create an output for the node's value
        QString outputKey = node->getId() + "_value";
        if (!context.outputMap.contains(outputKey)) {
            QString outputId = "output_" + QString::number(context.outputCounter++);
            context.outputMap[outputKey] = outputId;
            
            QJsonObject outputObj;
            outputObj["nodeId"] = node->getId();
            outputObj["type"] = "literal";
            outputObj["value"] = node->value();
            context.outputs[outputId] = outputObj;
        }
        
        // If this is a node that uses its value as input (like console log)
        // and has no data edges coming in
        if (functionType == "consoleLog" && dataEdgeCount == 0) {
            params.append(QJsonObject{{"output", context.outputMap[outputKey]}});
        }
    }
    
    // Process incoming edges
    for (Edge* edge : incomingEdges) {
        // Skip flow edges
        if (edge->sourcePortType() == "Flow") {
            continue;
        }
        
        // Check if the source is an output reference
        QString sourceNodeId = edge->sourceNodeId();
        QString outputKey = sourceNodeId + "_output";
        
        if (context.outputMap.contains(outputKey)) {
            params.append(QJsonObject{{"output", context.outputMap[outputKey]}});
        } else {
            // Check if source node has a value
            Node* sourceNode = scripts->getNode(sourceNodeId);
            if (sourceNode && !sourceNode->value().isEmpty()) {
                QString valueKey = sourceNodeId + "_value";
                if (!context.outputMap.contains(valueKey)) {
                    QString outputId = "output_" + QString::number(context.outputCounter++);
                    context.outputMap[valueKey] = outputId;
                    
                    QJsonObject outputObj;
                    outputObj["nodeId"] = sourceNodeId;
                    outputObj["type"] = "literal";
                    outputObj["value"] = sourceNode->value();
                    context.outputs[outputId] = outputObj;
                }
                params.append(QJsonObject{{"output", context.outputMap[valueKey]}});
            } else {
                // Default placeholder
                params.append(QJsonObject{{"value", "placeholder"}});
            }
        }
    }
    
    // If no parameters but node expects them, add a default
    if (params.isEmpty() && functionType == "consoleLog" && node->value().isEmpty() && dataEdgeCount == 0) {
        params.append(QJsonObject{{"value", "Hello from compiled script!"}});
    }
    
    invoke["params"] = params;
    
    // Handle outputs
    if (node->nodeType() == "Operation" && 
        (functionType.startsWith("math_") || functionType.contains("variable"))) {
        QString outputKey = node->getId() + "_output";
        QString outputId = "output_" + QString::number(context.outputCounter++);
        context.outputMap[outputKey] = outputId;
        invoke["output"] = outputId;
        
        // Add to outputs object
        QJsonObject outputObj;
        outputObj["nodeId"] = node->getId();
        outputObj["type"] = "value";
        context.outputs[outputId] = outputObj;
    }
    
    // Get next nodes and convert to invoke IDs
    QStringList nextNodeIds = getNextNodeIds(node, scripts);
    if (!nextNodeIds.isEmpty()) {
        QJsonArray nextArray;
        for (const QString& nodeId : nextNodeIds) {
            // Note: The invoke for this node might not exist yet if we haven't processed it
            // We'll need to handle this in a second pass
            nextArray.append(nodeId);  // Temporarily store node IDs
        }
        invoke["next"] = nextArray;
    }
    
    return invoke;
}

QString ScriptCompiler::getOrCreateFunction(const QString& functionType, CompilationContext& context) {
    if (context.functionMap.contains(functionType)) {
        return context.functionMap[functionType];
    }
    
    // Create a unique function ID
    QString functionId = functionType + "_" + QString::number(context.functionCounter++);
    context.functionMap[functionType] = functionId;
    return functionId;
}

void ScriptCompiler::buildFunctionDefinitions(CompilationContext& context) {
    // Build actual function definitions based on what was used
    for (auto it = context.functionMap.begin(); it != context.functionMap.end(); ++it) {
        const QString& functionType = it.key();
        const QString& functionId = it.value();
        
        QString functionDef;
        
        if (functionType == "consoleLog") {
            functionDef = buildConsoleLogFunction();
        } else if (functionType.startsWith("math_")) {
            QString operation = functionType.mid(5); // Remove "math_" prefix
            functionDef = buildMathFunction(operation);
        } else if (functionType == "setVariable" || functionType == "getVariable") {
            functionDef = buildVariableFunction(functionType);
        } else if (functionType == "displayOutput") {
            functionDef = buildDisplayFunction();
        } else {
            // Generic function
            functionDef = "(params) => { /* " + functionType + " */ }";
        }
        
        // Add the function to the functions object
        context.functions[functionId] = functionDef;
    }
}

QStringList ScriptCompiler::getNextNodeIds(Node* node, Scripts* scripts) {
    QStringList nextIds;
    
    // Get all outgoing edges
    QList<Edge*> outgoingEdges = scripts->getOutgoingEdges(node->getId());
    
    // Filter for flow edges and get target node IDs
    for (Edge* edge : outgoingEdges) {
        if (edge->sourcePortType() == "Flow") {
            nextIds.append(edge->targetNodeId());
        }
    }
    
    return nextIds;
}

bool ScriptCompiler::validateGraph(Scripts* scripts) {
    // Basic validation
    if (scripts->getAllNodes().isEmpty()) {
        m_lastError = "No nodes in the graph";
        return false;
    }
    
    // Check for cycles (simplified check)
    // TODO: Implement proper cycle detection
    
    return true;
}

QString ScriptCompiler::buildConsoleLogFunction() {
    return "(params) => console.log(params[0].value)";
}

QString ScriptCompiler::buildMathFunction(const QString& operation) {
    if (operation == "add") {
        return "(params) => (params[0].value || 0) + (params[1].value || 0)";
    } else if (operation == "subtract") {
        return "(params) => (params[0].value || 0) - (params[1].value || 0)";
    } else if (operation == "multiply") {
        return "(params) => (params[0].value || 0) * (params[1].value || 0)";
    } else if (operation == "divide") {
        return "(params) => (params[1].value || 1) !== 0 ? (params[0].value || 0) / (params[1].value || 1) : 0";
    }
    return "(params) => 0";
}

QString ScriptCompiler::buildVariableFunction(const QString& operation) {
    if (operation == "setVariable") {
        return "(params, context) => { context.variables[params[0].name] = params[1].value; }";
    } else {
        return "(params, context) => context.variables[params[0].name]";
    }
}

QString ScriptCompiler::buildDisplayFunction() {
    return "(params) => updateDisplay(params[0].value || params[0].output)";
}

void ScriptCompiler::convertNodeIdsToInvokeIds(CompilationContext& context) {
    // Convert all node IDs in "next" arrays to invoke IDs
    for (auto it = context.invokes.begin(); it != context.invokes.end(); ++it) {
        QJsonObject invoke = it.value().toObject();
        if (invoke.contains("next")) {
            QJsonArray nextArray = invoke["next"].toArray();
            QJsonArray newNextArray;
            
            for (const QJsonValue& val : nextArray) {
                QString nodeId = val.toString();
                if (context.nodeToInvokeMap.contains(nodeId)) {
                    newNextArray.append(context.nodeToInvokeMap[nodeId]);
                }
            }
            
            invoke["next"] = newNextArray;
            context.invokes[it.key()] = invoke;
        }
    }
}

QJsonObject ScriptCompiler::getCompiledJson() const {
    return m_lastCompiled;
}

QString ScriptCompiler::getLastError() const {
    return m_lastError;
}