#include "ScriptExecutor.h"
#include "Scripts.h"
#include "ElementModel.h"
#include "CanvasController.h"
#include "ConsoleMessageRepository.h"
#include "AuthenticationManager.h"
#include "AIService.h"
#include "Project.h"
#include "Variable.h"
#include <QJsonDocument>
#include <QJsonValue>
#include <QDebug>
#include <QApplication>
#include <QJSValueIterator>

// QtConsoleLog implementation
QtConsoleLog::QtConsoleLog(ScriptExecutor* executor, QObject *parent)
    : QObject(parent)
    , m_executor(executor)
{
}

void QtConsoleLog::log(const QString& message)
{
    if (m_executor && m_executor->parent()) {
        Project* project = qobject_cast<Project*>(m_executor->parent());
        if (project && project->console()) {
            project->console()->addOutput(message);
        }
    }
}

ScriptExecutor::ScriptExecutor(QObject *parent)
    : QObject(parent)
    , m_scripts(nullptr)
    , m_elementModel(nullptr)
    , m_canvasController(nullptr)
{
    setupJSContext();
}

ScriptExecutor::~ScriptExecutor() = default;

void ScriptExecutor::setScripts(Scripts* scripts)
{
    m_scripts = scripts;
}

void ScriptExecutor::setElementModel(ElementModel* model)
{
    m_elementModel = model;
}

void ScriptExecutor::setCanvasController(CanvasController* controller)
{
    m_canvasController = controller;
}


void ScriptExecutor::setupJSContext()
{
    // Register the QtConsoleLog helper object first
    QtConsoleLog* consoleLogger = new QtConsoleLog(this, &m_jsEngine);
    QJSValue qtConsole = m_jsEngine.newQObject(consoleLogger);
    m_jsEngine.globalObject().setProperty("_qtConsoleLog", qtConsole);
    
    // Register the ScriptExecutor for async callbacks
    QJSValue scriptExecutor = m_jsEngine.newQObject(this);
    m_jsEngine.globalObject().setProperty("_scriptExecutor", scriptExecutor);
    
    // Create and register AIService
    AIService* aiService = new AIService(&m_jsEngine);
    QJSValue aiServiceValue = m_jsEngine.newQObject(aiService);
    m_jsEngine.globalObject().setProperty("aiService", aiServiceValue);
    
    // Try to find and register the AuthenticationManager
    // Look for it in the parent hierarchy (it's a global object)
    QObject* p = parent();
    while (p) {
        // Check if this is the QApplication
        if (qobject_cast<QApplication*>(p)) {
            // Find AuthenticationManager among app's children
            AuthenticationManager* authManager = p->findChild<AuthenticationManager*>();
            if (authManager) {
                QJSValue authManagerValue = m_jsEngine.newQObject(authManager);
                m_jsEngine.globalObject().setProperty("authManager", authManagerValue);
            }
            break;
        }
        p = p->parent();
    }
    
    // Create console object with log function
    QString consoleScript = 
        "var console = {"
        "    log: function() {"
        "        var args = Array.prototype.slice.call(arguments);"
        "        var message = args.join(' ');"
        "        _qtConsoleLog.log(message);"
        "    }"
        "};";
    
    QJSValue result = m_jsEngine.evaluate(consoleScript);
    if (result.isError()) {
        qWarning() << "Failed to setup console:" << result.toString();
    }
}

