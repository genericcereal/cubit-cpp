#include "Project.h"
#include "CanvasController.h"
#include "DesignCanvas.h"
#include "SelectionManager.h"
#include "ElementModel.h"
#include "Scripts.h"
#include "ScriptExecutor.h"
#include "Node.h"
#include "Edge.h"
#include "Component.h"
#include "PrototypeController.h"
#include "Application.h"
#include "StreamingAIClient.h"
#include "ConsoleMessageRepository.h"
#include "AuthenticationManager.h"
#include "PlatformConfig.h"
#include "HitTestService.h"
#include "CanvasContext.h"
#include "contexts/MainCanvasContext.h"
#include "contexts/VariantCanvasContext.h"
#include "contexts/GlobalElementsContext.h"
#include "contexts/ScriptCanvasContext.h"

Project::Project(const QString& id, const QString& name, QObject *parent)
    : QObject(parent)
    , m_name(name.isEmpty() ? "Untitled Canvas" : name)
    , m_id(id)
    , m_viewMode("design")
{
    // Create default main canvas context
    m_currentContext = std::make_unique<MainCanvasContext>(this);
    // Create per-project console
    m_console = std::make_unique<ConsoleMessageRepository>(this);
    
    // Connect to console repository for AI commands
    connect(m_console.get(), &ConsoleMessageRepository::aiCommandReceived,
            this, &Project::onAICommandReceived, Qt::UniqueConnection);
    
    // Connect to console repository for AI continuation responses
    connect(m_console.get(), &ConsoleMessageRepository::aiContinuationResponse,
            this, &Project::onAIContinuationResponse, Qt::UniqueConnection);
    
    // Connect to console repository for AI mode disabled
    connect(m_console.get(), &ConsoleMessageRepository::aiModeDisabled,
            this, &Project::onAIModeDisabled, Qt::UniqueConnection);
}

Project::~Project() {
    // Clear elements before destroying other components to avoid dangling pointers
    if (m_elementModel) {
        m_elementModel->clear();
    }
}

