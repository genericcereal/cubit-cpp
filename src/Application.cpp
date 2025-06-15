#include "Application.h"
#include "CanvasController.h"
#include "SelectionManager.h"
#include "ElementModel.h"
#include "UniqueIdGenerator.h"
#include <QDebug>

Application* Application::s_instance = nullptr;

Application::Application(QObject *parent)
    : QObject(parent)
{
    s_instance = this;
    
    // Create panels manager
    m_panels = std::make_unique<Panels>(this);
    
    // Create initial canvas
    createCanvas("Canvas 1");
}

Application::~Application() {
    s_instance = nullptr;
}

Application* Application::instance() {
    return s_instance;
}

QString Application::createCanvas(const QString& name) {
    auto canvas = std::make_unique<Canvas>();
    canvas->id = generateCanvasId();
    canvas->name = name.isEmpty() ? QString("Canvas %1").arg(m_canvases.size() + 1) : name;
    canvas->currentViewMode = "design"; // Default to design view
    
    // Create components
    canvas->controller = std::make_unique<CanvasController>();
    canvas->selectionManager = std::make_unique<SelectionManager>();
    canvas->elementModel = std::make_unique<ElementModel>();
    
    initializeCanvas(canvas.get());
    
    QString canvasId = canvas->id;
    m_canvases.push_back(std::move(canvas));
    
    // If this is the first canvas or no active canvas, make it active
    if (m_activeCanvasId.isEmpty()) {
        setActiveCanvasId(canvasId);
    }
    
    emit canvasListChanged();
    emit canvasCreated(canvasId);
    
    return canvasId;
}

void Application::removeCanvas(const QString& canvasId) {
    auto it = std::find_if(m_canvases.begin(), m_canvases.end(),
        [&canvasId](const std::unique_ptr<Canvas>& c) { return c->id == canvasId; });
    
    if (it != m_canvases.end()) {
        bool wasActive = (m_activeCanvasId == canvasId);
        
        m_canvases.erase(it);
        
        emit canvasListChanged();
        emit canvasRemoved(canvasId);
        
        // If the removed canvas was active, switch to another
        if (wasActive) {
            if (!m_canvases.empty()) {
                setActiveCanvasId(m_canvases.front()->id);
            } else {
                m_activeCanvasId.clear();
                emit activeCanvasIdChanged();
                emit activeControllerChanged();
                emit activeSelectionManagerChanged();
                emit activeElementModelChanged();
            }
        }
    }
}

void Application::switchToCanvas(const QString& canvasId) {
    setActiveCanvasId(canvasId);
}

QString Application::getCanvasName(const QString& canvasId) const {
    const Canvas* canvas = findCanvas(canvasId);
    return canvas ? canvas->name : QString();
}

void Application::renameCanvas(const QString& canvasId, const QString& newName) {
    Canvas* canvas = findCanvas(canvasId);
    if (canvas && !newName.isEmpty()) {
        canvas->name = newName;
        emit canvasListChanged();
    }
}

QString Application::activeCanvasId() const {
    return m_activeCanvasId;
}

CanvasController* Application::activeController() const {
    const Canvas* canvas = findCanvas(m_activeCanvasId);
    return canvas ? canvas->controller.get() : nullptr;
}

SelectionManager* Application::activeSelectionManager() const {
    const Canvas* canvas = findCanvas(m_activeCanvasId);
    return canvas ? canvas->selectionManager.get() : nullptr;
}

ElementModel* Application::activeElementModel() const {
    const Canvas* canvas = findCanvas(m_activeCanvasId);
    return canvas ? canvas->elementModel.get() : nullptr;
}

QString Application::activeCanvasViewMode() const {
    const Canvas* canvas = findCanvas(m_activeCanvasId);
    return canvas ? canvas->currentViewMode : QString("design");
}

QStringList Application::canvasIds() const {
    QStringList ids;
    for (const auto& canvas : m_canvases) {
        ids.append(canvas->id);
    }
    return ids;
}

QStringList Application::canvasNames() const {
    QStringList names;
    for (const auto& canvas : m_canvases) {
        names.append(canvas->name);
    }
    return names;
}

void Application::setActiveCanvasId(const QString& canvasId) {
    if (m_activeCanvasId != canvasId && findCanvas(canvasId)) {
        m_activeCanvasId = canvasId;
        
        // Clear selection when switching canvases
        if (auto* selectionManager = activeSelectionManager()) {
            selectionManager->clearSelection();
        }
        
        emit activeCanvasIdChanged();
        emit activeControllerChanged();
        emit activeSelectionManagerChanged();
        emit activeElementModelChanged();
        emit activeCanvasViewModeChanged();
    }
}

void Application::setActiveCanvasViewMode(const QString& viewMode) {
    Canvas* canvas = findCanvas(m_activeCanvasId);
    if (canvas && canvas->currentViewMode != viewMode) {
        canvas->currentViewMode = viewMode;
        
        // Update the canvas controller's canvas type
        if (canvas->controller) {
            canvas->controller->setCanvasType(viewMode);
        }
        
        // Clear selection when switching view modes
        if (canvas->selectionManager) {
            canvas->selectionManager->clearSelection();
        }
        
        // Reset to select mode
        if (canvas->controller) {
            canvas->controller->setMode("select");
        }
        
        emit activeCanvasViewModeChanged();
    }
}

Canvas* Application::findCanvas(const QString& canvasId) {
    auto it = std::find_if(m_canvases.begin(), m_canvases.end(),
        [&canvasId](const std::unique_ptr<Canvas>& c) { return c->id == canvasId; });
    return (it != m_canvases.end()) ? it->get() : nullptr;
}

const Canvas* Application::findCanvas(const QString& canvasId) const {
    auto it = std::find_if(m_canvases.begin(), m_canvases.end(),
        [&canvasId](const std::unique_ptr<Canvas>& c) { return c->id == canvasId; });
    return (it != m_canvases.end()) ? it->get() : nullptr;
}

QString Application::generateCanvasId() const {
    return UniqueIdGenerator::generate16DigitId();
}

void Application::initializeCanvas(Canvas* canvas) {
    if (!canvas) return;
    
    // Set up connections between components
    canvas->controller->setElementModel(canvas->elementModel.get());
    canvas->controller->setSelectionManager(canvas->selectionManager.get());
    canvas->controller->setCanvasType(canvas->currentViewMode);
}

Panels* Application::panels() const {
    return m_panels.get();
}