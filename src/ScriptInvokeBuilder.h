#ifndef SCRIPTINVOKEBUILDER_H
#define SCRIPTINVOKEBUILDER_H

#include <QString>
#include <QJsonObject>
#include <QJsonArray>
#include <QMap>
#include <QList>

class Scripts;
class Node;
class Edge;

class ScriptInvokeBuilder
{
public:
    struct InvokeData {
        QString invokeId;
        QString nodeId;
        QString functionName;
        QJsonArray params;
        QList<QString> nextInvokes;
    };
    
    struct BuildContext {
        QMap<QString, InvokeData> invokes;
        QMap<QString, QJsonObject> outputs;
        int invokeCounter = 0;
        int outputCounter = 0;
    };
    
    ScriptInvokeBuilder();
    
    // Build invokes for an event node
    BuildContext buildInvokes(Node* eventNode, Scripts* scripts);
    
    // Get function name for a node type
    QString getFunctionNameForNode(Node* node) const;
    
private:
    // Build invokes recursively starting from a node
    void buildInvokesRecursive(Node* node, Scripts* scripts, BuildContext& context, 
                               const QString& parentInvokeId = QString());
    
    // Create parameters for a node
    QJsonArray createNodeParameters(Node* node, Scripts* scripts, BuildContext& context);
    
    // Get data edges connected to a node's input ports
    QList<Edge*> getIncomingDataEdges(Node* node, Scripts* scripts);
    
    // Get flow edges going out from a node
    QList<Edge*> getOutgoingFlowEdges(Node* node, Scripts* scripts);
    
    // Generate unique invoke ID
    QString generateInvokeId(BuildContext& context);
    
    // Generate unique output ID
    QString generateOutputId(BuildContext& context);
};

#endif // SCRIPTINVOKEBUILDER_H