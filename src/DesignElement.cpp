#include "DesignElement.h"
#include "Scripts.h"
#include "PropertyRegistry.h"
#include "ScriptExecutor.h"
#include "Application.h"
#include "Project.h"
#include "Component.h"
#include "ComponentInstanceTemplate.h"
#include "ComponentVariantTemplate.h"
#include "Frame.h"
#include "Text.h"
#include "platforms/web/WebTextInput.h"
#include "UniqueIdGenerator.h"
#include "ElementModel.h"
#include "SelectionManager.h"
#include "PropertyDefinition.h"
#include "CreationManager.h"
#include "CanvasController.h"
#include <QCoreApplication>
#include <QDebug>
#include <QHash>

DesignElement::DesignElement(const QString &id, QObject *parent)
    : CanvasElement(Element::FrameType, id, parent)  // Use FrameType as default, subclasses will override
{
    // Initialize scripts with default configuration
    initializeScripts(false);
}

DesignElement::~DesignElement() {
    // Destructor defined here so Scripts is complete type
}

void DesignElement::initializeScripts(bool isComponentInstance) {
    m_scripts = std::make_unique<Scripts>(this, isComponentInstance);
}

Scripts* DesignElement::scripts() const {
    return m_scripts.get();
}

void DesignElement::executeScriptEvent(const QString& eventName, const QVariantMap& eventData) {
    
    if (!m_scripts) {
        qWarning() << "DesignElement: No scripts available";
        return;
    }
    
    
    // Create a temporary script executor for this element
    ScriptExecutor executor(this);
    executor.setScripts(m_scripts.get());
    
    // Get element model and canvas controller from the parent hierarchy
    ElementModel* elementModel = qobject_cast<ElementModel*>(parent());
    if (elementModel) {
        executor.setElementModel(elementModel);
        
        // Get the Project from the ElementModel's parent
        Project* project = qobject_cast<Project*>(elementModel->parent());
        if (project && project->controller()) {
            executor.setCanvasController(project->controller());
        }
    }
    
    executor.executeEvent(eventName, eventData);
}

void DesignElement::setLeft(qreal value) {
    if (!qFuzzyCompare(m_left, value)) {
        m_left = value;
        emit leftChanged();
        
        // Update position and/or width based on anchor states
        CanvasElement* parent = parentElement();
        if (parent) {
            m_updatingFromAnchors = true;
            
            if (m_leftAnchored && m_rightAnchored) {
                // Both anchored: update position and width
                CanvasElement::setX(parent->x() + value);
                CanvasElement::setWidth(parent->width() - value - m_right);
            } else {
                // Only update position
                CanvasElement::setX(parent->x() + value);
            }
            
            m_updatingFromAnchors = false;
        }
    }
}

void DesignElement::setRight(qreal value) {
    if (!qFuzzyCompare(m_right, value)) {
        m_right = value;
        emit rightChanged();
        
        // Update position and/or width based on anchor states
        CanvasElement* parent = parentElement();
        if (parent) {
            m_updatingFromAnchors = true;
            
            if (m_leftAnchored && m_rightAnchored) {
                // Both anchored: update width only (position stays the same)
                CanvasElement::setWidth(parent->width() - m_left - value);
            } else if (!m_leftAnchored) {
                // Only right anchored: update position
                CanvasElement::setX(parent->x() + parent->width() - value - width());
            }
            
            m_updatingFromAnchors = false;
        }
    }
}

void DesignElement::setTop(qreal value) {
    if (!qFuzzyCompare(m_top, value)) {
        m_top = value;
        emit topChanged();
        
        // Update position and/or height based on anchor states
        CanvasElement* parent = parentElement();
        if (parent) {
            m_updatingFromAnchors = true;
            
            if (m_topAnchored && m_bottomAnchored) {
                // Both anchored: update position and height
                CanvasElement::setY(parent->y() + value);
                CanvasElement::setHeight(parent->height() - value - m_bottom);
            } else {
                // Only update position
                CanvasElement::setY(parent->y() + value);
            }
            
            m_updatingFromAnchors = false;
        }
    }
}

