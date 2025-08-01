#include "ScriptFunctionRegistry.h"
#include "Node.h"

ScriptFunctionRegistry::ScriptFunctionRegistry()
{
    registerDefaultFunctions();
}

void ScriptFunctionRegistry::registerFunction(const QString& nodeType, FunctionBuilder builder)
{
    m_functionBuilders[nodeType] = builder;
}

QString ScriptFunctionRegistry::getFunctionCode(Node* node) const
{
    if (!node) {
        return QString();
    }
    
    // Use the registered function builder first for certain node types that we want to override
    QString nodeType = node->nodeTitle().toLower().remove(' ');
    
    // Priority override for specific nodes we want to use registry functions for
    if (nodeType == "consolelog" || nodeType == "convertnumbertostring" || nodeType == "createnumberarray") {
        auto it = m_functionBuilders.find(nodeType);
        if (it != m_functionBuilders.end()) {
            return it.value()(node);
        }
    }
    
    // For other nodes, check if the node has a script property
    QString nodeScript = node->script();
    if (!nodeScript.isEmpty()) {
        return nodeScript;
    }
    
    // Use the registered function builder as fallback
    
    auto it = m_functionBuilders.find(nodeType);
    if (it != m_functionBuilders.end()) {
        return it.value()(node);
    }
    
    // Check if this is a "Set Variable Value" node
    if (nodeType.startsWith("set") && nodeType.endsWith("value")) {
        // This is a variable setter node
        // The function will return an object with the variable ID and value
        QString variableId = node->sourceElementId();
        if (!variableId.isEmpty()) {
            return QString("(params) => { "
                          "return { "
                          "variableId: '%1', "
                          "value: params && params.length > 0 && params[0] ? params[0].value : '' "
                          "}; "
                          "}").arg(variableId);
        }
    }
    
    // Default: return an empty function
    return "(params) => { }";
}

bool ScriptFunctionRegistry::hasFunction(const QString& nodeType) const
{
    return m_functionBuilders.contains(nodeType.toLower().remove(' '));
}

void ScriptFunctionRegistry::registerDefaultFunctions()
{
    // Console Log
    registerFunction("consolelog", &ScriptFunctionRegistry::buildConsoleLogFunction);
    
    // Variable
    registerFunction("variable", &ScriptFunctionRegistry::buildVariableFunction);
    
    // Event Data
    registerFunction("eventdata", &ScriptFunctionRegistry::buildEventDataFunction);
    
    // Math operations (future)
    registerFunction("add", &ScriptFunctionRegistry::buildMathFunction);
    registerFunction("subtract", &ScriptFunctionRegistry::buildMathFunction);
    registerFunction("multiply", &ScriptFunctionRegistry::buildMathFunction);
    registerFunction("divide", &ScriptFunctionRegistry::buildMathFunction);
    
    // Conditions (future)
    registerFunction("condition", &ScriptFunctionRegistry::buildConditionFunction);
    registerFunction("if", &ScriptFunctionRegistry::buildConditionFunction);
    
    // AI Prompt
    registerFunction("aiprompt", &ScriptFunctionRegistry::buildAiPromptFunction);
    
    // Convert Number to String
    registerFunction("convertnumbertostring", &ScriptFunctionRegistry::buildConvertNumberToStringFunction);
    
    // Create Number Array
    registerFunction("createnumberarray", &ScriptFunctionRegistry::buildCreateNumberArrayFunction);
    
    // Set Variable Value - dynamic registration handled separately
    // We need to handle all "setvariable*value" patterns dynamically
}

QString ScriptFunctionRegistry::buildConsoleLogFunction(Node* node)
{
    Q_UNUSED(node);
    return "(params) => { "
           "if (params && params.length > 0 && params[0]) { "
           "let message = params[0].value || ''; "
           "if (typeof message === 'object' && message !== null) { "
           "if (message.string !== undefined) { "
           "message = message.string; "
           "} else if (message.value !== undefined) { "
           "message = message.value; "
           "} else { "
           "message = String(message); "
           "} "
           "} "
           "console.log(message); "
           "} else { "
           "console.log(''); "
           "} "
           "}";
}

QString ScriptFunctionRegistry::buildVariableFunction(Node* node)
{
    Q_UNUSED(node);
    // Variables just pass through their value
    return "(params) => { "
           "return params && params.length > 0 ? params[0].value : undefined; "
           "}";
}

