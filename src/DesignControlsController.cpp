#include "DesignControlsController.h"
#include "Application.h"
#include "Project.h"
#include "DesignCanvas.h"
#include "SelectionManager.h"
#include "ElementModel.h"
#include "PrototypeController.h"
#include "Element.h"
#include "CanvasElement.h"
#include "CanvasController.h"
#include <QMetaObject>
#include <QQmlEngine>
#include <QObject>

// Default constructor for QML
DesignControlsController::DesignControlsController(QObject* parent)
    : QObject(parent)
{
    // Controller will be initialized when project is set
}

// Constructor for per-window instance
DesignControlsController::DesignControlsController(Project* project, QObject* parent)
    : QObject(parent), m_project(project)
{
    // For per-window instances, connect directly to the project's canvas
    if (m_project) {
        // No need to watch for canvas changes as each window has its own controller
        connectToActiveCanvas();
        updateCache();
    }
}

// Legacy constructor for global instance (to be removed)
DesignControlsController::DesignControlsController(Application* app, QObject* parent)
    : QObject(parent), m_application(app)
{
    // Initialize connections
    connectToActiveCanvas();
    updateCache();
}

void DesignControlsController::setProject(Project* project)
{
    if (m_project == project) return;
    
    // Disconnect from previous project
    if (m_project) {
        disconnectFromActiveCanvas();
    }
    
    m_project = project;
    emit projectChanged();
    
    // Connect to new project
    if (m_project) {
        connectToActiveCanvas();
        // Force cache update and signal emission by resetting cached values
        m_cachedIsResizingEnabled = false;
        m_cachedIsMovementEnabled = false;
        updateCache();
    }
}

bool DesignControlsController::isResizingEnabled() const
{
    DesignCanvas* canvas = getActiveDesignCanvas();
    if (!canvas) return false;
    
    // Resizing is disabled if:
    // 1. Explicitly disabled in DesignCanvas
    // 2. Any text is being edited
    // 3. Currently animating
    // 4. Currently prototyping
    return !canvas->isDesignControlsResizingDisabled() &&
           !isAnyTextEditing() &&
           !isAnimating() &&
           !isPrototyping();
}

bool DesignControlsController::isMovementEnabled() const
{
    DesignCanvas* canvas = getActiveDesignCanvas();
    if (!canvas) return false;
    
    // Movement is disabled if:
    // 1. Explicitly disabled in DesignCanvas
    // 2. Any text is being edited
    // 3. Currently animating
    // 4. Currently prototyping
    return !canvas->isDesignControlsMovementDisabled() &&
           !isAnyTextEditing() &&
           !isAnimating() &&
           !isPrototyping();
}

bool DesignControlsController::isAnyTextEditing() const
{
    SelectionManager* selectionManager = getActiveSelectionManager();
    if (!selectionManager) return false;
    
    // Check if any selected element is a text element that is being edited
    const auto& selectedElements = selectionManager->selectedElements();
    for (Element* element : selectedElements) {
        if (!element) continue;
        
        // Check if element is a text-based element with isEditing property
        if (element->getTypeName() == "Text" || 
            element->getTypeName() == "TextComponentVariant" ||
            element->getTypeName() == "TextComponentInstance") {
            
            // Check if element has isEditing property and it's true
            QVariant editingValue = element->property("isEditing");
            if (editingValue.isValid() && editingValue.toBool()) {
                return true;
            }
        }
    }
    
    return false;
}

bool DesignControlsController::isAnimating() const
{
    DesignCanvas* canvas = getActiveDesignCanvas();
    return canvas ? canvas->isAnimating() : false;
}

bool DesignControlsController::isPrototyping() const
{
    if (!m_project) return false;
    
    Project* project = m_project;
    
    PrototypeController* prototypeController = project->prototypeController();
    if (!prototypeController) return false;
    
    // Check if prototyping is active
    QVariant prototypingValue = prototypeController->property("isPrototyping");
    return prototypingValue.isValid() && prototypingValue.toBool();
}

QPointF DesignControlsController::mapToCanvas(QObject* parent, const QPointF& point, qreal zoom, QObject* flickable, const QPointF& canvasMin) const
{
    if (!parent || !flickable) return QPointF();
    
    // Get contentX and contentY from flickable
    qreal contentX = flickable->property("contentX").toReal();
    qreal contentY = flickable->property("contentY").toReal();
    
    // Convert from viewport coordinates to canvas coordinates
    return QPointF(
        (contentX + point.x()) / zoom + canvasMin.x(),
        (contentY + point.y()) / zoom + canvasMin.y()
    );
}