void ScriptExecutor::executeEvent(const QString& eventName, const QVariantMap& eventData)
{
    if (!m_scripts) {
        qWarning() << "ScriptExecutor: No scripts object set";
        return;
    }
    
    // Clear any previous async results and param results
    m_asyncResults.clear();
    m_paramResults.clear();
    
    // Store the event data for use in scripts
    m_currentEventData = eventData;
    
    // Check if scripts are compiled
    if (!m_scripts->isCompiled()) {
        // Get console from parent project
        QObject* console = nullptr;
        if (parent()) {
            Project* project = qobject_cast<Project*>(parent());
            if (project) {
                console = project->console();
            }
        }
        
        // Compile the scripts
        QString compiledJson = m_scripts->compile(m_elementModel, console);
        if (compiledJson.isEmpty()) {
            return; // Compilation failed
        }
    }
    
    // Get the stored compiled script
    QString compiledJson = m_scripts->compiledScript();
    if (compiledJson.isEmpty()) {
        return; // No compiled script available
    }
    
    QJsonDocument doc = QJsonDocument::fromJson(compiledJson.toUtf8());
    if (!doc.isObject()) {
        qWarning() << "ScriptExecutor: Invalid compiled script format";
        return;
    }
    
    m_compiledScript = doc.object();
    
    // Find the event
    QString normalizedEventName = eventName.toLower().remove(' ');
    
    if (!m_compiledScript.contains(normalizedEventName)) {
        // Event not found, this is normal if no scripts are attached to this event
        return;
    }
    
    // Store the current event name for use in evaluateFunction
    m_currentEventName = normalizedEventName;
    
    QJsonObject compiledEventData = m_compiledScript[normalizedEventName].toObject();
    if (!compiledEventData.contains("next")) {
        return;
    }
    
    // Execute the chain of invokes
    QJsonArray nextInvokes = compiledEventData["next"].toArray();
    executeInvokeChain(compiledEventData, nextInvokes);
}

void ScriptExecutor::executeInvokeChain(const QJsonObject& eventData, const QJsonArray& invokeIds)
{
    for (const QJsonValue& invokeIdValue : invokeIds) {
        QString invokeId = invokeIdValue.toString();
        executeInvoke(eventData, invokeId);
    }
}

