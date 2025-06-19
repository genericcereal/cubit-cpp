#include "DesignElement.h"
#include "Scripts.h"
#include "ScriptExecutor.h"
#include "Application.h"
#include "Canvas.h"
#include <QCoreApplication>
#include <QDebug>

DesignElement::DesignElement(const QString &id, QObject *parent)
    : CanvasElement(Element::FrameType, id, parent)  // Use FrameType as default, subclasses will override
{
    m_scripts = std::make_unique<Scripts>(this);
}

DesignElement::~DesignElement() {
    // Destructor defined here so Scripts is complete type
}

Scripts* DesignElement::scripts() const {
    return m_scripts.get();
}

void DesignElement::executeScriptEvent(const QString& eventName) {
    qDebug() << "DesignElement::executeScriptEvent called for" << getId() << "event:" << eventName;
    
    if (!m_scripts) {
        qWarning() << "DesignElement: No scripts available";
        return;
    }
    
    qDebug() << "DesignElement" << getId() << "has" << m_scripts->nodeCount() << "nodes," << m_scripts->edgeCount() << "edges";
    
    // Create a temporary script executor for this element
    ScriptExecutor executor(this);
    executor.setScripts(m_scripts.get());
    
    // Get element model and canvas controller from Application
    // Since design elements are part of a canvas, we can get the active canvas
    Application* app = qobject_cast<Application*>(qApp);
    if (app && app->activeCanvas()) {
        Canvas* canvas = app->activeCanvas();
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
            m_left = x() - parentElement()->x();
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
            m_right = parentElement()->x() + parentElement()->width() - (x() + width());
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
            m_top = y() - parentElement()->y();
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
            m_bottom = parentElement()->y() + parentElement()->height() - (y() + height());
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
    
    qDebug() << "DesignElement" << getId() << "updateFromParentGeometry called";
    qDebug() << "  Anchors - L:" << m_leftAnchored << "R:" << m_rightAnchored 
             << "T:" << m_topAnchored << "B:" << m_bottomAnchored;
    
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
        qDebug() << "  Updating geometry from:" << QRectF(x(), y(), width(), height())
                 << "to:" << QRectF(newX, newY, newWidth, newHeight);
        // Call base class methods directly to avoid triggering updateAnchorsFromGeometry
        CanvasElement::setRect(QRectF(newX, newY, newWidth, newHeight));
    } else {
        qDebug() << "  No geometry change needed";
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
    m_left = x() - parentX;
    m_right = parentX + parentWidth - (x() + width());
    
    // Update top/bottom values
    m_top = y() - parentY;
    m_bottom = parentY + parentHeight - (y() + height());
    
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
            
            qDebug() << "DesignElement" << getId() << "unparented - cleared all anchors";
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
    qDebug() << "Parent geometry changed for" << getId();
    updateFromParentGeometry();
}