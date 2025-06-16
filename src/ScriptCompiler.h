#ifndef SCRIPTCOMPILER_H
#define SCRIPTCOMPILER_H

#include <QObject>
#include <QJsonObject>
#include <QJsonArray>
#include <QJsonDocument>
#include <QString>
#include <QHash>
#include <QSet>
#include <memory>

class Scripts;
class Node;
class Edge;

class ScriptCompiler : public QObject {
    Q_OBJECT

public:
    explicit ScriptCompiler(QObject *parent = nullptr);
    ~ScriptCompiler();

    // Main compilation method
    Q_INVOKABLE QString compile(Scripts* scripts);
    
    // Get the last compilation result as JSON object
    QJsonObject getCompiledJson() const;
    
    // Get the last compilation error (if any)
    QString getLastError() const;

private:
    struct CompilationContext {
        QHash<QString, QString> functionMap;  // Maps function types to function IDs
        QHash<QString, QString> outputMap;    // Maps node output IDs to output keys in outputs object
        QHash<QString, QString> nodeToInvokeMap;  // Maps node IDs to invoke IDs
        QJsonObject functions;  // Object with functionId as key, arrow function as value
        QJsonObject outputs;    // Object with outputId as key, output definition as value
        QJsonObject invokes;    // Object with invokeId as key, invoke object as value
        QSet<QString> processedNodes;
        int functionCounter = 0;
        int outputCounter = 0;
        int invokeCounter = 0;
    };
    
    QJsonObject m_lastCompiled;
    QString m_lastError;
    
    // Helper methods
    void processNode(Node* node, CompilationContext& context, Scripts* scripts);
    QJsonObject createInvokeObject(Node* node, CompilationContext& context, Scripts* scripts);
    QString getOrCreateFunction(const QString& functionType, CompilationContext& context);
    void buildFunctionDefinitions(CompilationContext& context);
    void convertNodeIdsToInvokeIds(CompilationContext& context);
    QStringList getNextNodeIds(Node* node, Scripts* scripts);
    bool validateGraph(Scripts* scripts);
    
    // Function builders for different node types - now return arrow function strings
    QString buildConsoleLogFunction();
    QString buildMathFunction(const QString& operation);
    QString buildVariableFunction(const QString& operation);
    QString buildDisplayFunction();
};

#endif // SCRIPTCOMPILER_H