void ScriptExecutor::executeInvoke(const QJsonObject& eventData, const QString& invokeId)
{
    QJsonObject invokes = eventData["invoke"].toObject();
    
    if (!invokes.contains(invokeId)) {
        qWarning() << "ScriptExecutor: Invoke not found:" << invokeId;
        return;
    }
    
    QJsonObject invoke = invokes[invokeId].toObject();
    QString functionId = invoke["function"].toString();
    QJsonArray params = invoke["params"].toArray();
    bool isAsync = invoke["isAsync"].toBool(false);
    bool isLoop = invoke["isLoop"].toBool(false);
    
    // Get the function code
    QJsonObject functions = eventData["functions"].toObject();
    if (!functions.contains(functionId)) {
        qWarning() << "ScriptExecutor: Function not found:" << functionId;
        return;
    }
    
    QString functionCode = functions[functionId].toString();
    
    // Execute the function
    // qDebug() << "ScriptExecutor: Executing function for invoke" << invokeId << "functionId:" << functionId;
    QJSValue result = evaluateFunction(functionCode, params);
    
    if (isAsync && result.isObject() && result.hasProperty("then")) {
        
        // This is a Promise - we need to wait for it
        QJSValue thenFunc = result.property("then");
        
        // Create callbacks for promise resolution
        QJSValue onResolved = m_jsEngine.evaluate(
            "(function(value) {"
            "   _scriptExecutor.handleAsyncResult('" + invokeId + "', value);"
            "})"
        );
        
        QJSValue onRejected = m_jsEngine.evaluate(
            "(function(error) {"
            "   _scriptExecutor.handleAsyncError('" + invokeId + "', error);"
            "})"
        );
        
        // Call then() with our callbacks
        thenFunc.callWithInstance(result, QJSValueList() << onResolved << onRejected);
        
        // Store the event data and next invokes for later
        m_pendingAsyncInvokes[invokeId] = QPair<QJsonObject, QJsonArray>(
            eventData, 
            invoke.contains("next") ? invoke["next"].toArray() : QJsonArray()
        );
        
        // Don't continue with next invokes yet - wait for async completion
        return;
    }
    
    // Handle any outputs
    if (eventData.contains("outputs")) {
        QJsonObject outputs = eventData["outputs"].toObject();
        // Find output definition for this invoke if any
        for (auto it = outputs.begin(); it != outputs.end(); ++it) {
            QString outputId = it.key();
            QJsonObject outputDef = it.value().toObject();
            if (outputDef["sourceInvoke"].toString() == invokeId) {
                // Store the result for this output so other nodes can use it
                if (outputDef["type"].toString() == "invokeResult") {
                    m_asyncResults[outputId] = result;
                }
                handleOutput(outputDef, result);
            }
        }
    }
    
    // Handle loop invokes
    if (isLoop) {
        // qDebug() << "ScriptExecutor: Processing loop invoke" << invokeId;
        QString loopBodyInvoke = invoke["loopBody"].toString();
        QString loopCompleteInvoke = invoke["loopComplete"].toString();
        // qDebug() << "ScriptExecutor: Loop body invoke:" << loopBodyInvoke << "complete invoke:" << loopCompleteInvoke;
        
        // Extract the array from the result
        QVariantList arrayData;
        if (result.isObject() && result.hasProperty("array")) {
            QJSValue arrayValue = result.property("array");
            if (arrayValue.isArray()) {
                int length = arrayValue.property("length").toInt();
                // qDebug() << "ScriptExecutor: Found array with length" << length;
                for (int i = 0; i < length; i++) {
                    arrayData.append(arrayValue.property(i).toVariant());
                }
            } else {
                qWarning() << "ScriptExecutor: Result has 'array' property but it's not an array";
            }
        } else {
            qWarning() << "ScriptExecutor: Result doesn't have 'array' property";
            if (result.isObject()) {
                // List all properties
                QJSValueIterator it(result);
                while (it.hasNext()) {
                    it.next();
                    qDebug() << "  Property:" << it.name() << "=" << it.value().toString();
                }
            }
        }
        
        // qDebug() << "ScriptExecutor: Executing ForEachLoop with" << arrayData.size() << "items";
        
        // Execute the loop body for each item
        for (int i = 0; i < arrayData.size(); i++) {
            // Clear param results cache for this iteration
            m_paramResults.clear();
            
            // Make the current item available as a variable
            QJSValue itemValue = m_jsEngine.toScriptValue(arrayData[i]);
            // qDebug() << "ScriptExecutor: Setting currentLoopItem to:" << arrayData[i] << "as QJSValue:" << itemValue.toString();
            m_jsEngine.globalObject().setProperty("currentLoopItem", itemValue);
            m_jsEngine.globalObject().setProperty("currentLoopIndex", i);
            
            // Re-evaluate the loop node's function to get outputs with current item/index
            QJSValue loopResult = evaluateFunction(functionCode, params);
            
            // Handle outputs for the current iteration
            if (eventData.contains("outputs")) {
                QJsonObject outputs = eventData["outputs"].toObject();
                for (auto it = outputs.begin(); it != outputs.end(); ++it) {
                    QJsonObject outputDef = it.value().toObject();
                    if (outputDef["sourceInvoke"].toString() == invokeId) {
                        handleOutput(outputDef, loopResult);
                    }
                }
            }
            
            // Execute the loop body
            if (!loopBodyInvoke.isEmpty()) {
                executeInvoke(eventData, loopBodyInvoke);
            }
        }
        
        // Clear loop variables
        m_jsEngine.globalObject().deleteProperty("currentLoopItem");
        m_jsEngine.globalObject().deleteProperty("currentLoopIndex");
        
        // Execute the complete branch
        if (!loopCompleteInvoke.isEmpty()) {
            executeInvoke(eventData, loopCompleteInvoke);
        }
        
        // Don't execute normal next invokes for loops
        return;
    }
    
    // Execute next invokes in the chain
    if (invoke.contains("next")) {
        QJsonArray nextInvokes = invoke["next"].toArray();
        executeInvokeChain(eventData, nextInvokes);
    }
}

