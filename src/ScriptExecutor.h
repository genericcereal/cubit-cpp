#ifndef SCRIPTEXECUTOR_H
#define SCRIPTEXECUTOR_H

#include <QObject>
#include <QJSEngine>
#include <QJsonObject>
#include <QJsonArray>
#include <QMap>
#include <QPair>
#include <memory>

// Forward declaration
class ScriptExecutor;

// Helper class for console logging from JavaScript
class QtConsoleLog : public QObject
{
    Q_OBJECT
public:
    explicit QtConsoleLog(ScriptExecutor* executor, QObject *parent = nullptr);
    
    Q_INVOKABLE void log(const QString& message);
    
private:
    ScriptExecutor* m_executor;
};

class Scripts;
class ElementModel;
class CanvasController;

class ScriptExecutor : public QObject
{
    Q_OBJECT

public:
    explicit ScriptExecutor(QObject *parent = nullptr);
    ~ScriptExecutor();

    void setScripts(Scripts* scripts);
    void setElementModel(ElementModel* model);
    void setCanvasController(CanvasController* controller);

    // Execute a specific event with optional data
    void executeEvent(const QString& eventName, const QVariantMap& eventData = QVariantMap());
    
    // Handle async results from JavaScript
    Q_INVOKABLE void handleAsyncResult(const QString& invokeId, const QJSValue& result);
    Q_INVOKABLE void handleAsyncError(const QString& invokeId, const QJSValue& error);

private:
    // Execute a chain of invokes starting from given invoke IDs
    void executeInvokeChain(const QJsonObject& eventData, const QJsonArray& invokeIds);
    
    // Execute a single invoke
    void executeInvoke(const QJsonObject& eventData, const QString& invokeId);
    
    // Evaluate a JavaScript function with given parameters
    QJSValue evaluateFunction(const QString& functionCode, const QJsonArray& params);
    
    // Setup the JavaScript execution context with necessary globals
    void setupJSContext();
    
    // Handle output from scripts
    void handleOutput(const QJsonObject& outputDef, const QJSValue& value);

private:
    QJSEngine m_jsEngine;
    Scripts* m_scripts;
    ElementModel* m_elementModel;
    CanvasController* m_canvasController;
    QJsonObject m_compiledScript;
    QString m_currentEventName;
    QVariantMap m_currentEventData;
    QMap<QString, QPair<QJsonObject, QJsonArray>> m_pendingAsyncInvokes;
    QMap<QString, QJSValue> m_asyncResults; // Store async results by output ID
    QMap<QString, QJSValue> m_paramResults; // Store param node results by param invoke ID
};

#endif // SCRIPTEXECUTOR_H