#include "DesignElement.h"
#include "Scripts.h"
#include "PropertyRegistry.h"
#include "ScriptExecutor.h"
#include "Application.h"
#include "Project.h"
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
#include <QMetaProperty>

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

void DesignElement::setInstanceOf(const QString& sourceId) {
    if (m_instanceOf != sourceId) {        // Disconnect from old source
        if (!m_instanceOf.isEmpty() && m_sourceElement) {            disconnect(m_sourceConnection);
            m_sourceElement = nullptr;
        }
        
        m_instanceOf = sourceId;
        emit instanceOfChanged();
        
        // Connect to new source
        if (!sourceId.isEmpty()) {
            // Find the source element
            ElementModel* model = qobject_cast<ElementModel*>(parent());
            if (model) {
                Element* sourceElement = model->getElementById(sourceId);
                DesignElement* designSource = qobject_cast<DesignElement*>(sourceElement);
                if (designSource) {                    m_sourceElement = designSource;
                    
                    // Subscribe to all property changes
                    m_sourceConnection = connect(designSource, &QObject::destroyed,
                                               this, &DesignElement::onSourceElementDestroyed);
                    
                    // Connect to property change signal
                    connect(designSource, &Element::propertyChanged,
                           this, &DesignElement::onSourceElementChanged,
                           Qt::UniqueConnection);
                    
                    // Initial sync
                    onSourceElementChanged();
                } else {
                    qWarning() << "[DesignElement::setInstanceOf]" << getId() << "Source element not found or not a DesignElement:" << sourceId;
                }
            }
        }
    }
}

void DesignElement::setComponentId(const QString& compId) {
    if (m_componentId != compId) {
        m_componentId = compId;
        emit componentIdChanged();
    }
}

void DesignElement::setAncestorInstance(const QString& ancestorId) {
    if (m_ancestorInstance != ancestorId) {
        m_ancestorInstance = ancestorId;
        emit ancestorInstanceChanged();
    }
}

void DesignElement::setBoxShadow(const BoxShadow& boxShadow) {
    if (m_boxShadow != boxShadow) {
        m_boxShadow = boxShadow;
        emit boxShadowChanged();
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
        // Check if THIS element is an instance child (has instanceOf set)
        // If so, set ancestorInstance to the instance parent's source
        DesignElement* parentDesign = qobject_cast<DesignElement*>(parent);
        if (parentDesign && !m_instanceOf.isEmpty()) {
            // This element is an instance child
            // The parent should be an instance, so we get its instanceOf (the source parent)
            QString parentSourceId = parentDesign->instanceOf();
            if (!parentSourceId.isEmpty() && m_ancestorInstance.isEmpty()) {
                setAncestorInstance(parentSourceId);            }
            // Or if parent already has ancestorInstance, inherit it
            else if (!parentDesign->ancestorInstance().isEmpty() && m_ancestorInstance.isEmpty()) {
                setAncestorInstance(parentDesign->ancestorInstance());            }
        }
        
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

// Component system has been removed - use instanceOf pattern instead
// Component* DesignElement::createComponent() {
//     ...
// }

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
    m_properties->registerProperty("instanceOf", QString());
    m_properties->registerProperty("boxShadow", QVariant::fromValue(BoxShadow()));
    m_properties->registerProperty("isVariant", false);
    m_properties->registerProperty("isSelectable", true);
}

void DesignElement::onSourceElementChanged() {
    if (!m_sourceElement) {        return;
    }    // Check if this is a child element (has a parent)
    bool isChildElement = !getParentElementId().isEmpty();    // Get the meta object for property copying
    const QMetaObject* metaObject = m_sourceElement->metaObject();
    
    // Copy all properties except certain ones
    for (int i = 0; i < metaObject->propertyCount(); ++i) {
        QMetaProperty metaProp = metaObject->property(i);
        const char* propName = metaProp.name();
        
        // Skip certain properties that shouldn't be copied
        QString propNameStr(propName);
        if (propNameStr == "objectName" || 
            propNameStr == "elementId" || 
            propNameStr == "parentId" ||
            propNameStr == "name" ||  // Don't overwrite instance's custom name
            propNameStr == "selected" ||
            propNameStr == "instanceOf" ||
            propNameStr == "componentId") {  // Don't copy component ID
            continue;
        }
        
        // For root instances, skip position to maintain their placement
        // For child elements, sync position too
        if ((propNameStr == "x" || propNameStr == "y") && !isChildElement) {            continue;
        }
        
        // For now, skip width/height for root instances too
        if ((propNameStr == "width" || propNameStr == "height") && !isChildElement) {
            continue;
        }
        
        // Copy the property value
        QVariant value = m_sourceElement->property(propName);
        if (value.isValid()) {            setProperty(propName, value);
        }
    }
    
    // For child elements, also sync geometry using CanvasElement methods
    if (isChildElement) {
        if (CanvasElement* sourceCanvas = qobject_cast<CanvasElement*>(m_sourceElement)) {            setX(sourceCanvas->x());
            setY(sourceCanvas->y());
            setWidth(sourceCanvas->width());
            setHeight(sourceCanvas->height());
        }
    }}

void DesignElement::onSourceElementDestroyed() {
    m_sourceElement = nullptr;
    m_instanceOf.clear();
    emit instanceOfChanged();
}