QString ScriptFunctionRegistry::buildMathFunction(Node* node)
{
    if (!node) {
        return "(params) => { return 0; }";
    }
    
    QString operation = node->nodeTitle().toLower();
    
    if (operation == "add") {
        return "(params) => { "
               "const a = params[0] ? Number(params[0].value) : 0; "
               "const b = params[1] ? Number(params[1].value) : 0; "
               "return a + b; "
               "}";
    } else if (operation == "subtract") {
        return "(params) => { "
               "const a = params[0] ? Number(params[0].value) : 0; "
               "const b = params[1] ? Number(params[1].value) : 0; "
               "return a - b; "
               "}";
    } else if (operation == "multiply") {
        return "(params) => { "
               "const a = params[0] ? Number(params[0].value) : 0; "
               "const b = params[1] ? Number(params[1].value) : 0; "
               "return a * b; "
               "}";
    } else if (operation == "divide") {
        return "(params) => { "
               "const a = params[0] ? Number(params[0].value) : 0; "
               "const b = params[1] ? Number(params[1].value) : 0; "
               "return b !== 0 ? a / b : 0; "
               "}";
    }
    
    return "(params) => { return 0; }";
}

QString ScriptFunctionRegistry::buildConditionFunction(Node* node)
{
    Q_UNUSED(node);
    // Basic condition function - can be expanded later
    return "(params) => { "
           "const condition = params[0] ? params[0].value : false; "
           "return condition ? true : false; "
           "}";
}

QString ScriptFunctionRegistry::buildEventDataFunction(Node* node)
{
    Q_UNUSED(node);
    // Return the value from the global eventData object
    return "(params) => { "
           "if (typeof eventData !== 'undefined' && eventData.value !== undefined) { "
           "return eventData.value; "
           "} else { "
           "return ''; "
           "} "
           "}";
}

QString ScriptFunctionRegistry::buildAiPromptFunction(Node* node)
{
    Q_UNUSED(node);
    // This function returns a Promise for async execution
    return "(params, context) => { "
           "return new Promise((resolve, reject) => { "
           "const prompt = params && params.length > 0 && params[0] ? params[0].value : ''; "
           "if (!prompt) { "
           "resolve({ response: 'No prompt provided' }); "
           "return; "
           "} "
           "const token = typeof authManager !== 'undefined' && authManager.getAuthToken ? authManager.getAuthToken() : ''; "
           "if (!token) { "
           "resolve({ response: 'Authentication required' }); "
           "return; "
           "} "
           "if (typeof aiService === 'undefined') { "
           "resolve({ response: 'AI Service not available' }); "
           "return; "
           "} "
           "let responseHandler = null; "
           "let errorHandler = null; "
           "const cleanup = () => { "
           "if (responseHandler) aiService.responseReceived.disconnect(responseHandler); "
           "if (errorHandler) aiService.errorOccurred.disconnect(errorHandler); "
           "}; "
           "responseHandler = (response) => { "
           "cleanup(); "
           "resolve({ response: response }); "
           "}; "
           "errorHandler = (error) => { "
           "cleanup(); "
           "resolve({ response: 'Error: ' + error }); "
           "}; "
           "aiService.responseReceived.connect(responseHandler); "
           "aiService.errorOccurred.connect(errorHandler); "
           "aiService.callCubitAI(prompt, token); "
           "}); "
           "}";
}

QString ScriptFunctionRegistry::buildConvertNumberToStringFunction(Node* node)
{
    Q_UNUSED(node);
    // Convert the first parameter (which should be a number from the ForEachLoop) to a string
    return "(params) => { "
           "if (params && params.length > 0 && params[0] !== undefined) { "
           "const value = params[0].value !== undefined ? params[0].value : params[0]; "
           "return String(value); "
           "} "
           "return ''; "
           "}";
}

QString ScriptFunctionRegistry::buildCreateNumberArrayFunction(Node* node)
{
    if (!node) {
        return "(params) => { return { array: [] }; }";
    }
    
    // For dynamic nodes, the parameters contain the individual values
    // We need to create an array from all the parameter values
    return "(params) => { "
           "const array = []; "
           "if (params && params.length > 0) { "
           "for (let i = 0; i < params.length; i++) { "
           "if (params[i] && params[i].value !== undefined) { "
           "array.push(Number(params[i].value) || 0); "
           "} "
           "} "
           "} "
           "return { array: array }; "
           "}";
}