QJSValue ScriptExecutor::evaluateFunction(const QString& functionCode, const QJsonArray& params)
{
    // Make event data available as a global variable in the script
    if (!m_currentEventData.isEmpty()) {
        QJSValue eventDataObj = m_jsEngine.newObject();
        for (auto it = m_currentEventData.begin(); it != m_currentEventData.end(); ++it) {
            eventDataObj.setProperty(it.key(), m_jsEngine.toScriptValue(it.value()));
        }
        m_jsEngine.globalObject().setProperty("eventData", eventDataObj);
    }
    
    // First, we need to resolve any output references in the parameters
    QJsonArray resolvedParams;
    
    for (const QJsonValue& param : params) {
        QJsonObject paramObj = param.toObject();
        QJsonObject resolvedParam;
        
        if (paramObj.contains("output")) {
            // This is an output reference, we need to get the actual value
            QString outputId = paramObj["output"].toString();
            QJsonObject outputs = m_compiledScript[m_currentEventName].toObject()["outputs"].toObject();
            
            if (outputs.contains(outputId)) {
                QJsonObject outputDef = outputs[outputId].toObject();
                QString outputType = outputDef["type"].toString();
                
                // qDebug() << "ScriptExecutor: Resolving output" << outputId << "type:" << outputType << "outputDef:" << outputDef;
                
                if (outputType == "eventData") {
                    // For event data outputs, get the value from the current event data
                    int sourcePortIndex = outputDef["sourcePortIndex"].toInt();
                    
                    // Map port indices to event data keys
                    // For WebTextInput events: port 0 = "done" (Flow), port 1 = "value" (String)
                    QString eventDataKey = "value"; // Default to "value"
                    if (sourcePortIndex == 1) {
                        eventDataKey = "value";
                    }
                    
                    // Get the value from the current event data and convert to JSON-compatible type
                    if (m_currentEventData.contains(eventDataKey)) {
                        QVariant variant = m_currentEventData[eventDataKey];
                        resolvedParam["value"] = QJsonValue::fromVariant(variant);
                    } else {
                        resolvedParam["value"] = "";
                    }
                } else if (outputType == "invokeResult") {
                    // For invoke results, check if we have a stored result
                    if (m_asyncResults.contains(outputId)) {
                        QJSValue asyncResult = m_asyncResults[outputId];
                        // Convert QJSValue to QVariant for JSON serialization
                        if (asyncResult.isObject()) {
                            // For objects, we need to convert to a proper JSON structure
                            QVariantMap resultMap;
                            if (asyncResult.hasProperty("array")) {
                                // Handle array results
                                QJSValue arrayValue = asyncResult.property("array");
                                if (arrayValue.isArray()) {
                                    QVariantList arrayList;
                                    int length = arrayValue.property("length").toInt();
                                    for (int i = 0; i < length; i++) {
                                        arrayList.append(arrayValue.property(i).toVariant());
                                    }
                                    resultMap["array"] = arrayList;
                                }
                            } else if (asyncResult.hasProperty("response")) {
                                // Handle async responses
                                resultMap["response"] = asyncResult.property("response").toString();
                            }
                            resolvedParam["value"] = QJsonValue::fromVariant(resultMap);
                        } else {
                            resolvedParam["value"] = QJsonValue::fromVariant(asyncResult.toVariant());
                        }
                    } else {
                        // Check if this is a ForEachLoop output that should use current loop values
                        QString sourceInvoke = outputDef["sourceInvoke"].toString();
                        QString sourceNodeId = outputDef["sourceNodeId"].toString();
                        int sourcePortIndex = outputDef["sourcePortIndex"].toInt();
                        
                        // Check if we're in a loop context and this output is from the loop node
                        if (m_jsEngine.globalObject().hasProperty("currentLoopItem") && 
                            m_jsEngine.globalObject().hasProperty("currentLoopIndex")) {
                            
                            // Check if this output is from a ForEachLoop node
                            QJsonObject invokes = m_compiledScript[m_currentEventName].toObject()["invoke"].toObject();
                            if (invokes.contains(sourceInvoke)) {
                                QJsonObject invoke = invokes[sourceInvoke].toObject();
                                if (invoke["isLoop"].toBool()) {
                                    // This is a loop output - use current loop values
                                    if (sourcePortIndex == 1) {
                                        // Array Element port - use currentLoopItem
                                        QJSValue currentItem = m_jsEngine.globalObject().property("currentLoopItem");
                                        resolvedParam["value"] = QJsonValue::fromVariant(currentItem.toVariant());
                                    } else if (sourcePortIndex == 2) {
                                        // Array Index port - use currentLoopIndex
                                        QJSValue currentIndex = m_jsEngine.globalObject().property("currentLoopIndex");
                                        resolvedParam["value"] = QJsonValue::fromVariant(currentIndex.toVariant());
                                    } else {
                                        resolvedParam["value"] = "";
                                    }
                                } else {
                                    resolvedParam["value"] = "";
                                }
                            } else {
                                resolvedParam["value"] = "";
                            }
                        } else {
                            resolvedParam["value"] = "";
                        }
                    }
                } else if (outputType == "paramResult") {
                    // For param results, we need to evaluate the param function
                    QString sourceInvoke = outputDef["sourceInvoke"].toString();
                    
                    // Check if we already computed this param
                    if (m_paramResults.contains(sourceInvoke)) {
                        QJSValue paramResult = m_paramResults[sourceInvoke];
                        if (paramResult.isString()) {
                            // If the result is a string, use it directly
                            resolvedParam["value"] = paramResult.toString();
                        } else if (paramResult.isNumber()) {
                            // If the result is a number, convert to variant
                            resolvedParam["value"] = paramResult.toNumber();
                        } else if (paramResult.isObject()) {
                            QVariantMap resultMap;
                            if (paramResult.hasProperty("array")) {
                                QJSValue arrayValue = paramResult.property("array");
                                if (arrayValue.isArray()) {
                                    QVariantList arrayList;
                                    int length = arrayValue.property("length").toInt();
                                    for (int i = 0; i < length; i++) {
                                        arrayList.append(arrayValue.property(i).toVariant());
                                    }
                                    resultMap["array"] = arrayList;
                                }
                            }
                            if (paramResult.hasProperty("string")) {
                                QJSValue stringValue = paramResult.property("string");
                                resultMap["string"] = stringValue.toString();
                            }
                            resolvedParam["value"] = QJsonValue::fromVariant(resultMap);
                        } else {
                            resolvedParam["value"] = QJsonValue::fromVariant(paramResult.toVariant());
                        }
                    } else {
                        // Evaluate the param function
                        QJsonObject paramInvokes = m_compiledScript[m_currentEventName].toObject()["params"].toObject();
                        if (paramInvokes.contains(sourceInvoke)) {
                            QJsonObject paramInvoke = paramInvokes[sourceInvoke].toObject();
                            QString functionName = paramInvoke["function"].toString();
                            QJsonArray paramParams = paramInvoke["params"].toArray();
                            
                            // Get the function code
                            QJsonObject functions = m_compiledScript[m_currentEventName].toObject()["functions"].toObject();
                            if (functions.contains(functionName)) {
                                QString functionCode = functions[functionName].toString();
                                
                                // Pre-resolve output references in param parameters for loop context
                                QJsonArray resolvedParamParams;
                                for (const QJsonValue& paramParam : paramParams) {
                                    QJsonObject paramParamObj = paramParam.toObject();
                                    QJsonObject resolvedParamParam;
                                    
                                    if (paramParamObj.contains("output")) {
                                        QString outputId = paramParamObj["output"].toString();
                                        QJsonObject outputs = m_compiledScript[m_currentEventName].toObject()["outputs"].toObject();
                                        
                                        if (outputs.contains(outputId)) {
                                            QJsonObject outputDef = outputs[outputId].toObject();
                                            QString outputType = outputDef["type"].toString();
                                            
                                            // qDebug() << "ScriptExecutor: Resolving param param output" << outputId << "type:" << outputType << "outputDef:" << outputDef;
                                            
                                            if (outputType == "invokeResult") {
                                                // Check if this is a ForEachLoop output in loop context
                                                QString sourceInvoke = outputDef["sourceInvoke"].toString();
                                                int sourcePortIndex = outputDef["sourcePortIndex"].toInt();
                                                
                                                // qDebug() << "ScriptExecutor: Checking loop context - hasLoopItem:" << m_jsEngine.globalObject().hasProperty("currentLoopItem") << "hasLoopIndex:" << m_jsEngine.globalObject().hasProperty("currentLoopIndex");
                                                if (m_jsEngine.globalObject().hasProperty("currentLoopItem") && 
                                                    m_jsEngine.globalObject().hasProperty("currentLoopIndex")) {
                                                    
                                                    QJsonObject invokes = m_compiledScript[m_currentEventName].toObject()["invoke"].toObject();
                                                    if (invokes.contains(sourceInvoke)) {
                                                        QJsonObject invoke = invokes[sourceInvoke].toObject();
                                                        if (invoke["isLoop"].toBool()) {
                                                            // This is a loop output - use current loop values
                                                            // qDebug() << "ScriptExecutor: Loop output with sourcePortIndex:" << sourcePortIndex;
                                                            if (sourcePortIndex == 1) {
                                                                // Array Element port - use currentLoopItem
                                                                QJSValue currentItem = m_jsEngine.globalObject().property("currentLoopItem");
                                                                // qDebug() << "ScriptExecutor: Current loop item value:" << currentItem.toString() << "toVariant:" << currentItem.toVariant();
                                                                resolvedParamParam["value"] = QJsonValue::fromVariant(currentItem.toVariant());
                                                            } else if (sourcePortIndex == 2) {
                                                                // Array Index port - use currentLoopIndex
                                                                QJSValue currentIndex = m_jsEngine.globalObject().property("currentLoopIndex");
                                                                // qDebug() << "ScriptExecutor: Current loop index value:" << currentIndex.toString() << "toVariant:" << currentIndex.toVariant();
                                                                resolvedParamParam["value"] = QJsonValue::fromVariant(currentIndex.toVariant());
                                                            } else {
                                                                resolvedParamParam["value"] = "";
                                                            }
                                                        } else {
                                                            resolvedParamParam["value"] = "";
                                                        }
                                                    } else {
                                                        resolvedParamParam["value"] = "";
                                                    }
                                                } else {
                                                    resolvedParamParam["value"] = "";
                                                }
                                            } else if (outputType == "literal") {
                                                // For literal outputs, use the stored value
                                                resolvedParamParam["value"] = outputDef["value"];
                                            } else {
                                                resolvedParamParam["value"] = "";
                                            }
                                        } else {
                                            resolvedParamParam["value"] = "";
                                        }
                                    } else if (paramParamObj.contains("value")) {
                                        resolvedParamParam["value"] = paramParamObj["value"];
                                    }
                                    
                                    resolvedParamParams.append(resolvedParamParam);
                                }
                                
                                // Evaluate the param function with resolved parameters
                                // qDebug() << "ScriptExecutor: Evaluating param function" << functionName << "with params:" << resolvedParamParams;
                                QJSValue result = evaluateFunction(functionCode, resolvedParamParams);
                                // qDebug() << "ScriptExecutor: Param function result:" << result.toString();
                                // qDebug() << "ScriptExecutor: Result isString:" << result.isString() << "isObject:" << result.isObject() << "isNumber:" << result.isNumber();
                                
                                // Special handling for string "0" 
                                if (result.isString() && result.toString() == "[object Object]") {
                                    // qDebug() << "ScriptExecutor: WARNING - String result is showing as [object Object]";
                                    // Try to get the actual string value
                                    QString strValue = result.toString();
                                    // qDebug() << "ScriptExecutor: Actual string value:" << strValue;
                                }
                                if (result.isObject() && result.hasProperty("string")) {
                                    // qDebug() << "ScriptExecutor: Param function returned string property:" << result.property("string").toString();
                                }
                                
                                // Store the result for reuse
                                m_paramResults[sourceInvoke] = result;
                                
                                // Convert result to proper format
                                if (result.isString()) {
                                    // If the result is a string, use it directly
                                    resolvedParam["value"] = result.toString();
                                } else if (result.isNumber()) {
                                    // If the result is a number, convert to variant
                                    resolvedParam["value"] = result.toNumber();
                                } else if (result.isObject()) {
                                    QVariantMap resultMap;
                                    if (result.hasProperty("array")) {
                                        QJSValue arrayValue = result.property("array");
                                        if (arrayValue.isArray()) {
                                            QVariantList arrayList;
                                            int length = arrayValue.property("length").toInt();
                                            for (int i = 0; i < length; i++) {
                                                arrayList.append(arrayValue.property(i).toVariant());
                                            }
                                            resultMap["array"] = arrayList;
                                        }
                                    }
                                    if (result.hasProperty("string")) {
                                        QJSValue stringValue = result.property("string");
                                        resultMap["string"] = stringValue.toString();
                                    }
                                    resolvedParam["value"] = QJsonValue::fromVariant(resultMap);
                                } else {
                                    resolvedParam["value"] = QJsonValue::fromVariant(result.toVariant());
                                }
                            }
                        }
                    }
                } else if (outputDef.contains("value")) {
                    // For literal outputs, use the stored value
                    resolvedParam["value"] = outputDef["value"];
                } else {
                    // TODO: Handle other computed outputs
                    resolvedParam["value"] = "";
                }
            }
        } else if (paramObj.contains("value")) {
            // Direct value parameter
            resolvedParam["value"] = paramObj["value"];
        }
        
        resolvedParams.append(resolvedParam);
    }
    
    // Create a wrapper to call the function with parameters
    // Check if function expects context parameter (for async functions)
    bool needsContext = functionCode.contains("(params, context)");
    
    QString wrapper;
    if (needsContext) {
        wrapper = "(function() { "
                 "var fn = " + functionCode + "; "
                 "var params = " + QJsonDocument(resolvedParams).toJson(QJsonDocument::Compact) + "; "
                 "var context = {}; "  // Empty context for now
                 "return fn(params, context); "
                 "})()";
    } else {
        wrapper = "(function() { "
                 "var fn = " + functionCode + "; "
                 "var params = " + QJsonDocument(resolvedParams).toJson(QJsonDocument::Compact) + "; "
                 "return fn(params); "
                 "})()";
    }
    
    
    QJSValue result = m_jsEngine.evaluate(wrapper);
    
    if (result.isError()) {
        QString error = result.toString();
        if (parent()) {
            Project* project = qobject_cast<Project*>(parent());
            if (project && project->console()) {
                project->console()->addError("Script execution error: " + error);
            }
        }
        qWarning() << "ScriptExecutor: JavaScript error:" << error;
    }
    
    return result;
}

