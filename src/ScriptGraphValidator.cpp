#include "ScriptGraphValidator.h"
#include "Scripts.h"
#include "Node.h"
#include "Edge.h"

ScriptGraphValidator::ScriptGraphValidator()
{
}

bool ScriptGraphValidator::validate(Scripts* scripts)
{
    if (!scripts) {
        m_lastError = "No scripts object provided";
        return false;
    }
    
    // Check for cycles
    if (hasCycles(scripts)) {
        m_lastError = "Circular dependency detected in script graph";
        return false;
    }
    
    // Check required ports
    if (!areRequiredPortsConnected(scripts)) {
        return false; // Error already set
    }
    
    m_lastError.clear();
    return true;
}

QString ScriptGraphValidator::getLastError() const
{
    return m_lastError;
}

bool ScriptGraphValidator::hasCycles(Scripts* scripts)
{
    QSet<QString> visited;
    QSet<QString> recursionStack;
    
    // Check each node for cycles
    QList<Node*> nodes = scripts->getAllNodes();
    for (Node* node : nodes) {
        if (!visited.contains(node->getId())) {
            if (hasCyclesHelper(node, visited, recursionStack, scripts)) {
                return true;
            }
        }
    }
    
    return false;
}

bool ScriptGraphValidator::hasCyclesHelper(Node* node, QSet<QString>& visited, 
                                          QSet<QString>& recursionStack, Scripts* scripts)
{
    visited.insert(node->getId());
    recursionStack.insert(node->getId());
    
    // Get all outgoing flow edges
    QList<Edge*> outgoingEdges = getOutgoingFlowEdges(node, scripts);
    
    for (Edge* edge : outgoingEdges) {
        QString targetNodeId = edge->targetNodeId();
        
        if (!visited.contains(targetNodeId)) {
            Node* targetNode = scripts->getNode(targetNodeId);
            if (targetNode && hasCyclesHelper(targetNode, visited, recursionStack, scripts)) {
                return true;
            }
        } else if (recursionStack.contains(targetNodeId)) {
            // Found a cycle
            return true;
        }
    }
    
    recursionStack.remove(node->getId());
    return false;
}

QList<Edge*> ScriptGraphValidator::getOutgoingFlowEdges(Node* node, Scripts* scripts)
{
    QList<Edge*> result;
    QList<Edge*> edges = scripts->getOutgoingEdges(node->getId());
    
    for (Edge* edge : edges) {
        if (edge->sourcePortType() == "Flow") {
            result.append(edge);
        }
    }
    
    return result;
}

bool ScriptGraphValidator::areRequiredPortsConnected(Scripts* scripts)
{
    Q_UNUSED(scripts);
    // For now, we don't have required ports marked in the system
    // This is a placeholder for future validation
    return true;
}