CanvasController* Project::controller() const {
    if (!m_controller) {
        qWarning() << "Project::controller - Controller is null for project" << m_id;
    }
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

PrototypeController* Project::prototypeController() const {
    return m_prototypeController.get();
}

ConsoleMessageRepository* Project::console() const {
    return m_console.get();
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

CanvasContext* Project::currentContext() const {
    return m_currentContext.get();
}

QQmlListProperty<PlatformConfig> Project::platforms() {
    return QQmlListProperty<PlatformConfig>(
        this, this,
        &Project::appendPlatform,
        &Project::platformCount,
        &Project::platformAt,
        &Project::clearPlatforms
    );
}

void Project::addPlatform(const QString& platformName) {
    // Check if platform already exists
    for (const auto& platform : m_platforms) {
        if (platform->name() == platformName) {
            qWarning() << "Platform" << platformName << "already exists in project";
            return;
        }
    }
    
    // Create new platform config
    auto platformConfig = std::unique_ptr<PlatformConfig>(PlatformConfig::create(platformName, this));
    if (platformConfig) {
        // Connect property sync for all global elements
        platformConfig->connectAllGlobalElementsPropertySync(m_elementModel.get());
        
        m_platforms.push_back(std::move(platformConfig));
        emit platformsChanged();
    }
}

void Project::removePlatform(const QString& platformName) {
    auto it = std::find_if(m_platforms.begin(), m_platforms.end(),
        [&platformName](const std::unique_ptr<PlatformConfig>& platform) {
            return platform->name() == platformName;
        }
    );
    
    if (it != m_platforms.end()) {
        m_platforms.erase(it);
        emit platformsChanged();
    }
}

PlatformConfig* Project::getPlatform(const QString& platformName) const {
    auto it = std::find_if(m_platforms.begin(), m_platforms.end(),
        [&platformName](const std::unique_ptr<PlatformConfig>& platform) {
            return platform->name() == platformName;
        }
    );
    
    return (it != m_platforms.end()) ? it->get() : nullptr;
}

QList<PlatformConfig*> Project::getAllPlatforms() const {
    QList<PlatformConfig*> result;
    for (const auto& platform : m_platforms) {
        result.append(platform.get());
    }
    return result;
}

Scripts* Project::getPlatformScripts(const QString& platformName) const {
    PlatformConfig* platform = getPlatform(platformName);
    return platform ? platform->scripts() : nullptr;
}

void Project::setName(const QString& name) {
    if (m_name != name) {
        m_name = name;
        emit nameChanged();
    }
}

void Project::setViewMode(const QString& viewMode) {
    if (m_viewMode != viewMode) {
        QString previousMode = m_viewMode;
        
        // Emit signal to save viewport state before changing mode
        if (previousMode == "design" || previousMode == "variant") {
            emit viewportStateShouldBeSaved();
        }
        
        m_viewMode = viewMode;
        
        // Create and set the appropriate context
        createContextForViewMode(viewMode);
        
        // Clear selection when switching view modes
        if (m_selectionManager) {
            m_selectionManager->clearSelection();
        }
        
        // Reset to select mode
        if (m_controller) {
            m_controller->setMode(CanvasController::Mode::Select);
        }
        
        // Emit signal to restore viewport state for design/variant modes
        if (viewMode == "design" || viewMode == "variant" || viewMode == "globalElements") {
            emit viewportStateShouldBeRestored();
        }
        
        emit viewModeChanged();
    }
}

void Project::setCanvasContext(std::unique_ptr<CanvasContext> context) {
    // IMPORTANT: Clear the context pointer from HitTestService BEFORE destroying the old context
    // This prevents dangling pointer access during destruction
    if (m_controller && m_controller->hitTestService()) {
        m_controller->hitTestService()->setCanvasContext(nullptr);
    }
    
    // Set flag to indicate we're switching contexts
    if (m_elementModel) {
        m_elementModel->setProperty("_switchingContext", true);
    }
    
    // Deactivate current context
    if (m_currentContext && m_elementModel) {
        m_currentContext->deactivateContext(m_elementModel.get());
    }
    
    // Set new context
    m_currentContext = std::move(context);
    
    // Activate new context
    if (m_currentContext && m_elementModel) {
        m_currentContext->activateContext(m_elementModel.get());
        
        // Clear the switching context flag
        m_elementModel->setProperty("_switchingContext", false);
        
        // Update canvas controller type
        if (m_controller) {
            m_controller->setCanvasType(m_currentContext->getCanvasType());
        }
        
        // Configure hit testing if available
        if (m_controller && m_controller->hitTestService()) {
            m_controller->hitTestService()->setCanvasContext(m_currentContext.get());
            m_currentContext->configureHitTestService(m_controller->hitTestService());
        }
        
        // Rebuild spatial index after context change
        if (m_controller && m_controller->hitTestService()) {
            m_controller->hitTestService()->rebuildSpatialIndex();
        }
    }
    
    emit currentContextChanged();
}

// QML list property helpers
void Project::appendPlatform(QQmlListProperty<PlatformConfig>* list, PlatformConfig* platform) {
    Q_UNUSED(list)
    Q_UNUSED(platform)
    // Not implemented - use addPlatform() instead
}

qsizetype Project::platformCount(QQmlListProperty<PlatformConfig>* list) {
    Project* project = qobject_cast<Project*>(list->object);
    return project ? project->m_platforms.size() : 0;
}

PlatformConfig* Project::platformAt(QQmlListProperty<PlatformConfig>* list, qsizetype index) {
    Project* project = qobject_cast<Project*>(list->object);
    if (project && index >= 0 && index < static_cast<qsizetype>(project->m_platforms.size())) {
        return project->m_platforms[index].get();
    }
    return nullptr;
}

void Project::clearPlatforms(QQmlListProperty<PlatformConfig>* list) {
    Project* project = qobject_cast<Project*>(list->object);
    if (project) {
        project->m_platforms.clear();
        emit project->platformsChanged();
    }
}

void Project::setId(const QString& id) {
    if (m_id != id) {
        m_id = id;
        // Note: We don't emit a signal here because id is a CONSTANT property in QML
        // The ID change happens during API sync before the project is exposed to QML
    }
}

void Project::initialize() {
    // Initialize project components
    
    // Create the components that don't have dependencies first
    m_elementModel = std::make_unique<ElementModel>(this);
    m_selectionManager = std::make_unique<SelectionManager>(this);
    m_scripts = std::make_unique<Scripts>(this);
    m_scriptExecutor = std::make_unique<ScriptExecutor>(this);
    
    // Create the controller with its required dependencies
    // For now, always create DesignCanvas as it handles both design and variant modes
    m_controller = std::make_unique<DesignCanvas>(*m_elementModel, *m_selectionManager, this);
    
    // Controller created and initialized
    
    // Set initial canvas context on hit test service
    if (m_controller && m_controller->hitTestService() && m_currentContext) {
        m_controller->hitTestService()->setCanvasContext(m_currentContext.get());
    }
    
    // Create the prototype controller
    m_prototypeController = std::make_unique<PrototypeController>(*m_elementModel, *m_selectionManager, this);
    
    // Set the canvas controller on the prototype controller
    m_prototypeController->setCanvasController(m_controller.get());
    
    // Connect prototype controller's isPrototyping to design canvas disable flags
    if (DesignCanvas* designCanvas = qobject_cast<DesignCanvas*>(m_controller.get())) {
        connect(m_prototypeController.get(), &PrototypeController::isPrototypingChanged,
                this, [designCanvas, this]() {
                    bool isPrototyping = m_prototypeController->isPrototyping();
                    designCanvas->setIsDesignControlsResizingDisabled(isPrototyping);
                    designCanvas->setIsDesignControlsMovementDisabled(isPrototyping);
                });
        
        // Set initial state in case prototyping is already active
        bool isPrototyping = m_prototypeController->isPrototyping();
        designCanvas->setIsDesignControlsResizingDisabled(isPrototyping);
        designCanvas->setIsDesignControlsMovementDisabled(isPrototyping);
    }
    
    // Set up script executor
    m_scriptExecutor->setScripts(m_scripts.get());
    m_scriptExecutor->setElementModel(m_elementModel.get());
    m_scriptExecutor->setCanvasController(m_controller.get());
    
    // Connect to element model changes to track when nodes/edges are added in script mode
    // The ScriptCanvasContext handles the script synchronization, but we still need to
    // ensure proper ownership when elements are created by the controller
    connect(m_elementModel.get(), &ElementModel::elementAdded, this, [this](Element* element) {
        if (m_viewMode == "script" && m_currentContext && m_currentContext->contextType() == "script") {
            Scripts* targetScripts = activeScripts();
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
        if (m_viewMode == "script" && m_currentContext && m_currentContext->contextType() == "script") {
            // Don't remove from Scripts if we're just switching contexts
            if (m_elementModel->property("_switchingContext").toBool()) {
                return;
            }
            
            Scripts* targetScripts = activeScripts();
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
        // Check if it's a PlatformConfig
        else if (PlatformConfig* platform = qobject_cast<PlatformConfig*>(m_editingElement)) {
            if (platform->scripts()) {
                return platform->scripts();
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

void Project::setEditingPlatform(PlatformConfig* platform, const QString& viewMode) {
    if (m_editingElement != platform) {
        m_editingElement = platform;
        emit editingElementChanged();
        updateActiveScripts();
        
        // Update the canvas controller's hit test service
        if (m_controller) {
            // For globalElements mode, we need to pass the platform to the hit test service
            m_controller->setEditingElement(platform);
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

void Project::createContextForViewMode(const QString& mode) {
    std::unique_ptr<CanvasContext> newContext;
    
    if (mode == "design") {
        newContext = std::make_unique<MainCanvasContext>(this);
        
        // Ensure property sync is connected for all platforms
        for (const auto& platform : m_platforms) {
            if (platform) {
                platform->connectAllGlobalElementsPropertySync(m_elementModel.get());
            }
        }
    } else if (mode == "variant") {
        if (m_editingElement) {
            newContext = std::make_unique<VariantCanvasContext>(m_editingElement, this);
            qDebug() << "Creating variant context for element:" << m_editingElement;
        } else {
            // Fall back to main canvas if no editing element
            newContext = std::make_unique<MainCanvasContext>(this);
            qWarning() << "No editing element for variant mode, falling back to main canvas";
        }
    } else if (mode == "globalElements") {
        if (PlatformConfig* platform = qobject_cast<PlatformConfig*>(m_editingElement)) {
            newContext = std::make_unique<GlobalElementsContext>(platform, this);
        } else {
            // Fall back to main canvas if no platform
            newContext = std::make_unique<MainCanvasContext>(this);
            qWarning() << "No platform for globalElements mode, falling back to main canvas";
        }
    } else if (mode == "script") {
        Scripts* targetScripts = activeScripts();
        if (targetScripts) {
            newContext = std::make_unique<ScriptCanvasContext>(targetScripts, m_editingElement, this);
        } else {
            // This shouldn't happen, but handle gracefully
            newContext = std::make_unique<ScriptCanvasContext>(m_scripts.get(), nullptr, this);
            qWarning() << "No active scripts found, using canvas scripts";
        }
    } else {
        // Default to main canvas for unknown modes
        newContext = std::make_unique<MainCanvasContext>(this);
    }
    
    // Set the new context
    setCanvasContext(std::move(newContext));
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

void Project::handleAICommand(const QString& prompt) {
    // Get the Application instance
    Application* app = Application::instance();
    if (!app) {
        m_console->addError("Application instance not available.");
        return;
    }
    
    // Get the authentication manager from the application
    AuthenticationManager* authManager = app->authManager();
    
    if (!authManager || !authManager->isAuthenticated()) {
        m_console->addError("You must be authenticated to use the AI assistant. Please log in first.");
        return;
    }
    
    // Create AI client if not exists
    if (!m_aiClient) {
        m_aiClient = std::make_unique<StreamingAIClient>(authManager, app, this);
    }
    
    // Set this project as the target
    m_aiClient->setTargetProject(this);
    
    // Send the prompt to the AI
    m_aiClient->sendMessage(prompt);
}

void Project::handleAIContinuationResponse(bool accepted, const QString& feedback) {
    if (!m_aiClient) {
        m_console->addError("AI client is not initialized.");
        return;
    }
    
    // Forward the response to the AI client
    m_aiClient->handleUserContinuationResponse(accepted, feedback);
}

void Project::onAICommandReceived(const QString& prompt, QObject* project) {
    // Only handle if this is the target project
    if (project == this) {
        handleAICommand(prompt);
    }
}

void Project::onAIContinuationResponse(bool accepted, const QString& feedback, QObject* project) {
    // Only handle if this is the target project
    if (project == this) {
        handleAIContinuationResponse(accepted, feedback);
    }
}

void Project::onAIModeDisabled() {
    // Clear the AI conversation when AI mode is disabled
    if (m_aiClient) {
        m_aiClient->clearConversation();
    }
}