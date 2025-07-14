#ifndef SCRIPTCOMPILER_H
#define SCRIPTCOMPILER_H

#include <QObject>
#include <QString>
#include <memory>

class Scripts;
class ScriptGraphValidator;
class ScriptInvokeBuilder;
class ScriptFunctionRegistry;
class ScriptSerializer;
class ElementModel;

class ScriptCompiler : public QObject {
    Q_OBJECT

public:
    explicit ScriptCompiler(QObject *parent = nullptr);
    ~ScriptCompiler();

    // Main compilation method
    Q_INVOKABLE QString compile(Scripts* scripts, ElementModel* elementModel = nullptr);
    
    // Get the last compilation error (if any)
    QString getLastError() const;

private:
    QString m_lastError;
    
    // Component classes
    std::unique_ptr<ScriptGraphValidator> m_validator;
    std::unique_ptr<ScriptInvokeBuilder> m_invokeBuilder;
    std::unique_ptr<ScriptFunctionRegistry> m_functionRegistry;
    std::unique_ptr<ScriptSerializer> m_serializer;
};

#endif // SCRIPTCOMPILER_H