void DesignControlsController::onSelectionChanged()
{
    updateCache();
}

void DesignControlsController::onDesignControlsStateChanged()
{
    updateCache();
}

DesignCanvas* DesignControlsController::getActiveDesignCanvas() const
{
    Project* project = m_project;
    
    if (!project) return nullptr;
    return qobject_cast<DesignCanvas*>(project->controller());
}

SelectionManager* DesignControlsController::getActiveSelectionManager() const
{
    Project* project = m_project;
    
    if (!project) return nullptr;
    return project->selectionManager();
}

ElementModel* DesignControlsController::getActiveElementModel() const
{
    Project* project = m_project;
    
    if (!project) return nullptr;
    return project->elementModel();
}

void DesignControlsController::updateCache()
{
    bool newIsResizingEnabled = isResizingEnabled();
    bool newIsMovementEnabled = isMovementEnabled();
    bool newIsAnyTextEditing = isAnyTextEditing();
    bool newIsAnimating = isAnimating();
    bool newIsPrototyping = isPrototyping();
    
    // Emit signals only if values changed
    if (m_cachedIsResizingEnabled != newIsResizingEnabled) {
        m_cachedIsResizingEnabled = newIsResizingEnabled;
        emit isResizingEnabledChanged();
    }
    
    if (m_cachedIsMovementEnabled != newIsMovementEnabled) {
        m_cachedIsMovementEnabled = newIsMovementEnabled;
        emit isMovementEnabledChanged();
    }
    
    if (m_cachedIsAnyTextEditing != newIsAnyTextEditing) {
        m_cachedIsAnyTextEditing = newIsAnyTextEditing;
        emit isAnyTextEditingChanged();
    }
    
    if (m_cachedIsAnimating != newIsAnimating) {
        m_cachedIsAnimating = newIsAnimating;
        emit isAnimatingChanged();
    }
    
    if (m_cachedIsPrototyping != newIsPrototyping) {
        m_cachedIsPrototyping = newIsPrototyping;
        emit isPrototypingChanged();
    }
}

void DesignControlsController::connectToActiveCanvas()
{
    DesignCanvas* canvas = getActiveDesignCanvas();
    SelectionManager* selectionManager = getActiveSelectionManager();
    
    if (canvas) {
        // Connect to canvas state changes
        m_resizingDisabledConnection = connect(canvas, &DesignCanvas::isDesignControlsResizingDisabledChanged, 
                                              this, &DesignControlsController::onDesignControlsStateChanged, Qt::UniqueConnection);
        m_movementDisabledConnection = connect(canvas, &DesignCanvas::isDesignControlsMovementDisabledChanged, 
                                              this, &DesignControlsController::onDesignControlsStateChanged, Qt::UniqueConnection);
        m_animatingConnection = connect(canvas, &DesignCanvas::isAnimatingChanged, 
                                       this, &DesignControlsController::onDesignControlsStateChanged, Qt::UniqueConnection);
    }
    
    if (selectionManager) {
        // Connect to selection changes
        m_selectionConnection = connect(selectionManager, &SelectionManager::selectionChanged, 
                                       this, &DesignControlsController::onSelectionChanged, Qt::UniqueConnection);
    }
    
    Project* project = m_project;
    
    if (project) {
        PrototypeController* prototypeController = project->prototypeController();
        if (prototypeController) {
            // Connect to prototype state changes
            m_prototypingConnection = connect(prototypeController, SIGNAL(isPrototypingChanged()), 
                                            this, SLOT(onDesignControlsStateChanged()), Qt::UniqueConnection);
        }
    }
}

void DesignControlsController::disconnectFromActiveCanvas()
{
    // Disconnect all existing connections
    if (m_canvasConnection) {
        disconnect(m_canvasConnection);
        m_canvasConnection = QMetaObject::Connection();
    }
    if (m_selectionConnection) {
        disconnect(m_selectionConnection);
        m_selectionConnection = QMetaObject::Connection();
    }
    if (m_resizingDisabledConnection) {
        disconnect(m_resizingDisabledConnection);
        m_resizingDisabledConnection = QMetaObject::Connection();
    }
    if (m_movementDisabledConnection) {
        disconnect(m_movementDisabledConnection);
        m_movementDisabledConnection = QMetaObject::Connection();
    }
    if (m_animatingConnection) {
        disconnect(m_animatingConnection);
        m_animatingConnection = QMetaObject::Connection();
    }
    if (m_prototypingConnection) {
        disconnect(m_prototypingConnection);
        m_prototypingConnection = QMetaObject::Connection();
    }
}

