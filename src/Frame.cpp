#include "Frame.h"
#include "FlexLayoutEngine.h"
#include "ElementModel.h"
#include "Application.h"
#include "Project.h"
#include <QTimer>
#include <QDebug>

Frame::Frame(const QString &id, QObject *parent)
    : DesignElement(id, parent)
    , m_fillColor(LightBlue)
    , m_borderColor(Qt::black)
    , m_borderWidth(1)
    , m_borderRadius(0)
    , m_overflow(Hidden)
    , m_acceptsChildren(true)
    , m_flex(false)
    , m_orientation(Row)
    , m_gap(0.0)
    , m_position(Absolute)
    , m_justify(JustifyStart)
    , m_align(AlignStart)
    , m_layoutEngine(std::make_unique<FlexLayoutEngine>())
{
    // Set element type
    elementType = Element::FrameType;
    
    setName(QString("Frame %1").arg(id.right(4)));  // Use last 4 digits for display
    
    // Defer connection setup to ensure application is ready
    QTimer::singleShot(0, this, [this]() {
        setupElementModelConnections();
    });
}

Frame::~Frame() 
{
    // Disconnect all child geometry signals
    if (m_layoutEngine) {
        m_layoutEngine->disconnectChildGeometrySignals(this);
    }
}

QColor Frame::fill() const
{
    switch (m_fillColor) {
        case LightBlue:
            return QColor(173, 216, 230); // Light blue
        case DarkBlue:
            return QColor(0, 0, 139); // Dark blue
        case Green:
            return QColor(0, 128, 0); // Green
        case Red:
            return QColor(255, 0, 0); // Red
        default:
            return QColor(173, 216, 230); // Default to light blue
    }
}

void Frame::setFillColor(FillColor color)
{
    if (m_fillColor != color) {
        m_fillColor = color;
        emit fillColorChanged();
        emit elementChanged();
    }
}

void Frame::setBorderColor(const QColor &color)
{
    if (m_borderColor != color) {
        m_borderColor = color;
        emit borderColorChanged();
        emit elementChanged();
    }
}

void Frame::setBorderWidth(int width)
{
    if (m_borderWidth != width) {
        m_borderWidth = width;
        emit borderWidthChanged();
        emit elementChanged();
    }
}

void Frame::setBorderRadius(int radius)
{
    if (m_borderRadius != radius) {
        m_borderRadius = radius;
        emit borderRadiusChanged();
        emit elementChanged();
    }
}

void Frame::setOverflow(OverflowMode mode)
{
    if (m_overflow != mode) {
        m_overflow = mode;
        emit overflowChanged();
        emit elementChanged();
    }
}

void Frame::setAcceptsChildren(bool accepts)
{
    if (m_acceptsChildren != accepts) {
        m_acceptsChildren = accepts;
        emit acceptsChildrenChanged();
        emit elementChanged();
    }
}

void Frame::setFlex(bool flex)
{
    if (m_flex != flex) {
        m_flex = flex;
        emit flexChanged();
        emit elementChanged();
        
        // Immediately layout children if enabling flex
        if (flex) {
            // Get the element model
            Application* app = Application::instance();
            if (app && app->activeCanvas() && app->activeCanvas()->elementModel()) {
                ElementModel* elementModel = app->activeCanvas()->elementModel();
                m_layoutEngine->connectChildGeometrySignals(this, elementModel);
            }
            layoutChildren();
        } else {
            // Disconnect child geometry signals when disabling flex
            m_layoutEngine->disconnectChildGeometrySignals(this);
        }
    }
}

void Frame::setOrientation(LayoutOrientation orientation)
{
    if (m_orientation != orientation) {
        m_orientation = orientation;
        emit orientationChanged();
        emit elementChanged();
        
        if (m_flex) {
            layoutChildren();
        }
    }
}

void Frame::setGap(qreal gap)
{
    if (m_gap != gap) {
        m_gap = gap;
        emit gapChanged();
        emit elementChanged();
        
        if (m_flex) {
            // Trigger layout with GapChanged reason to allow parent resize even if selected
            Application* app = Application::instance();
            if (app && app->activeCanvas() && app->activeCanvas()->elementModel()) {
                m_layoutEngine->layoutChildren(this, app->activeCanvas()->elementModel(), 
                                             FlexLayoutEngine::GapChanged);
            }
        }
    }
}

void Frame::setPosition(PositionType position)
{
    if (m_position != position) {
        m_position = position;
        emit positionChanged();
        emit elementChanged();
        triggerLayout();
        
        // Also trigger layout on parent if it's a frame with flex
        if (parentElement()) {
            Frame* parentFrame = qobject_cast<Frame*>(parentElement());
            if (parentFrame && parentFrame->flex()) {
                parentFrame->triggerLayout();
            }
        }
    }
}

void Frame::setJustify(JustifyContent justify)
{
    if (m_justify != justify) {
        m_justify = justify;
        emit justifyChanged();
        emit elementChanged();
        
        if (m_flex) {
            // When justify changes, we need to reposition children
            // but we should NOT resize the parent
            Application* app = Application::instance();
            if (app && app->activeCanvas() && app->activeCanvas()->elementModel()) {
                m_layoutEngine->layoutChildren(this, app->activeCanvas()->elementModel(),
                                             FlexLayoutEngine::JustifyChanged);
            }
        }
    }
}

