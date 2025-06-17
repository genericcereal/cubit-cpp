#ifndef SCRIPTGRAPHVALIDATOR_H
#define SCRIPTGRAPHVALIDATOR_H

#include <QString>
#include <QSet>
#include <QList>

class Scripts;
class Node;
class Edge;

class ScriptGraphValidator
{
public:
    ScriptGraphValidator();
    
    // Validate the entire script graph
    bool validate(Scripts* scripts);
    
    // Get the last validation error
    QString getLastError() const;
    
    // Check for cycles in the graph
    bool hasCycles(Scripts* scripts);
    
    // Check if all required ports are connected
    bool areRequiredPortsConnected(Scripts* scripts);
    
private:
    // Helper for cycle detection using DFS
    bool hasCyclesHelper(Node* node, QSet<QString>& visited, QSet<QString>& recursionStack, Scripts* scripts);
    
    // Get outgoing edges for a node
    QList<Edge*> getOutgoingFlowEdges(Node* node, Scripts* scripts);
    
    QString m_lastError;
};

#endif // SCRIPTGRAPHVALIDATOR_H