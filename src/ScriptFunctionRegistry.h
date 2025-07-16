#ifndef SCRIPTFUNCTIONREGISTRY_H
#define SCRIPTFUNCTIONREGISTRY_H

#include <QString>
#include <QMap>
#include <functional>

class Node;

class ScriptFunctionRegistry
{
public:
    using FunctionBuilder = std::function<QString(Node*)>;
    
    ScriptFunctionRegistry();
    
    // Register a function builder for a node type
    void registerFunction(const QString& nodeType, FunctionBuilder builder);
    
    // Get the JavaScript function code for a node
    QString getFunctionCode(Node* node) const;
    
    // Check if a function is registered for a node type
    bool hasFunction(const QString& nodeType) const;
    
    // Initialize with default functions
    void registerDefaultFunctions();
    
private:
    QMap<QString, FunctionBuilder> m_functionBuilders;
    
    // Built-in function builders
    static QString buildConsoleLogFunction(Node* node);
    static QString buildVariableFunction(Node* node);
    static QString buildMathFunction(Node* node);
    static QString buildConditionFunction(Node* node);
    static QString buildEventDataFunction(Node* node);
};

#endif // SCRIPTFUNCTIONREGISTRY_H