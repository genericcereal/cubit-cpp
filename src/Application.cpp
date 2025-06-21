#include "Application.h"
#include "Project.h"
#include "SelectionManager.h"
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
    // Clear all canvases before destruction to ensure proper cleanup order
    m_canvases.clear();
    s_instance = nullptr;
}

Application* Application::instance() {
    return s_instance;
}

QString Application::createCanvas(const QString& name) {
    QString canvasId = generateCanvasId();
    QString canvasName = name.isEmpty() ? QString("Canvas %1").arg(m_canvases.size() + 1) : name;
    
    auto canvas = std::make_unique<Project>(canvasId, canvasName, this);
    canvas->initialize();
    
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
        [&canvasId](const std::unique_ptr<Project>& c) { return c->id() == canvasId; });
    
    if (it != m_canvases.end()) {
        bool wasActive = (m_activeCanvasId == canvasId);
        
        m_canvases.erase(it);
        
        emit canvasListChanged();
        emit canvasRemoved(canvasId);
        
        // If the removed canvas was active, switch to another
        if (wasActive) {
            if (!m_canvases.empty()) {
                setActiveCanvasId(m_canvases.front()->id());
            } else {
                m_activeCanvasId.clear();
                emit activeCanvasIdChanged();
                emit activeCanvasChanged();
            }
        }
    }
}

void Application::switchToCanvas(const QString& canvasId) {
    setActiveCanvasId(canvasId);
}

QString Application::getCanvasName(const QString& canvasId) const {
    const Project* canvas = findCanvas(canvasId);
    return canvas ? canvas->name() : QString();
}

void Application::renameCanvas(const QString& canvasId, const QString& newName) {
    Project* canvas = findCanvas(canvasId);
    if (canvas && !newName.isEmpty()) {
        canvas->setName(newName);
        emit canvasListChanged();
    }
}

QString Application::activeCanvasId() const {
    return m_activeCanvasId;
}

Project* Application::activeCanvas() const {
    return const_cast<Project*>(findCanvas(m_activeCanvasId));
}

QStringList Application::canvasIds() const {
    QStringList ids;
    for (const auto& canvas : m_canvases) {
        ids.append(canvas->id());
    }
    return ids;
}

QStringList Application::canvasNames() const {
    QStringList names;
    for (const auto& canvas : m_canvases) {
        names.append(canvas->name());
    }
    return names;
}

void Application::setActiveCanvasId(const QString& canvasId) {
    if (m_activeCanvasId != canvasId && findCanvas(canvasId)) {
        m_activeCanvasId = canvasId;
        
        // Clear selection when switching canvases
        Project* canvas = findCanvas(canvasId);
        if (canvas && canvas->selectionManager()) {
            canvas->selectionManager()->clearSelection();
        }
        
        emit activeCanvasIdChanged();
        emit activeCanvasChanged();
    }
}


Project* Application::findCanvas(const QString& canvasId) {
    auto it = std::find_if(m_canvases.begin(), m_canvases.end(),
        [&canvasId](const std::unique_ptr<Project>& c) { return c->id() == canvasId; });
    return (it != m_canvases.end()) ? it->get() : nullptr;
}

const Project* Application::findCanvas(const QString& canvasId) const {
    auto it = std::find_if(m_canvases.begin(), m_canvases.end(),
        [&canvasId](const std::unique_ptr<Project>& c) { return c->id() == canvasId; });
    return (it != m_canvases.end()) ? it->get() : nullptr;
}

QString Application::generateCanvasId() const {
    return UniqueIdGenerator::generate16DigitId();
}


Panels* Application::panels() const {
    return m_panels.get();
}