void DesignElement::setBottom(qreal value) {
    if (!qFuzzyCompare(m_bottom, value)) {
        m_bottom = value;
        emit bottomChanged();
        
        // Update position and/or height based on anchor states
        CanvasElement* parent = parentElement();
        if (parent) {
            m_updatingFromAnchors = true;
            
            if (m_topAnchored && m_bottomAnchored) {
                // Both anchored: update height only (position stays the same)
                CanvasElement::setHeight(parent->height() - m_top - value);
            } else if (!m_topAnchored) {
                // Only bottom anchored: update position
                CanvasElement::setY(parent->y() + parent->height() - value - height());
            }
            
            m_updatingFromAnchors = false;
        }
    }
}

void DesignElement::setLeftAnchored(bool anchored) {
    if (m_leftAnchored != anchored) {
        m_leftAnchored = anchored;
        emit leftAnchoredChanged();
        
        // If enabling anchor, calculate the current left distance
        if (anchored && parentElement()) {
            m_left = qRound(x() - parentElement()->x());
            emit leftChanged();
        }
        
        updateFromParentGeometry();
    }
}

void DesignElement::setRightAnchored(bool anchored) {
    if (m_rightAnchored != anchored) {
        m_rightAnchored = anchored;
        emit rightAnchoredChanged();
        
        // If enabling anchor, calculate the current right distance
        if (anchored && parentElement()) {
            m_right = qRound(parentElement()->x() + parentElement()->width() - (x() + width()));
            emit rightChanged();
        }
        
        updateFromParentGeometry();
    }
}

void DesignElement::setTopAnchored(bool anchored) {
    if (m_topAnchored != anchored) {
        m_topAnchored = anchored;
        emit topAnchoredChanged();
        
        // If enabling anchor, calculate the current top distance
        if (anchored && parentElement()) {
            m_top = qRound(y() - parentElement()->y());
            emit topChanged();
        }
        
        updateFromParentGeometry();
    }
}

void DesignElement::setBottomAnchored(bool anchored) {
    if (m_bottomAnchored != anchored) {
        m_bottomAnchored = anchored;
        emit bottomAnchoredChanged();
        
        // If enabling anchor, calculate the current bottom distance
        if (anchored && parentElement()) {
            m_bottom = qRound(parentElement()->y() + parentElement()->height() - (y() + height()));
            emit bottomChanged();
        }
        
        updateFromParentGeometry();
    }
}

void DesignElement::setIsFrozen(bool frozen) {
    if (m_isFrozen != frozen) {
        m_isFrozen = frozen;
        emit isFrozenChanged();
    }
}

void DesignElement::setSourceId(const QString& sourceId) {
    if (m_sourceId != sourceId) {
        m_sourceId = sourceId;
        emit sourceIdChanged();
    }
}

void DesignElement::updateFromParentGeometry() {
    CanvasElement* parent = parentElement();
    if (!parent || m_updatingFromAnchors) {
        return;
    }
    
    
    // Set flag to prevent circular updates
    m_updatingFromAnchors = true;
    
    // Calculate absolute position based on anchors
    qreal parentWidth = parent->width();
    qreal parentHeight = parent->height();
    qreal parentX = parent->x();
    qreal parentY = parent->y();
    
    qreal newX = x();
    qreal newY = y();
    qreal newWidth = width();
    qreal newHeight = height();
    
    // Horizontal constraints
    if (m_leftAnchored && m_rightAnchored) {
        // Both anchored: maintain distances from edges
        newX = parentX + m_left;
        newWidth = parentWidth - m_left - m_right;
    } else if (m_leftAnchored) {
        // Only left anchored: maintain left distance
        newX = parentX + m_left;
    } else if (m_rightAnchored) {
        // Only right anchored: maintain right distance
        newX = parentX + parentWidth - m_right - width();
    }
    
    // Vertical constraints
    if (m_topAnchored && m_bottomAnchored) {
        // Both anchored: maintain distances from edges
        newY = parentY + m_top;
        newHeight = parentHeight - m_top - m_bottom;
    } else if (m_topAnchored) {
        // Only top anchored: maintain top distance
        newY = parentY + m_top;
    } else if (m_bottomAnchored) {
        // Only bottom anchored: maintain bottom distance
        newY = parentY + parentHeight - m_bottom - height();
    }
    
    // Apply changes
    if (!qFuzzyCompare(newX, x()) || !qFuzzyCompare(newY, y()) ||
        !qFuzzyCompare(newWidth, width()) || !qFuzzyCompare(newHeight, height())) {
        // Call base class methods directly to avoid triggering updateAnchorsFromGeometry
        CanvasElement::setRect(QRectF(newX, newY, newWidth, newHeight));
    } else {
    }
    
    // Clear flag
    m_updatingFromAnchors = false;
}

