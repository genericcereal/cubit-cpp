#ifndef SCRIPTSERIALIZER_H
#define SCRIPTSERIALIZER_H

#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QMap>
#include "ScriptInvokeBuilder.h"

class Scripts;
class Node;
class ScriptFunctionRegistry;

class ScriptSerializer
{
public:
    ScriptSerializer();
    
    // Set the function registry to use
    void setFunctionRegistry(ScriptFunctionRegistry* registry);
    
    // Serialize the build context into a JSON document
    QJsonDocument serialize(const QMap<QString, ScriptInvokeBuilder::BuildContext>& eventContexts,
                           Scripts* scripts);
    
private:
    // Serialize a single event's context
    QJsonObject serializeEventContext(const ScriptInvokeBuilder::BuildContext& context,
                                     const QList<QString>& initialInvokes,
                                     Scripts* scripts);
    
    // Serialize invokes
    QJsonObject serializeInvokes(const ScriptInvokeBuilder::BuildContext& context);
    
    // Serialize outputs
    QJsonObject serializeOutputs(const ScriptInvokeBuilder::BuildContext& context);
    
    // Serialize functions
    QJsonObject serializeFunctions(const ScriptInvokeBuilder::BuildContext& context,
                                  Scripts* scripts);
    
    // Get unique function names from context
    QSet<QString> getUniqueFunctionNames(const ScriptInvokeBuilder::BuildContext& context);
    
    ScriptFunctionRegistry* m_functionRegistry;
};

#endif // SCRIPTSERIALIZER_H