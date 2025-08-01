#include "ScriptCanvasContext.h"
#include "../Scripts.h"
#include "../ElementModel.h"
#include "../Node.h"
#include "../Edge.h"
#include "../Component.h"
#include "../DesignElement.h"
#include "../PlatformConfig.h"
#include <QDebug>

ScriptCanvasContext::ScriptCanvasContext(Scripts* scripts, QObject* editingElement, QObject *parent)
    : CanvasContext(parent)
    , m_scripts(scripts)
    , m_editingElement(editingElement)
{
}

QString ScriptCanvasContext::displayName() const
{
    QString elementName = getEditingElementName();
    if (!elementName.isEmpty()) {
        return QString("Scripts - %1").arg(elementName);
    }
    return "Scripts - Canvas";
}

void ScriptCanvasContext::activateContext(ElementModel* targetModel)
{
    if (!targetModel || !m_scripts) return;
    
    // qDebug() << "ScriptCanvasContext::activateContext - Starting";
    
    // Clear existing script elements from the model first
    deactivateContext(targetModel);
    
    // Add all nodes to the element model
    auto nodes = m_scripts->getAllNodes();
    // qDebug() << "ScriptCanvasContext: Loading" << nodes.size() << "nodes";
    for (Node* node : nodes) {
        if (node) {
            // Add to model but Scripts retains ownership
            targetModel->addElement(node);
        }
    }
    
    // Add all edges to the element model
    auto edges = m_scripts->getAllEdges();
    // qDebug() << "ScriptCanvasContext: Loading" << edges.size() << "edges";
    for (Edge* edge : edges) {
        if (edge) {
            // Add to model but Scripts retains ownership
            targetModel->addElement(edge);
        }
    }
}

void ScriptCanvasContext::deactivateContext(ElementModel* targetModel)
{
    if (!targetModel) return;
    
    // Get all elements and remove nodes and edges
    auto elements = targetModel->getAllElements();
    QList<Element*> scriptElements;
    
    for (Element* element : elements) {
        if (element) {
            // Collect nodes and edges (script elements)
            if (qobject_cast<Node*>(element) || qobject_cast<Edge*>(element)) {
                scriptElements.append(element);
            }
        }
    }
    
    // Remove the elements from model without deleting them
    // They are owned by Scripts, not ElementModel
    for (Element* element : scriptElements) {
        targetModel->removeElementWithoutDelete(element);
    }
}

QString ScriptCanvasContext::getEditingElementName() const
{
    if (!m_editingElement) return QString();
    
    if (Component* comp = qobject_cast<Component*>(m_editingElement.data())) {
        return comp->getName();
    }
    if (DesignElement* elem = qobject_cast<DesignElement*>(m_editingElement.data())) {
        return elem->getName();
    }
    if (PlatformConfig* platform = qobject_cast<PlatformConfig*>(m_editingElement.data())) {
        return platform->name();
    }
    
    return QString();
}