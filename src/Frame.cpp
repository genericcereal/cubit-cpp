#include "Frame.h"
#include "FlexLayoutEngine.h"
#include "ElementModel.h"
#include "Application.h"
#include "Project.h"
#include "SelectionManager.h"
#include "FrameFactory.h"
#include <QTimer>
#include <QDebug>

Frame::Frame(const QString &id, QObject *parent)
    : DesignElement(id, parent)
    , m_fill(173, 216, 230) // Default light blue
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


void Frame::setFill(const QColor &color)
{
    if (m_fill != color) {
        m_fill = color;
        emit fillChanged();
        emit elementChanged();
    }
}

void Frame::setColorFormat(ColorFormat format)
{
    if (m_colorFormat != format) {
        m_colorFormat = format;
        emit colorFormatChanged();
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
        bool oldCanResizeWidth = canResizeWidth();
        bool oldCanResizeHeight = canResizeHeight();
        
        m_flex = flex;
        emit flexChanged();
        emit elementChanged();
        
        // Emit canResize signals if the state changed
        if (oldCanResizeWidth != canResizeWidth()) {
            emit canResizeWidthChanged();
        }
        if (oldCanResizeHeight != canResizeHeight()) {
            emit canResizeHeightChanged();
        }
        
        // Immediately layout children if enabling flex
        if (flex) {
            // Get the element model
            Application* app = Application::instance();
            if (app && app->activeCanvas() && app->activeCanvas()->elementModel()) {
                ElementModel* elementModel = app->activeCanvas()->elementModel();
                m_layoutEngine->connectChildGeometrySignals(this, elementModel);
            }
            m_layoutEngine->scheduleLayout(this);
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
        
        if (m_flex && m_layoutEngine) {
            m_layoutEngine->scheduleLayout(this);
        }
    }
}

void Frame::setGap(qreal gap)
{
    if (m_gap != gap) {
        m_gap = gap;
        emit gapChanged();
        emit elementChanged();
        
        if (m_flex && m_layoutEngine) {
            // Trigger layout with GapChanged reason to allow parent resize even if selected
            m_layoutEngine->scheduleLayout(this, FlexLayoutEngine::GapChanged);
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
        
        if (m_flex && m_layoutEngine) {
            // When justify changes, we need to reposition children
            // but we should NOT resize the parent
            m_layoutEngine->scheduleLayout(this, FlexLayoutEngine::JustifyChanged);
        }
    }
}

void Frame::setAlign(AlignItems align)
{
    if (m_align != align) {
        m_align = align;
        emit alignChanged();
        emit elementChanged();
        
        if (m_flex && m_layoutEngine) {
            // When align changes, we need to reposition children
            // but we should NOT resize the parent
            m_layoutEngine->scheduleLayout(this, FlexLayoutEngine::AlignChanged);
        }
    }
}

void Frame::setWidthType(SizeType type)
{
    // Validate constraints
    if (type == SizeRelative && !m_flex) {
        qWarning() << "Cannot set width type to Relative when flex is disabled";
        return;
    }
    
    if (type == SizeFill && parentElementId.isEmpty()) {
        qWarning() << "Cannot set width type to Fill when frame has no parent";
        return;
    }
    
    // TODO: Check for children when type == SizeFitContent
    
    if (m_widthType != type) {
        bool oldCanResize = canResizeWidth();
        m_widthType = type;
        emit widthTypeChanged();
        emit elementChanged();
        triggerLayout();
        
        if (oldCanResize != canResizeWidth()) {
            emit canResizeWidthChanged();
        }
    }
}

void Frame::setHeightType(SizeType type)
{
    // Validate constraints
    if (type == SizeRelative && !m_flex) {
        qWarning() << "Cannot set height type to Relative when flex is disabled";
        return;
    }
    
    if (type == SizeFill && parentElementId.isEmpty()) {
        qWarning() << "Cannot set height type to Fill when frame has no parent";
        return;
    }
    
    // TODO: Check for children when type == SizeFitContent
    
    if (m_heightType != type) {
        bool oldCanResize = canResizeHeight();
        m_heightType = type;
        emit heightTypeChanged();
        emit elementChanged();
        triggerLayout();
        
        if (oldCanResize != canResizeHeight()) {
            emit canResizeHeightChanged();
        }
    }
}

bool Frame::canResizeWidth() const
{
    // Cannot resize width if flex is enabled and width type is fit content
    return !(m_flex && m_widthType == SizeFitContent);
}

bool Frame::canResizeHeight() const
{
    // Cannot resize height if flex is enabled and height type is fit content
    return !(m_flex && m_heightType == SizeFitContent);
}

void Frame::setControlled(bool controlled)
{
    if (m_controlled != controlled) {
        m_controlled = controlled;
        emit controlledChanged();
        emit elementChanged();
    }
}

void Frame::setRole(Role role)
{
    if (m_role != role) {
        m_role = role;
        emit roleChanged();
        emit elementChanged();
    }
}

void Frame::setPlatform(const QString& platform)
{
    if (m_platform != platform) {
        m_platform = platform;
        
        // When platform is set (not undefined), automatically set role to container
        if (!platform.isEmpty()) {
            if (m_role != container) {
                m_role = container;
                emit roleChanged();
            }
        }
        
        emit platformChanged();
        emit elementChanged();
    }
}

void Frame::triggerLayout()
{
    // If we have flex enabled, schedule a layout through the layout engine
    if (m_flex && m_layoutEngine) {
        qDebug() << "Frame::triggerLayout - Called for frame" << getId();
        m_layoutEngine->scheduleLayout(this);
    }
}

void Frame::setWidth(qreal w)
{
    // Check if parent is currently laying out (in addition to our own layout engine)
    bool parentIsLayouting = false;
    if (parentElement()) {
        Frame* parentFrame = qobject_cast<Frame*>(parentElement());
        if (parentFrame && parentFrame->m_layoutEngine) {
            parentIsLayouting = parentFrame->m_layoutEngine->isLayouting();
        }
    }
    
    // Check if width resizing is allowed (but allow if layout engine is doing it)
    if (!canResizeWidth() && !parentIsLayouting && (!m_layoutEngine || !m_layoutEngine->isLayouting())) {
        return;
    }
    
    DesignElement::setWidth(w);
    
    // Only trigger layout if neither we nor our parent are in a layout operation
    if (!parentIsLayouting && (!m_layoutEngine || !m_layoutEngine->isLayouting())) {
        triggerLayout();
        
        // Also trigger layout on parent if it's a frame with flex
        if (parentElement() && m_position == Relative) {
            Frame* parentFrame = qobject_cast<Frame*>(parentElement());
            // Check if parent is currently laying out its children
            if (parentFrame && parentFrame->flex() && 
                (!parentFrame->m_layoutEngine || !parentFrame->m_layoutEngine->isLayouting())) {
                parentFrame->triggerLayout();
            }
        }
    }
}

void Frame::setHeight(qreal h)
{
    // Check if parent is currently laying out (in addition to our own layout engine)
    bool parentIsLayouting = false;
    if (parentElement()) {
        Frame* parentFrame = qobject_cast<Frame*>(parentElement());
        if (parentFrame && parentFrame->m_layoutEngine) {
            parentIsLayouting = parentFrame->m_layoutEngine->isLayouting();
        }
    }
    
    // Check if height resizing is allowed (but allow if layout engine is doing it)
    if (!canResizeHeight() && !parentIsLayouting && (!m_layoutEngine || !m_layoutEngine->isLayouting())) {
        return;
    }
    
    DesignElement::setHeight(h);
    
    // Only trigger layout if neither we nor our parent are in a layout operation
    if (!parentIsLayouting && (!m_layoutEngine || !m_layoutEngine->isLayouting())) {
        triggerLayout();
        
        // Also trigger layout on parent if it's a frame with flex
        if (parentElement() && m_position == Relative) {
            Frame* parentFrame = qobject_cast<Frame*>(parentElement());
            // Check if parent is currently laying out its children
            if (parentFrame && parentFrame->flex() && 
                (!parentFrame->m_layoutEngine || !parentFrame->m_layoutEngine->isLayouting())) {
                parentFrame->triggerLayout();
            }
        }
    }
}

void Frame::setRect(const QRectF &rect)
{
    // Check if parent is currently laying out (in addition to our own layout engine)
    bool parentIsLayouting = false;
    if (parentElement()) {
        Frame* parentFrame = qobject_cast<Frame*>(parentElement());
        if (parentFrame && parentFrame->m_layoutEngine) {
            parentIsLayouting = parentFrame->m_layoutEngine->isLayouting();
        }
    }
    
    // Create a modified rect that respects resize restrictions (but allow if layout engine is doing it)
    QRectF modifiedRect = rect;
    
    // If width resize is not allowed and layout engine is not doing it, keep the current width
    if (!canResizeWidth() && !parentIsLayouting && (!m_layoutEngine || !m_layoutEngine->isLayouting())) {
        modifiedRect.setWidth(width());
    }
    
    // If height resize is not allowed and layout engine is not doing it, keep the current height
    if (!canResizeHeight() && !parentIsLayouting && (!m_layoutEngine || !m_layoutEngine->isLayouting())) {
        modifiedRect.setHeight(height());
    }
    
    DesignElement::setRect(modifiedRect);
    
    // Only trigger layout if neither we nor our parent are in a layout operation
    if (!parentIsLayouting && (!m_layoutEngine || !m_layoutEngine->isLayouting())) {
        triggerLayout();
        
        // Also trigger layout on parent if it's a frame with flex
        if (parentElement() && m_position == Relative) {
            Frame* parentFrame = qobject_cast<Frame*>(parentElement());
            // Check if parent is currently laying out its children
            if (parentFrame && parentFrame->flex() && 
                (!parentFrame->m_layoutEngine || !parentFrame->m_layoutEngine->isLayouting())) {
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
                        // Only trigger layout if we're not already laying out
                        if (m_layoutEngine && !m_layoutEngine->isLayouting()) {
                            // Trigger layout when child properties change
                            triggerLayout();
                        }
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

QList<PropertyDefinition> Frame::propertyDefinitions() const {
    // Use the FrameFactory to get consistent property definitions
    static FrameFactory factory;
    return factory.propertyDefinitions();
}