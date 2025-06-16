#include "Canvas.h"
#include "CanvasController.h"
#include "SelectionManager.h"
#include "ElementModel.h"
#include "Scripts.h"

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
}