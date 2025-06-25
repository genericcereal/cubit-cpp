#include "Project.h"
#include "CanvasController.h"
#include "SelectionManager.h"
#include "ElementModel.h"
#include "Scripts.h"
#include "ScriptExecutor.h"
#include "Node.h"
#include "Edge.h"
#include "Component.h"

Project::Project(const QString& id, const QString& name, QObject *parent)
    : QObject(parent)
    , m_name(name.isEmpty() ? "Untitled Canvas" : name)
    , m_id(id)
    , m_viewMode("design")
{
}

Project::~Project() {
    // Clear elements before destroying other components to avoid dangling pointers
    if (m_elementModel) {
        m_elementModel->clear();
    }
}

CanvasController* Project::controller() const {
    return m_controller.get();
}

SelectionManager* Project::selectionManager() const {
    return m_selectionManager.get();
}

ElementModel* Project::elementModel() const {
    return m_elementModel.get();
}

Scripts* Project::scripts() const {
    return m_scripts.get();
}

QString Project::name() const {
    return m_name;
}

QString Project::id() const {
    return m_id;
}

QString Project::viewMode() const {
    return m_viewMode;
}

void Project::setName(const QString& name) {
    if (m_name != name) {
        m_name = name;
        emit nameChanged();
    }
}

void Project::setViewMode(const QString& viewMode) {
    if (m_viewMode != viewMode) {
        m_viewMode = viewMode;
        
        // Handle mode-specific logic
        if (viewMode == "script") {
            // Load scripts into ElementModel
            loadScriptsIntoElementModel();
        } else if (viewMode == "design") {
            // Save any changes back to scripts (either editing element's or canvas's)
            saveElementModelToScripts();
            
            // Clear script elements from ElementModel
            clearScriptElementsFromModel();
        } else if (viewMode == "variant") {
            // Variant mode: similar to design mode but for component variants
            // Save any changes back to scripts if coming from script mode
            if (m_viewMode == "script") {
                saveElementModelToScripts();
                clearScriptElementsFromModel();
            }
            // Log the editing element when switching to variant mode
            qDebug() << "Switching to variant mode, editingElement:" << m_editingElement;
            // Variant canvas will show component variants
        }
        
        // Clear selection when switching view modes
        if (m_selectionManager) {
            m_selectionManager->clearSelection();
        }
        
        // Update the canvas controller's canvas type
        if (m_controller) {
            CanvasController::CanvasType canvasType;
            if (viewMode == "script") {
                canvasType = CanvasController::CanvasType::Script;
            } else if (viewMode == "variant") {
                canvasType = CanvasController::CanvasType::Variant;
            } else {
                canvasType = CanvasController::CanvasType::Design;
            }
            m_controller->setCanvasType(canvasType);
        }
        
        // Reset to select mode
        if (m_controller) {
            m_controller->setMode(CanvasController::Mode::Select);
        }
        
        emit viewModeChanged();
    }
}

void Project::initialize() {
    // Create the components that don't have dependencies first
    m_elementModel = std::make_unique<ElementModel>(this);
    m_selectionManager = std::make_unique<SelectionManager>(this);
    m_scripts = std::make_unique<Scripts>(this);
    m_scriptExecutor = std::make_unique<ScriptExecutor>(this);
    
    // Create the controller with its required dependencies
    m_controller = std::make_unique<CanvasController>(*m_elementModel, *m_selectionManager, this);
    
    // Set up script executor
    m_scriptExecutor->setScripts(m_scripts.get());
    m_scriptExecutor->setElementModel(m_elementModel.get());
    m_scriptExecutor->setCanvasController(m_controller.get());
    
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
                        qDebug() << "Project: Adding node" << node->getId() << "to" 
                                 << (m_editingElement ? "editing element's" : "canvas") << "scripts";
                        // The node was created by the controller, we need to transfer it to scripts
                        // Set the parent to scripts so it gets proper ownership
                        node->setParent(targetScripts);
                        targetScripts->addNode(node);
                    }
                } else if (Edge* edge = qobject_cast<Edge*>(element)) {
                    if (!targetScripts->getEdge(edge->getId())) {
                        qDebug() << "Project: Adding edge" << edge->getId() << "to" 
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
                    qDebug() << "Project: Removing node" << elementId << "from" 
                             << (m_editingElement ? "editing element's" : "canvas") << "scripts";
                    targetScripts->removeNode(node);
                } else if (Edge* edge = targetScripts->getEdge(elementId)) {
                    qDebug() << "Project: Removing edge" << elementId << "from" 
                             << (m_editingElement ? "editing element's" : "canvas") << "scripts";
                    targetScripts->removeEdge(edge);
                }
            }
        }
    });
}