void DesignControlsController::startDragOperation()
{
    // Clear previous state
    m_dragStartSelectedElements.clear();
    m_dragStartElementPositions.clear();
    m_dragStartElementSizes.clear();
    
    // Get active selection manager
    SelectionManager* selectionManager = getActiveSelectionManager();
    if (!selectionManager) {
        qDebug() << "DesignControlsController::startDragOperation - No active selection manager";
        return;
    }
    
    // Store initial state of all selected elements
    auto selectedElements = selectionManager->selectedElements();
    for (Element* element : selectedElements) {
        if (element && element->isVisual()) {
            m_dragStartSelectedElements.append(element);
            
            // Store position and size for visual elements
            if (auto canvasElement = qobject_cast<CanvasElement*>(element)) {
                m_dragStartElementPositions[element->getId()] = QPointF(canvasElement->x(), canvasElement->y());
                m_dragStartElementSizes[element->getId()] = QSizeF(canvasElement->width(), canvasElement->height());
                
                qDebug() << "DesignControlsController: Storing state for element" << element->getId() 
                         << "position:" << canvasElement->x() << "," << canvasElement->y()
                         << "size:" << canvasElement->width() << "x" << canvasElement->height();
            }
        }
    }
    
    qDebug() << "DesignControlsController::startDragOperation - Stored state for" << m_dragStartSelectedElements.size() << "elements";
}

void DesignControlsController::endMoveOperation(const QPointF& totalDelta)
{
    qDebug() << "DesignControlsController::endMoveOperation called with delta:" << totalDelta.x() << "," << totalDelta.y();
    
    // Only fire command if there was actual movement
    if (qAbs(totalDelta.x()) < 0.001 && qAbs(totalDelta.y()) < 0.001) {
        qDebug() << "DesignControlsController::endMoveOperation - No significant movement, skipping command";
        return;
    }
    
    if (m_dragStartSelectedElements.isEmpty()) {
        qDebug() << "DesignControlsController::endMoveOperation - No elements stored, skipping command";
        return;
    }
    
    // Get active canvas controller
    DesignCanvas* designCanvas = getActiveDesignCanvas();
    if (!designCanvas) {
        qDebug() << "DesignControlsController::endMoveOperation - No active canvas controller";
        return;
    }
    
    qDebug() << "DesignControlsController::endMoveOperation - Firing move command for" << m_dragStartSelectedElements.size() << "elements";
    designCanvas->moveElements(m_dragStartSelectedElements, totalDelta);
}

void DesignControlsController::endResizeOperation()
{
    qDebug() << "DesignControlsController::endResizeOperation called";
    
    if (m_dragStartSelectedElements.isEmpty()) {
        qDebug() << "DesignControlsController::endResizeOperation - No elements stored, skipping command";
        return;
    }
    
    // Get active canvas controller
    DesignCanvas* designCanvas = getActiveDesignCanvas();
    if (!designCanvas) {
        qDebug() << "DesignControlsController::endResizeOperation - No active canvas controller";
        return;
    }
    
    // Fire resize command for each element that changed size
    for (Element* element : m_dragStartSelectedElements) {
        if (!element || !element->isVisual()) continue;
        
        auto canvasElement = qobject_cast<CanvasElement*>(element);
        if (!canvasElement) continue;
        
        auto oldSize = m_dragStartElementSizes.value(element->getId());
        QSizeF newSize(canvasElement->width(), canvasElement->height());
        
        qDebug() << "DesignControlsController::endResizeOperation - Element" << element->getId()
                 << "oldSize:" << oldSize.width() << "x" << oldSize.height() 
                 << "newSize:" << newSize.width() << "x" << newSize.height();
        
        // Only fire command if size actually changed
        if (qAbs(newSize.width() - oldSize.width()) > 0.001 || 
            qAbs(newSize.height() - oldSize.height()) > 0.001) {
            qDebug() << "DesignControlsController::endResizeOperation - Firing resize command for element" << element->getId();
            designCanvas->resizeElement(canvasElement, oldSize, newSize);
        } else {
            qDebug() << "DesignControlsController::endResizeOperation - Size didn't change enough for element" << element->getId();
        }
    }
}