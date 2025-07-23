#include "ScriptExecutor.h"
#include "Scripts.h"
#include "ElementModel.h"
#include "CanvasController.h"
#include "ConsoleMessageRepository.h"
#include <QJsonDocument>
#include <QJsonValue>
#include <QDebug>

// QtConsoleLog implementation
void QtConsoleLog::log(const QString& message)
{
    if (m_console) {
        QMetaObject::invokeMethod(m_console, "addOutput", Q_ARG(QString, message));
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

void ScriptExecutor::setConsole(QObject* console)
{
    m_console = console;
    // Re-setup JS context with the new console
    setupJSContext();
}

void ScriptExecutor::setupJSContext()
{
    // Register the QtConsoleLog helper object first
    QtConsoleLog* consoleLogger = new QtConsoleLog(&m_jsEngine, m_console);
    QJSValue qtConsole = m_jsEngine.newQObject(consoleLogger);
    m_jsEngine.globalObject().setProperty("_qtConsoleLog", qtConsole);
    
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
    
    // Store the event data for use in scripts
    m_currentEventData = eventData;
    
    // Check if scripts are compiled
    if (!m_scripts->isCompiled()) {
        // Compile the scripts
        QString compiledJson = m_scripts->compile(m_elementModel);
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
    
    // Get the function code
    QJsonObject functions = eventData["functions"].toObject();
    if (!functions.contains(functionId)) {
        qWarning() << "ScriptExecutor: Function not found:" << functionId;
        return;
    }
    
    QString functionCode = functions[functionId].toString();
    
    // Execute the function
    QJSValue result = evaluateFunction(functionCode, params);
    
    // Handle any outputs
    if (eventData.contains("outputs")) {
        QJsonObject outputs = eventData["outputs"].toObject();
        // Find output definition for this invoke if any
        for (auto it = outputs.begin(); it != outputs.end(); ++it) {
            QJsonObject outputDef = it.value().toObject();
            if (outputDef["sourceInvoke"].toString() == invokeId) {
                handleOutput(outputDef, result);
            }
        }
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
    QString wrapper = "(function() { "
                     "var fn = " + functionCode + "; "
                     "var params = " + QJsonDocument(resolvedParams).toJson(QJsonDocument::Compact) + "; "
                     "return fn(params); "
                     "})()";
    
    QJSValue result = m_jsEngine.evaluate(wrapper);
    
    if (result.isError()) {
        QString error = result.toString();
        if (m_console) {
            QMetaObject::invokeMethod(m_console, "addError", Q_ARG(QString, "Script execution error: " + error));
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
        QString message = value.toString();
        if (m_console) {
            QMetaObject::invokeMethod(m_console, "addOutput", Q_ARG(QString, message));
        }
    } else if (type == "variable") {
        // Store in variable (future implementation)
        QString variableId = outputDef["targetId"].toString();
        // TODO: Implement variable storage
    } else if (type == "element") {
        // Update element property (future implementation)
        QString elementId = outputDef["targetId"].toString();
        QString property = outputDef["targetProperty"].toString();
        // TODO: Implement element property updates
    }
}