QObject* Project::editingElement() const {
    return m_editingElement;
}

Scripts* Project::activeScripts() const {
    if (m_editingElement) {
        // Check if it's a Component
        if (Component* component = qobject_cast<Component*>(m_editingElement)) {
            if (component->scripts()) {
                return component->scripts();
            }
        }
        // Check if it's a DesignElement
        else if (DesignElement* designElement = qobject_cast<DesignElement*>(m_editingElement)) {
            if (designElement->scripts()) {
                return designElement->scripts();
            }
        }
    }
    // Otherwise return the canvas's global scripts
    return m_scripts.get();
}

void Project::setEditingElement(DesignElement* element, const QString& viewMode) {
    // If switching to script mode and element is null, try to auto-detect from selection
    if (viewMode == "script" && !element && m_selectionManager) {
        auto selectedElements = m_selectionManager->selectedElements();
        qDebug() << "Project: Auto-detecting editing element, selected elements:" << selectedElements.size();
        
        if (selectedElements.size() == 1) {
            auto selectedElement = selectedElements.first();
            
            // Check if it's a Component
            if (Component* component = qobject_cast<Component*>(selectedElement)) {
                qDebug() << "Project: Auto-detected Component:" << component->getId();
                setEditingComponent(component, viewMode);
                return;
            }
            
            // Check if it's a visual design element
            if (selectedElement && selectedElement->isVisual()) {
                CanvasElement* canvasElement = qobject_cast<CanvasElement*>(selectedElement);
                if (canvasElement && canvasElement->isDesignElement()) {
                    element = qobject_cast<DesignElement*>(selectedElement);
                    qDebug() << "Project: Auto-detected editing element:" << (element ? element->getId() : "null");
                }
            }
        }
    }
    
    if (m_editingElement != element) {
        m_editingElement = element;
        emit editingElementChanged();
        updateActiveScripts();
        
        // Update the canvas controller's hit test service
        if (m_controller) {
            m_controller->setEditingElement(m_editingElement);
        }
    }
    
    // If a viewMode is specified, switch to that mode
    if (!viewMode.isEmpty() && m_viewMode != viewMode) {
        setViewMode(viewMode);
    }
}

void Project::setEditingComponent(Component* component, const QString& viewMode) {
    if (m_editingElement != component) {
        m_editingElement = component;
        emit editingElementChanged();
        updateActiveScripts();
        
        // Update the canvas controller's hit test service
        if (m_controller) {
            m_controller->setEditingElement(m_editingElement);
        }
    }
    
    // If a viewMode is specified, switch to that mode
    if (!viewMode.isEmpty() && m_viewMode != viewMode) {
        setViewMode(viewMode);
    }
}

void Project::updateActiveScripts() {
    emit activeScriptsChanged();
}

void Project::loadScriptsIntoElementModel() {
    if (!m_elementModel) return;
    
    qDebug() << "Project::loadScriptsIntoElementModel - Starting";
    
    // Clear existing script elements from the model first
    clearScriptElementsFromModel();
    
    // Get the active scripts
    Scripts* scripts = activeScripts();
    if (!scripts) {
        qDebug() << "Project::loadScriptsIntoElementModel - No active scripts";
        return;
    }
    
    // Important: Scripts owns the nodes/edges with unique_ptr
    // We need to just add references to ElementModel without transferring ownership
    // ElementModel should NOT delete these elements
    
    // Add all nodes to the element model
    auto nodes = scripts->getAllNodes();
    qDebug() << "Project::loadScriptsIntoElementModel - Loading" << nodes.size() << "nodes";
    for (Node* node : nodes) {
        if (node) {
            // Add to model but Scripts retains ownership
            m_elementModel->addElement(node);
        }
    }
    
    // Add all edges to the element model
    auto edges = scripts->getAllEdges();
    qDebug() << "Project::loadScriptsIntoElementModel - Loading" << edges.size() << "edges";
    for (Edge* edge : edges) {
        if (edge) {
            // Add to model but Scripts retains ownership
            m_elementModel->addElement(edge);
        }
    }
}

void Project::saveElementModelToScripts() {
    if (!m_elementModel) return;
    
    // The nodes/edges are already in the scripts
    // They were added/removed dynamically via the connect() signals
    // No additional action needed here
    
    qDebug() << "Project::saveElementModelToScripts - Scripts already synced via signals";
}

void Project::clearScriptElementsFromModel() {
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

void Project::executeScriptEvent(const QString& eventName) {
    if (!m_scriptExecutor) {
        qWarning() << "Project: ScriptExecutor not initialized";
        return;
    }
    
    // Execute event on canvas scripts
    m_scriptExecutor->setScripts(m_scripts.get());
    m_scriptExecutor->executeEvent(eventName);
}