void ScriptExecutor::handleOutput(const QJsonObject& outputDef, const QJSValue& value)
{
    QString type = outputDef["type"].toString();
    
    if (type == "console") {
        // Output to console
        QString message;
        
        // Check if the value is an object with a specific property to extract
        if (value.isObject()) {
            // For async results like AI responses, extract the response property if it exists
            if (value.hasProperty("response")) {
                message = value.property("response").toString();
            } else {
                // If no specific property, convert the whole object to string
                message = value.toString();
            }
        } else {
            // For simple values, just convert to string
            message = value.toString();
        }
        
        if (parent()) {
            Project* project = qobject_cast<Project*>(parent());
            if (project && project->console()) {
                project->console()->addOutput(message);
            }
        }
    } else if (type == "invokeResult") {
        // This output will be populated by an async result
        // We'll update the outputs in the compiled script so subsequent nodes can use it
    } else if (type == "variable") {
        // Store in variable
        QString variableId = outputDef["targetId"].toString();
        
        if (m_elementModel && !variableId.isEmpty()) {
            // Get the variable element
            Element* element = m_elementModel->getElementById(variableId);
            if (element) {
                Variable* variable = qobject_cast<Variable*>(element);
                if (variable) {
                    // Extract the value from the result
                    QString newValue;
                    if (value.isObject()) {
                        // The result is an object with variableId and value properties
                        if (value.hasProperty("value")) {
                            newValue = value.property("value").toString();
                        } else {
                            newValue = value.toString();
                        }
                    } else {
                        newValue = value.toString();
                    }
                    
                    // Update the variable value
                    variable->setValue(QVariant(newValue));
                    
                    // Log the update
                    if (parent()) {
                        Project* project = qobject_cast<Project*>(parent());
                        if (project && project->console()) {
                            project->console()->addOutput(QString("Variable '%1' updated to: %2")
                                .arg(variable->getName())
                                .arg(newValue));
                        }
                    }
                }
            }
        }
    } else if (type == "element") {
        // Update element property (future implementation)
        QString elementId = outputDef["targetId"].toString();
        QString property = outputDef["targetProperty"].toString();
        // TODO: Implement element property updates
    }
}

