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
    
    // Use the registered function builder
    QString nodeType = node->nodeTitle().toLower().remove(' ');
    
    auto it = m_functionBuilders.find(nodeType);
    if (it != m_functionBuilders.end()) {
        return it.value()(node);
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
}

QString ScriptFunctionRegistry::buildConsoleLogFunction(Node* node)
{
    Q_UNUSED(node);
    return "(params) => { "
           "if (params && params.length > 0 && params[0]) { "
           "console.log(params[0].value || ''); "
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