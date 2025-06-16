#include "Canvas.h"
#include "CanvasController.h"
#include "SelectionManager.h"
#include "ElementModel.h"
#include "Scripts.h"
#include "Node.h"
#include "Edge.h"

Canvas::Canvas(const QString& id, const QString& name, QObject *parent)
    : QObject(parent)
    , m_name(name.isEmpty() ? "Untitled Canvas" : name)
    , m_id(id)
    , m_viewMode("design")
{
}

Canvas::~Canvas() = default;

CanvasController* Canvas::controller() const {
    return m_controller.get();
}

SelectionManager* Canvas::selectionManager() const {
    return m_selectionManager.get();
}

ElementModel* Canvas::elementModel() const {
    return m_elementModel.get();
}

Scripts* Canvas::scripts() const {
    return m_scripts.get();
}

QString Canvas::name() const {
    return m_name;
}

QString Canvas::id() const {
    return m_id;
}

QString Canvas::viewMode() const {
    return m_viewMode;
}

void Canvas::setName(const QString& name) {
    if (m_name != name) {
        m_name = name;
        emit nameChanged();
    }
}

void Canvas::setViewMode(const QString& viewMode) {
    if (m_viewMode != viewMode) {
        m_viewMode = viewMode;
        
        // When switching to script mode, check if we have a selected design element
        if (viewMode == "script") {
            if (m_selectionManager) {
                auto selectedElements = m_selectionManager->selectedElements();
                qDebug() << "Canvas: Switching to script mode, selected elements:" << selectedElements.size();
                if (selectedElements.size() == 1) {
                    auto element = selectedElements.first();
                    if (element && element->isVisual()) {
                        CanvasElement* canvasElement = qobject_cast<CanvasElement*>(element);
                        if (canvasElement && canvasElement->isDesignElement()) {
                            qDebug() << "Canvas: Setting editing element to" << element->getId();
                            setEditingElement(qobject_cast<DesignElement*>(element));
                        } else {
                            qDebug() << "Canvas: Element is not a design element";
                            setEditingElement(nullptr);
                        }
                    } else {
                        qDebug() << "Canvas: Element is not visual";
                        setEditingElement(nullptr);
                    }
                } else {
                    qDebug() << "Canvas: Not exactly one element selected";
                    setEditingElement(nullptr);
                }
            }
            
            // Load scripts into ElementModel
            loadScriptsIntoElementModel();
        } else if (viewMode == "design") {
            // Save any changes back to scripts (either editing element's or canvas's)
            saveElementModelToScripts();
            
            // Clear editing element when switching back to design mode
            setEditingElement(nullptr);
            
            // Clear script elements from ElementModel
            clearScriptElementsFromModel();
        }
        
        // Clear selection when switching view modes
        if (m_selectionManager) {
            m_selectionManager->clearSelection();
        }
        
        // Update the canvas controller's canvas type
        if (m_controller) {
            m_controller->setCanvasType(viewMode);
        }
        
        // Reset to select mode
        if (m_controller) {
            m_controller->setMode("select");
        }
        
        emit viewModeChanged();
    }
}