void ScriptExecutor::handleAsyncResult(const QString& invokeId, const QJSValue& result)
{
    if (!m_pendingAsyncInvokes.contains(invokeId)) {
        // Async invoke already processed or doesn't exist
        return;
    }
    
    
    // Debug: log the result
    if (result.isObject()) {
        if (result.hasProperty("response")) {
        }
    }
    
    QPair<QJsonObject, QJsonArray> pendingData = m_pendingAsyncInvokes.take(invokeId);
    QJsonObject eventData = pendingData.first;
    QJsonArray nextInvokes = pendingData.second;
    
    // Store async results for later use by other nodes
    if (eventData.contains("outputs")) {
        QJsonObject outputs = eventData["outputs"].toObject();
        
        // Find output definition for this invoke if any
        for (auto it = outputs.begin(); it != outputs.end(); ++it) {
            QString outputId = it.key();
            QJsonObject outputDef = it.value().toObject();
            QString sourceInvoke = outputDef["sourceInvoke"].toString();
            
            if (sourceInvoke == invokeId) {
                // Store the async result for this output
                m_asyncResults[outputId] = result;
                
                // Also handle the output immediately if it's a console output
                if (outputDef["type"].toString() == "console") {
                    handleOutput(outputDef, result);
                }
            }
        }
    } else {
    }
    
    // Continue with next invokes
    if (!nextInvokes.isEmpty()) {
        executeInvokeChain(eventData, nextInvokes);
    }
}

void ScriptExecutor::handleAsyncError(const QString& invokeId, const QJSValue& error)
{
    QString errorMessage = error.toString();
    if (parent()) {
        Project* project = qobject_cast<Project*>(parent());
        if (project && project->console()) {
            project->console()->addError("Async script error in invoke " + invokeId + ": " + errorMessage);
        }
    }
    qWarning() << "ScriptExecutor: Async error in invoke" << invokeId << ":" << errorMessage;
    
    // Remove from pending
    m_pendingAsyncInvokes.remove(invokeId);
}