#include "DesignElement.h"
#include "Scripts.h"
#include "ScriptExecutor.h"
#include "Application.h"
#include "Project.h"
#include "Component.h"
#include "FrameComponentInstance.h"
#include "FrameComponentVariant.h"
#include "TextComponentVariant.h"
#include "TextComponentInstance.h"
#include "Frame.h"
#include "Text.h"
#include "UniqueIdGenerator.h"
#include "ElementModel.h"
#include "SelectionManager.h"
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

void DesignElement::executeScriptEvent(const QString& eventName) {
    
    if (!m_scripts) {
        qWarning() << "DesignElement: No scripts available";
        return;
    }
    
    
    // Create a temporary script executor for this element
    ScriptExecutor executor(this);
    executor.setScripts(m_scripts.get());
    
    // Get element model and canvas controller from Application
    // Since design elements are part of a canvas, we can get the active canvas
    Application* app = qobject_cast<Application*>(qApp);
    if (app && app->activeCanvas()) {
        Project* canvas = app->activeCanvas();
        if (canvas->elementModel()) {
            executor.setElementModel(canvas->elementModel());
        }
        if (canvas->controller()) {
            executor.setCanvasController(canvas->controller());
        }
    }
    
    executor.executeEvent(eventName);
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
        
        qDebug() << "DesignElement" << getId() << "leftAnchored set to" << anchored;
        
        // If enabling anchor, calculate the current left distance
        if (anchored && parentElement()) {
            m_left = qRound(x() - parentElement()->x());
            emit leftChanged();
            qDebug() << "  Left distance:" << m_left;
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
    // Get the element model from the Application
    Application* app = Application::instance();
    if (!app || !app->activeCanvas() || !app->activeCanvas()->elementModel()) {
        qWarning() << "DesignElement::createComponent - No active canvas or element model";
        return nullptr;
    }
    
    ElementModel* elementModel = app->activeCanvas()->elementModel();
    SelectionManager* selectionManager = app->activeCanvas()->selectionManager();
    
    // Create a new Component
    QString componentId = UniqueIdGenerator::generate16DigitId();
    Component* component = new Component(componentId, this->parent());
    
    // Create a mapping of old IDs to new IDs for maintaining parent-child relationships
    QHash<QString, QString> oldToNewIdMap;
    
    // Create a ComponentVariant that is a copy of this element
    QString variantId = UniqueIdGenerator::generate16DigitId();
    DesignElement* variant = nullptr;
    
    // Store the ID mapping for the root element
    oldToNewIdMap[this->getId()] = variantId;
    
    // Create the appropriate variant type based on the source element
    if (Frame* sourceFrame = qobject_cast<Frame*>(this)) {
        // Frame elements become ComponentVariants
        FrameComponentVariant* variantFrame = new FrameComponentVariant(variantId, this->parent());
        variantFrame->setVariantName("Variant1");
        
        // Center the variant in the canvas by positioning it so its center is at (0,0)
        // This ensures it will be visible when the viewport centers on entering variant mode
        qreal variantX = -width() / 2.0;
        qreal variantY = -height() / 2.0;
        variantFrame->setRect(QRectF(variantX, variantY, width(), height()));
        
        // Copy all style properties from the source frame
        variantFrame->setFill(sourceFrame->fill());
        variantFrame->setBorderColor(sourceFrame->borderColor());
        variantFrame->setBorderWidth(sourceFrame->borderWidth());
        variantFrame->setBorderRadius(sourceFrame->borderRadius());
        variantFrame->setOverflow(sourceFrame->overflow());
        
        // Copy anchor properties
        copyElementProperties(variantFrame, sourceFrame, false);
        
        variant = variantFrame;
        component->setComponentType("frame");
    } else if (Text* sourceText = qobject_cast<Text*>(this)) {
        // Text elements become TextComponentVariants
        TextComponentVariant* variantText = new TextComponentVariant(variantId, this->parent());
        variantText->setVariantName("Variant1");
        
        // Center the variant in the canvas by positioning it so its center is at (0,0)
        // This ensures it will be visible when the viewport centers on entering variant mode
        qreal variantX = -width() / 2.0;
        qreal variantY = -height() / 2.0;
        variantText->setRect(QRectF(variantX, variantY, width(), height()));
        
        // Copy all text properties from the source text
        variantText->setContent(sourceText->content());
        variantText->setFont(sourceText->font());
        variantText->setColor(sourceText->color());
        variantText->setPosition(sourceText->position());
        
        // Copy anchor properties
        copyElementProperties(variantText, sourceText, false);
        
        variant = variantText;
        component->setComponentType("text");
    } else {
        qWarning() << "DesignElement::createComponent - Only Frame and Text elements can be converted to components";
        delete component;
        return nullptr;
    }
    
    // Add the variant to the component
    component->addVariant(variant);
    
    // Add the variant to the element model
    elementModel->addElement(variant);
    
    // Now copy all children of this element to be children of the variant frame
    QList<Element*> children = elementModel->getChildrenRecursive(this->getId());
    
    // Filter to only get direct children (not grandchildren)
    QList<CanvasElement*> directChildren;
    for (Element* child : children) {
        if (child->getParentElementId() == this->getId()) {
            if (CanvasElement* canvasChild = qobject_cast<CanvasElement*>(child)) {
                directChildren.append(canvasChild);
            }
        }
    }
    
    // Recursively copy each direct child
    for (CanvasElement* child : directChildren) {
        copyElementRecursively(child, variant, elementModel, oldToNewIdMap);
    }
    
    // Add the component to the element model first
    elementModel->addElement(component);
    
    // Create the appropriate instance type based on component type
    QString instanceId = UniqueIdGenerator::generate16DigitId();
    CanvasElement* instance = nullptr;
    
    if (component->componentType() == "text") {
        TextComponentInstance* textInstance = new TextComponentInstance(instanceId, this->parent());
        textInstance->setRect(QRectF(x(), y(), width(), height()));
        instance = textInstance;
    } else {
        // Default to frame-based ComponentInstance
        FrameComponentInstance* frameInstance = new FrameComponentInstance(instanceId, this->parent());
        frameInstance->setRect(QRectF(x(), y(), width(), height()));
        instance = frameInstance;
    }
    
    // Add the instance to the element model
    elementModel->addElement(instance);
    
    // Set the instance to reference this component after both are in the model
    if (TextComponentInstance* textInstance = qobject_cast<TextComponentInstance*>(instance)) {
        textInstance->setInstanceOf(componentId);
    } else if (FrameComponentInstance* frameInstance = qobject_cast<FrameComponentInstance*>(instance)) {
        frameInstance->setInstanceOf(componentId);
    }
    
    // Clear selection first
    if (selectionManager) {
        selectionManager->clearSelection();
    }
    
    // Get all children of this element recursively
    QList<Element*> childrenToDelete = elementModel->getChildrenRecursive(this->getId());
    
    // Select the new instance
    if (selectionManager) {
        selectionManager->selectElement(instance);
    }
    
    // Delete all children first (in reverse order to handle nested children properly)
    for (int i = childrenToDelete.size() - 1; i >= 0; --i) {
        elementModel->removeElement(childrenToDelete[i]->getId());
    }
    
    // Finally delete the original element itself
    elementModel->removeElement(this->getId());
    
    return component;
}

CanvasElement* DesignElement::copyElementRecursively(CanvasElement* sourceElement, CanvasElement* parentInVariant, ElementModel* elementModel, QHash<QString, QString>& oldToNewIdMap) {
    if (!sourceElement || !parentInVariant || !elementModel) {
        return nullptr;
    }
    
    // Create the appropriate type of copy
    CanvasElement* copiedElement = nullptr;
    QString newId = UniqueIdGenerator::generate16DigitId();
    
    // Store the ID mapping
    oldToNewIdMap[sourceElement->getId()] = newId;
    
    // Create copy based on type
    if (Frame* frame = qobject_cast<Frame*>(sourceElement)) {
        Frame* frameCopy = new Frame(newId, parentInVariant);
        frameCopy->setName("Copied " + frame->getName());
        
        // For child elements, calculate relative position
        qreal relX = sourceElement->x() - sourceElement->parentElement()->x();
        qreal relY = sourceElement->y() - sourceElement->parentElement()->y();
        
        // Set position relative to parent in variant
        frameCopy->setRect(QRectF(relX, relY, frame->width(), frame->height()));
        
        // Use utility function to copy all properties
        copyElementProperties(frameCopy, frame, false);
        
        copiedElement = frameCopy;
    } else if (Text* text = qobject_cast<Text*>(sourceElement)) {
        Text* textCopy = new Text(newId, parentInVariant);
        textCopy->setName("Copied " + text->getName());
        
        // For child elements, calculate relative position
        qreal relX = sourceElement->x() - sourceElement->parentElement()->x();
        qreal relY = sourceElement->y() - sourceElement->parentElement()->y();
        
        textCopy->setRect(QRectF(relX, relY, text->width(), text->height()));
        
        // Use utility function to copy all properties
        copyElementProperties(textCopy, text, false);
        
        copiedElement = textCopy;
    }
    
    if (copiedElement) {
        // Set parent relationship
        copiedElement->setParentElementId(parentInVariant->getId());
        copiedElement->setParentElement(parentInVariant);
        
        // Add to element model
        elementModel->addElement(copiedElement);
        
        // Now recursively copy all children
        QList<Element*> children = elementModel->getChildrenRecursive(sourceElement->getId());
        
        // Filter to only get direct children (not grandchildren)
        QList<CanvasElement*> directChildren;
        for (Element* child : children) {
            if (child->getParentElementId() == sourceElement->getId()) {
                if (CanvasElement* canvasChild = qobject_cast<CanvasElement*>(child)) {
                    directChildren.append(canvasChild);
                }
            }
        }
        
        // Recursively copy each direct child
        for (CanvasElement* child : directChildren) {
            copyElementRecursively(child, copiedElement, elementModel, oldToNewIdMap);
        }
    }
    
    return copiedElement;
}

void DesignElement::copyElementProperties(CanvasElement* target, CanvasElement* source, bool copyGeometry)
{
    if (!target || !source) {
        return;
    }
    
    // Copy geometry if requested
    if (copyGeometry) {
        target->setWidth(source->width());
        target->setHeight(source->height());
        // Note: Position (x, y) is handled differently in each context, so not copied here
    }
    
    // Copy anchor properties for DesignElements
    if (DesignElement* targetDesign = qobject_cast<DesignElement*>(target)) {
        if (DesignElement* sourceDesign = qobject_cast<DesignElement*>(source)) {
            // Copy anchor values
            targetDesign->setLeft(sourceDesign->left());
            targetDesign->setRight(sourceDesign->right());
            targetDesign->setTop(sourceDesign->top());
            targetDesign->setBottom(sourceDesign->bottom());
            
            // Copy anchor states
            targetDesign->setLeftAnchored(sourceDesign->leftAnchored());
            targetDesign->setRightAnchored(sourceDesign->rightAnchored());
            targetDesign->setTopAnchored(sourceDesign->topAnchored());
            targetDesign->setBottomAnchored(sourceDesign->bottomAnchored());
        }
    }
    
    // Copy type-specific properties
    if (Frame* targetFrame = qobject_cast<Frame*>(target)) {
        if (Frame* sourceFrame = qobject_cast<Frame*>(source)) {
            targetFrame->setFill(sourceFrame->fill());
            targetFrame->setBorderColor(sourceFrame->borderColor());
            targetFrame->setBorderWidth(sourceFrame->borderWidth());
            targetFrame->setBorderRadius(sourceFrame->borderRadius());
            targetFrame->setOverflow(sourceFrame->overflow());
        }
    } else if (Text* targetText = qobject_cast<Text*>(target)) {
        if (Text* sourceText = qobject_cast<Text*>(source)) {
            targetText->setContent(sourceText->content());
            targetText->setFont(sourceText->font());
            targetText->setColor(sourceText->color());
        }
    }
}