void Canvas::initialize() {
    // Create the components
    m_elementModel = std::make_unique<ElementModel>(this);
    m_selectionManager = std::make_unique<SelectionManager>(this);
    m_controller = std::make_unique<CanvasController>(this);
    m_scripts = std::make_unique<Scripts>(this);
    
    // Set up the connections
    m_controller->setElementModel(m_elementModel.get());
    m_controller->setSelectionManager(m_selectionManager.get());
    
    // Connect to element model changes to track when nodes/edges are added in script mode
    // We need to ensure they're properly added to the appropriate scripts (either design element's or canvas's)
    connect(m_elementModel.get(), &ElementModel::elementAdded, this, [this](Element* element) {
        if (m_viewMode == "script") {
            Scripts* targetScripts = activeScripts();  // This returns either editing element's scripts or canvas scripts
            if (targetScripts) {
                // When a new node/edge is created in script mode, it needs to be added to scripts
                if (Node* node = qobject_cast<Node*>(element)) {
                    // Check if it's not already in scripts (to avoid duplicates during load)
                    if (!targetScripts->getNode(node->getId())) {
                        qDebug() << "Canvas: Adding node" << node->getId() << "to" 
                                 << (m_editingElement ? "editing element's" : "canvas") << "scripts";
                        // The node was created by the controller, we need to transfer it to scripts
                        // Set the parent to scripts so it gets proper ownership
                        node->setParent(targetScripts);
                        targetScripts->addNode(node);
                    }
                } else if (Edge* edge = qobject_cast<Edge*>(element)) {
                    if (!targetScripts->getEdge(edge->getId())) {
                        qDebug() << "Canvas: Adding edge" << edge->getId() << "to" 
                                 << (m_editingElement ? "editing element's" : "canvas") << "scripts";
                        // Transfer ownership to scripts
                        edge->setParent(targetScripts);
                        targetScripts->addEdge(edge);
                    }
                }
            }
        }
    });
    
    // Also handle removal of elements
    connect(m_elementModel.get(), &ElementModel::elementRemoved, this, [this](const QString& elementId) {
        if (m_viewMode == "script") {
            Scripts* targetScripts = activeScripts();  // This returns either editing element's scripts or canvas scripts
            if (targetScripts) {
                // Remove from scripts when removed from model
                if (Node* node = targetScripts->getNode(elementId)) {
                    qDebug() << "Canvas: Removing node" << elementId << "from" 
                             << (m_editingElement ? "editing element's" : "canvas") << "scripts";
                    targetScripts->removeNode(node);
                } else if (Edge* edge = targetScripts->getEdge(elementId)) {
                    qDebug() << "Canvas: Removing edge" << elementId << "from" 
                             << (m_editingElement ? "editing element's" : "canvas") << "scripts";
                    targetScripts->removeEdge(edge);
                }
            }
        }
    });
}

DesignElement* Canvas::editingElement() const {
    return m_editingElement;
}

Scripts* Canvas::activeScripts() const {
    // If we're editing a design element's scripts, return those
    // Otherwise return the canvas's global scripts
    if (m_editingElement && m_editingElement->scripts()) {
        return m_editingElement->scripts();
    }
    return m_scripts.get();
}

void Canvas::setEditingElement(DesignElement* element) {
    if (m_editingElement != element) {
        m_editingElement = element;
        emit editingElementChanged();
        updateActiveScripts();
    }
}

void Canvas::updateActiveScripts() {
    emit activeScriptsChanged();
}

void Canvas::loadScriptsIntoElementModel() {
    if (!m_elementModel) return;
    
    qDebug() << "Canvas::loadScriptsIntoElementModel - Starting";
    
    // Clear existing script elements from the model first
    clearScriptElementsFromModel();
    
    // Get the active scripts
    Scripts* scripts = activeScripts();
    if (!scripts) {
        qDebug() << "Canvas::loadScriptsIntoElementModel - No active scripts";
        return;
    }
    
    // Important: Scripts owns the nodes/edges with unique_ptr
    // We need to just add references to ElementModel without transferring ownership
    // ElementModel should NOT delete these elements
    
    // Add all nodes to the element model
    auto nodes = scripts->getAllNodes();
    qDebug() << "Canvas::loadScriptsIntoElementModel - Loading" << nodes.size() << "nodes";
    for (Node* node : nodes) {
        if (node) {
            // Add to model but Scripts retains ownership
            m_elementModel->addElement(node);
        }
    }
    
    // Add all edges to the element model
    auto edges = scripts->getAllEdges();
    qDebug() << "Canvas::loadScriptsIntoElementModel - Loading" << edges.size() << "edges";
    for (Edge* edge : edges) {
        if (edge) {
            // Add to model but Scripts retains ownership
            m_elementModel->addElement(edge);
        }
    }
}

void Canvas::saveElementModelToScripts() {
    if (!m_elementModel) return;
    
    // The nodes/edges are already in the scripts
    // They were added/removed dynamically via the connect() signals
    // No additional action needed here
    
    qDebug() << "Canvas::saveElementModelToScripts - Scripts already synced via signals";
}

void Canvas::clearScriptElementsFromModel() {
    if (!m_elementModel) return;
    
    // Get all elements and remove nodes and edges
    auto elements = m_elementModel->getAllElements();
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
        // Find the index and remove without triggering deleteLater
        int index = -1;
        auto allElements = m_elementModel->getAllElements();
        for (int i = 0; i < allElements.size(); ++i) {
            if (allElements[i] == element) {
                index = i;
                break;
            }
        }
        
        if (index >= 0) {
            // Remove from model's internal list without deleting
            // We'll need to add a method to ElementModel for this
            m_elementModel->removeElementWithoutDelete(element);
        }
    }
}