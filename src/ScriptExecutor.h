#ifndef SCRIPTEXECUTOR_H
#define SCRIPTEXECUTOR_H

#include <QObject>
#include <QJSEngine>
#include <QJsonObject>
#include <QJsonArray>
#include <memory>

// Helper class for console logging from JavaScript
class QtConsoleLog : public QObject
{
    Q_OBJECT
public:
    explicit QtConsoleLog(QObject *parent = nullptr) : QObject(parent) {}
    
    Q_INVOKABLE void log(const QString& message);
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
};

#endif // SCRIPTEXECUTOR_H