void DesignElement::updateAnchorsFromGeometry() {
    CanvasElement* parent = parentElement();
    if (!parent) {
        return;
    }
    
    // Calculate anchor values based on current position relative to parent
    qreal parentWidth = parent->width();
    qreal parentHeight = parent->height();
    qreal parentX = parent->x();
    qreal parentY = parent->y();
    
    // Update left/right values
    m_left = qRound(x() - parentX);
    m_right = qRound(parentX + parentWidth - (x() + width()));
    
    // Update top/bottom values
    m_top = qRound(y() - parentY);
    m_bottom = qRound(parentY + parentHeight - (y() + height()));
    
    // Emit signals for property changes
    emit leftChanged();
    emit rightChanged();
    emit topChanged();
    emit bottomChanged();
}

void DesignElement::recalculateAnchors() {
    updateAnchorsFromGeometry();
}

void DesignElement::setX(qreal x) {
    CanvasElement::setX(x);
    if (parentElement()) {
        updateAnchorsFromGeometry();
    }
}

void DesignElement::setY(qreal y) {
    CanvasElement::setY(y);
    if (parentElement()) {
        updateAnchorsFromGeometry();
    }
}

void DesignElement::setWidth(qreal w) {
    CanvasElement::setWidth(w);
    if (parentElement()) {
        updateAnchorsFromGeometry();
    }
}

void DesignElement::setHeight(qreal h) {
    CanvasElement::setHeight(h);
    if (parentElement()) {
        updateAnchorsFromGeometry();
    }
}

void DesignElement::setRect(const QRectF &rect) {
    CanvasElement::setRect(rect);
    if (parentElement()) {
        updateAnchorsFromGeometry();
    }
}

void DesignElement::setParentElement(CanvasElement* parent) {
    // First disconnect any existing parent connections
    if (parentElement()) {
        disconnect(parentElement(), nullptr, this, nullptr);
    }
    
    // Call base class but it will skip parent tracking since isDesignElement() returns true
    // and we handle it ourselves
    CanvasElement::setParentElement(parent);
    
    if (parent) {
        // Subscribe to parent geometry changes
        connect(parent, &CanvasElement::widthChanged, 
                this, &DesignElement::onParentGeometryChanged,
                Qt::UniqueConnection);
        
        connect(parent, &CanvasElement::heightChanged, 
                this, &DesignElement::onParentGeometryChanged,
                Qt::UniqueConnection);
        
        connect(parent, &CanvasElement::xChanged, 
                this, &DesignElement::onParentGeometryChanged,
                Qt::UniqueConnection);
        
        connect(parent, &CanvasElement::yChanged, 
                this, &DesignElement::onParentGeometryChanged,
                Qt::UniqueConnection);
        
        // Initialize anchor values when parent is set
        updateAnchorsFromGeometry();
        
        // Trigger layout on parent if it's a Frame with flex
        Frame* parentFrame = qobject_cast<Frame*>(parent);
        if (parentFrame && parentFrame->flex()) {
            parentFrame->triggerLayout();
        }
    } else {
        // Element is being unparented - clear all anchor settings
        if (m_leftAnchored || m_rightAnchored || m_topAnchored || m_bottomAnchored) {
            m_leftAnchored = false;
            m_rightAnchored = false;
            m_topAnchored = false;
            m_bottomAnchored = false;
            
            emit leftAnchoredChanged();
            emit rightAnchoredChanged();
            emit topAnchoredChanged();
            emit bottomAnchoredChanged();
            
        }
        
        // Reset anchor position values
        m_left = 0;
        m_right = 0;
        m_top = 0;
        m_bottom = 0;
        
        emit leftChanged();
        emit rightChanged();
        emit topChanged();
        emit bottomChanged();
    }
}

