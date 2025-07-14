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
class ElementModel;

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
    
    struct NodeReference {
        Node* node;
        Scripts* scripts;  // The Scripts object that contains this node
    };
    
    struct BuildContext {
        QMap<QString, InvokeData> invokes;
        QMap<QString, QJsonObject> outputs;
        QMap<QString, NodeReference> nodeReferences;  // Map nodeId to node and its Scripts
        int invokeCounter = 0;
        int outputCounter = 0;
    };
    
    ScriptInvokeBuilder();
    
    // Build invokes for an event node
    BuildContext buildInvokes(Node* eventNode, Scripts* scripts, ElementModel* elementModel = nullptr);
    
    // Get function name for a node type
    QString getFunctionNameForNode(Node* node) const;
    
private:
    // Build invokes recursively starting from a node
    void buildInvokesRecursive(Node* node, Scripts* scripts, BuildContext& context, 
                               ElementModel* elementModel = nullptr,
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
    
    // Build invokes for component scripts
    void buildComponentInvokesRecursive(Node* node, Scripts* scripts, BuildContext& context,
                                       ElementModel* elementModel = nullptr);
};

#endif // SCRIPTINVOKEBUILDER_H