void Frame::setAlign(AlignItems align)
{
    if (m_align != align) {
        m_align = align;
        emit alignChanged();
        emit elementChanged();
        
        if (m_flex) {
            // When align changes, we need to reposition children
            // but we should NOT resize the parent
            Application* app = Application::instance();
            if (app && app->activeCanvas() && app->activeCanvas()->elementModel()) {
                m_layoutEngine->layoutChildren(this, app->activeCanvas()->elementModel(),
                                             FlexLayoutEngine::AlignChanged);
            }
        }
    }
}

void Frame::layoutChildren()
{
    // Get the element model from the application
    Application* app = Application::instance();
    if (!app || !app->activeCanvas()) {
        qDebug() << "Frame::layoutChildren - No active canvas";
        return;
    }
    
    ElementModel* elementModel = app->activeCanvas()->elementModel();
    if (!elementModel) {
        qDebug() << "Frame::layoutChildren - No element model";
        return;
    }
    
    qDebug() << "Frame::layoutChildren - Laying out children for frame" << getId() 
             << "flex:" << m_flex << "position:" << m_position;
    
    // Use the layout engine to layout children
    m_layoutEngine->layoutChildren(this, elementModel);
}

void Frame::triggerLayout()
{
    // Use a timer to batch layout updates
    QTimer::singleShot(0, this, &Frame::layoutChildren);
}

void Frame::setWidth(qreal w)
{
    DesignElement::setWidth(w);
    
    // Only trigger layout if we're not already in a layout operation
    if (!m_layoutEngine || !m_layoutEngine->isLayouting()) {
        triggerLayout();
        
        // Also trigger layout on parent if it's a frame with flex
        if (parentElement() && m_position == Relative) {
            Frame* parentFrame = qobject_cast<Frame*>(parentElement());
            if (parentFrame && parentFrame->flex()) {
                parentFrame->triggerLayout();
            }
        }
    }
}

void Frame::setHeight(qreal h)
{
    DesignElement::setHeight(h);
    
    // Only trigger layout if we're not already in a layout operation
    if (!m_layoutEngine || !m_layoutEngine->isLayouting()) {
        triggerLayout();
        
        // Also trigger layout on parent if it's a frame with flex
        if (parentElement() && m_position == Relative) {
            Frame* parentFrame = qobject_cast<Frame*>(parentElement());
            if (parentFrame && parentFrame->flex()) {
                parentFrame->triggerLayout();
            }
        }
    }
}

void Frame::setRect(const QRectF &rect)
{
    DesignElement::setRect(rect);
    
    // Only trigger layout if we're not already in a layout operation
    if (!m_layoutEngine || !m_layoutEngine->isLayouting()) {
        triggerLayout();
        
        // Also trigger layout on parent if it's a frame with flex
        if (parentElement() && m_position == Relative) {
            Frame* parentFrame = qobject_cast<Frame*>(parentElement());
            if (parentFrame && parentFrame->flex()) {
                parentFrame->triggerLayout();
            }
        }
    }
}

void Frame::setupElementModelConnections()
{
    // Connect to application to listen for element changes
    Application* app = Application::instance();
    if (app && app->activeCanvas() && app->activeCanvas()->elementModel()) {
        qDebug() << "Frame::setupElementModelConnections - Setting up connections for frame" << getId();
        
        connect(app->activeCanvas()->elementModel(), &ElementModel::elementAdded,
                this, [this, app](Element* element) {
                    if (element && element->getParentElementId() == this->getId()) {
                        qDebug() << "Frame - Child added to frame" << getId();
                        if (m_flex && app->activeCanvas() && app->activeCanvas()->elementModel()) {
                            // Reconnect signals when children are added
                            m_layoutEngine->disconnectChildGeometrySignals(this);
                            m_layoutEngine->connectChildGeometrySignals(this, app->activeCanvas()->elementModel());
                        }
                        triggerLayout();
                    }
                });
        connect(app->activeCanvas()->elementModel(), &ElementModel::elementRemoved,
                this, [this, app](const QString&) {
                    // Check if removed element was our child
                    if (m_flex && app->activeCanvas() && app->activeCanvas()->elementModel()) {
                        // Reconnect signals when children are removed
                        m_layoutEngine->disconnectChildGeometrySignals(this);
                        m_layoutEngine->connectChildGeometrySignals(this, app->activeCanvas()->elementModel());
                    }
                    // Trigger layout when any child is removed
                    triggerLayout();
                });
        connect(app->activeCanvas()->elementModel(), &ElementModel::elementUpdated,
                this, [this](Element* element) {
                    if (element && element->getParentElementId() == this->getId()) {
                        // Trigger layout when child properties change
                        triggerLayout();
                    }
                });
                
        // Trigger initial layout if we have flex enabled
        if (m_flex) {
            triggerLayout();
        }
    } else {
        qDebug() << "Frame::setupElementModelConnections - No active canvas/model yet for frame" << getId();
    }
}