void DesignElement::setParentElement(CanvasElement* parent, qreal left, qreal top) {
    // First set the parent
    setParentElement(parent);
    
    // Then position the element at the specified left and top
    if (parent) {
        setX(parent->x() + left);
        setY(parent->y() + top);
    }
}

void DesignElement::onParentGeometryChanged() {
    // Prevent re-entry during updates
    if (m_updatingFromAnchors) return;
    
    
    CanvasElement* parent = parentElement();
    if (!parent) return;
    
    // Track parent position changes
    static QMap<QString, QPointF> lastParentPositions;
    QString parentId = parent->getId();
    
    QPointF currentParentPos(parent->x(), parent->y());
    QPointF delta(0, 0);
    
    if (lastParentPositions.contains(parentId)) {
        QPointF lastPos = lastParentPositions[parentId];
        delta = currentParentPos - lastPos;
        
        if (!qFuzzyIsNull(delta.x()) || !qFuzzyIsNull(delta.y())) {
            // Set flag to prevent circular updates
            m_updatingFromAnchors = true;
            // Always move child when parent moves
            CanvasElement::setX(x() + delta.x());
            CanvasElement::setY(y() + delta.y());
            m_updatingFromAnchors = false;
        }
    }
    lastParentPositions[parentId] = currentParentPos;
    
    // Handle anchor-based resizing
    updateFromParentGeometry();
}

Component* DesignElement::createComponent() {
    // Get the element model from the parent (which should be the ElementModel)
    ElementModel* elementModel = qobject_cast<ElementModel*>(parent());
    if (!elementModel) {
        qWarning() << "DesignElement::createComponent - Parent is not an ElementModel";
        return nullptr;
    }
    
    // Get the Project from the ElementModel's parent
    Project* project = qobject_cast<Project*>(elementModel->parent());
    if (!project) {
        qWarning() << "DesignElement::createComponent - ElementModel has no Project parent";
        return nullptr;
    }
    
    // Get the creation manager from the project's controller
    CreationManager* creationManager = project->controller() ? project->controller()->creationManager() : nullptr;
    if (!creationManager) {
        qWarning() << "DesignElement::createComponent - No CreationManager available";
        return nullptr;
    }
    
    // Delegate to CreationManager
    return creationManager->createComponent(this);
}

// Note: copyElementRecursively has been moved to CreationManager

// Note: copyElementProperties has been moved to CreationManager

QList<PropertyDefinition> DesignElement::propertyDefinitions() const {
    // Base DesignElement has no specific properties
    // Derived classes should override this
    return QList<PropertyDefinition>();
}

void DesignElement::registerProperties() {
    // Call parent implementation first
    CanvasElement::registerProperties();
    
    // Register DesignElement-specific properties (anchor properties)
    m_properties->registerProperty("left", 0.0);
    m_properties->registerProperty("right", 0.0);
    m_properties->registerProperty("top", 0.0);
    m_properties->registerProperty("bottom", 0.0);
    m_properties->registerProperty("leftAnchored", false);
    m_properties->registerProperty("rightAnchored", false);
    m_properties->registerProperty("topAnchored", false);
    m_properties->registerProperty("bottomAnchored", false);
    m_properties->registerProperty("isFrozen", false);
    m_properties->registerProperty("sourceId", QString());
}