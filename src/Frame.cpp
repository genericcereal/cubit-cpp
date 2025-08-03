#include "Frame.h"
#include "FlexLayoutEngine.h"
#include "PropertyRegistry.h"
#include "ElementModel.h"
#include "Application.h"
#include "Project.h"
#include "SelectionManager.h"
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
    , m_elementModel(nullptr)
{
    // Set element type
    elementType = Element::FrameType;
    
    setName(QString("Frame %1").arg(id.right(4)));  // Use last 4 digits for display
    
    // Register properties - this will call parent's registerProperties too
    registerProperties();
    
    // Don't setup connections here - wait for setElementModel to be called
}

Frame::~Frame() 
{
    // Disconnect all child geometry signals
    if (m_layoutEngine) {
        m_layoutEngine->disconnectChildGeometrySignals(this);
    }
}

void Frame::setElementModel(ElementModel* model)
{
    m_elementModel = model;
    if (m_elementModel) {
        setupElementModelConnections();
    }
}


void Frame::setFill(const QColor &color)
{
    // Setting fill color
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

void Frame::setBorderStyle(const QString &style)
{
    if (m_borderStyle != style) {
        m_borderStyle = style;
        emit borderStyleChanged();
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
            // Use the stored element model
            if (m_elementModel) {
                m_layoutEngine->connectChildGeometrySignals(this, m_elementModel);
            }
            m_layoutEngine->scheduleLayout(this, m_elementModel);
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
            m_layoutEngine->scheduleLayout(this, m_elementModel);
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
            m_layoutEngine->scheduleLayout(this, m_elementModel, FlexLayoutEngine::GapChanged);
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
            m_layoutEngine->scheduleLayout(this, m_elementModel, FlexLayoutEngine::JustifyChanged);
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
            m_layoutEngine->scheduleLayout(this, m_elementModel, FlexLayoutEngine::AlignChanged);
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
        // unless it's already set to appContainer (for global frames)
        if (!platform.isEmpty() && m_role != appContainer) {
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
        m_layoutEngine->scheduleLayout(this, m_elementModel);
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
    // Use the stored element model
    if (m_elementModel) {
        // Setting up element model connections
        
        connect(m_elementModel, &ElementModel::elementAdded,
                this, [this](Element* element) {
                    if (element && element->getParentElementId() == this->getId()) {
                        if (m_flex && m_elementModel) {
                            // Reconnect signals when children are added
                            m_layoutEngine->disconnectChildGeometrySignals(this);
                            m_layoutEngine->connectChildGeometrySignals(this, m_elementModel);
                        }
                        triggerLayout();
                    }
                });
        connect(m_elementModel, &ElementModel::elementRemoved,
                this, [this](const QString&) {
                    // Check if removed element was our child
                    if (m_flex && m_elementModel) {
                        // Reconnect signals when children are removed
                        m_layoutEngine->disconnectChildGeometrySignals(this);
                        m_layoutEngine->connectChildGeometrySignals(this, m_elementModel);
                    }
                    // Trigger layout when any child is removed
                    triggerLayout();
                });
        connect(m_elementModel, &ElementModel::elementUpdated,
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
    }
}

QList<PropertyDefinition> Frame::propertyDefinitions() const {
    // Use the static method to get consistent property definitions
    return Frame::staticPropertyDefinitions();
}

void Frame::registerProperties() {
    // Call parent implementation first
    DesignElement::registerProperties();
    
    // Register Frame-specific properties
    m_properties->registerProperty("fill", QColor(173, 216, 230)); // Default light blue
    m_properties->registerProperty("colorFormat", static_cast<int>(HEX));
    m_properties->registerProperty("borderColor", QColor(Qt::black));
    m_properties->registerProperty("borderWidth", 1);
    m_properties->registerProperty("borderStyle", QString("Solid"));
    m_properties->registerProperty("borderRadius", 0);
    m_properties->registerProperty("overflow", static_cast<int>(Hidden));
    m_properties->registerProperty("acceptsChildren", true);
    m_properties->registerProperty("flex", false);
    m_properties->registerProperty("orientation", static_cast<int>(Row));
    m_properties->registerProperty("gap", 0.0);
    m_properties->registerProperty("position", static_cast<int>(Absolute));
    m_properties->registerProperty("justify", static_cast<int>(JustifyStart));
    m_properties->registerProperty("align", static_cast<int>(AlignStart));
    m_properties->registerProperty("widthType", static_cast<int>(SizeFixed));
    m_properties->registerProperty("heightType", static_cast<int>(SizeFixed));
    m_properties->registerProperty("controlled", true);
    m_properties->registerProperty("role", static_cast<int>(undefined));
    m_properties->registerProperty("platform", QString());
}

QVariant Frame::getProperty(const QString& name) const {
    // Handle Frame-specific properties that need special getters
    if (name == "fill") return fill();
    if (name == "colorFormat") return static_cast<int>(colorFormat());
    if (name == "borderColor") return borderColor();
    if (name == "borderWidth") return borderWidth();
    if (name == "borderStyle") return borderStyle();
    if (name == "borderRadius") return borderRadius();
    if (name == "overflow") return static_cast<int>(overflow());
    if (name == "acceptsChildren") return acceptsChildren();
    if (name == "flex") return flex();
    if (name == "orientation") return static_cast<int>(orientation());
    if (name == "gap") return gap();
    if (name == "position") return static_cast<int>(position());
    if (name == "justify") return static_cast<int>(justify());
    if (name == "align") return static_cast<int>(align());
    if (name == "widthType") return static_cast<int>(widthType());
    if (name == "heightType") return static_cast<int>(heightType());
    if (name == "canResizeWidth") return canResizeWidth();
    if (name == "canResizeHeight") return canResizeHeight();
    if (name == "controlled") return controlled();
    if (name == "role") return static_cast<int>(role());
    if (name == "platform") return platform();
    
    // Fall back to parent implementation
    return DesignElement::getProperty(name);
}

void Frame::setProperty(const QString& name, const QVariant& value) {
    // Handle Frame-specific properties that need special setters
    if (name == "fill") {
        setFill(value.value<QColor>());
        return;
    }
    if (name == "colorFormat") {
        setColorFormat(static_cast<ColorFormat>(value.toInt()));
        return;
    }
    if (name == "borderColor") {
        setBorderColor(value.value<QColor>());
        return;
    }
    if (name == "borderWidth") {
        setBorderWidth(value.toInt());
        return;
    }
    if (name == "borderStyle") {
        setBorderStyle(value.toString());
        return;
    }
    if (name == "borderRadius") {
        setBorderRadius(value.toInt());
        return;
    }
    if (name == "overflow") {
        setOverflow(static_cast<OverflowMode>(value.toInt()));
        return;
    }
    if (name == "acceptsChildren") {
        setAcceptsChildren(value.toBool());
        return;
    }
    if (name == "flex") {
        setFlex(value.toBool());
        return;
    }
    if (name == "orientation") {
        setOrientation(static_cast<LayoutOrientation>(value.toInt()));
        return;
    }
    if (name == "gap") {
        setGap(value.toReal());
        return;
    }
    if (name == "position") {
        setPosition(static_cast<PositionType>(value.toInt()));
        return;
    }
    if (name == "justify") {
        setJustify(static_cast<JustifyContent>(value.toInt()));
        return;
    }
    if (name == "align") {
        setAlign(static_cast<AlignItems>(value.toInt()));
        return;
    }
    if (name == "widthType") {
        setWidthType(static_cast<SizeType>(value.toInt()));
        return;
    }
    if (name == "heightType") {
        setHeightType(static_cast<SizeType>(value.toInt()));
        return;
    }
    if (name == "controlled") {
        setControlled(value.toBool());
        return;
    }
    if (name == "role") {
        setRole(static_cast<Role>(value.toInt()));
        return;
    }
    if (name == "platform") {
        setPlatform(value.toString());
        return;
    }
    
    // Fall back to parent implementation
    DesignElement::setProperty(name, value);
}

void Frame::triggerLayoutIfNeeded(const QString& propertyName) {
    // Properties that should trigger layout
    static const QStringList layoutProperties = {
        "flex", "orientation", "gap", "position", "justify", "align",
        "widthType", "heightType", "width", "height", "x", "y"
    };
    
    if (layoutProperties.contains(propertyName)) {
        triggerLayout();
    }
}

QList<PropertyDefinition> Frame::staticPropertyDefinitions() {
    QList<PropertyDefinition> props;
    
    // Appearance properties
    props.append(PropertyDefinition("fill", QMetaType::QColor, QColor(173, 216, 230), PropertyDefinition::Appearance));
    props.append(PropertyDefinition("borderColor", QMetaType::QColor, QColor(Qt::black), PropertyDefinition::Appearance));
    props.append(PropertyDefinition("borderWidth", QMetaType::Int, 0, PropertyDefinition::Appearance));
    props.append(PropertyDefinition("borderStyle", QMetaType::QString, QString("Solid"), PropertyDefinition::Appearance));
    props.append(PropertyDefinition("borderRadius", QMetaType::Int, 0, PropertyDefinition::Appearance));
    
    // Layout properties
    props.append(PropertyDefinition("flex", QMetaType::Bool, false, PropertyDefinition::Layout));
    props.append(PropertyDefinition("gap", QMetaType::Double, 0.0, PropertyDefinition::Layout));
    
    // Size properties
    props.append(PropertyDefinition("widthType", QMetaType::Int, Frame::SizeFixed, PropertyDefinition::Layout));
    props.append(PropertyDefinition("heightType", QMetaType::Int, Frame::SizeFixed, PropertyDefinition::Layout));
    
    // Position properties
    props.append(PropertyDefinition("position", QMetaType::Int, Frame::Absolute, PropertyDefinition::Layout));
    
    // Flex layout properties
    props.append(PropertyDefinition("orientation", QMetaType::Int, Frame::Row, PropertyDefinition::Layout));
    props.append(PropertyDefinition("justify", QMetaType::Int, Frame::JustifyStart, PropertyDefinition::Layout));
    props.append(PropertyDefinition("align", QMetaType::Int, Frame::AlignStart, PropertyDefinition::Layout));
    
    // Behavior properties
    props.append(PropertyDefinition("acceptsChildren", QMetaType::Bool, true, PropertyDefinition::Behavior));
    props.append(PropertyDefinition("controlled", QMetaType::Bool, true, PropertyDefinition::Behavior));
    props.append(PropertyDefinition("overflow", QMetaType::Int, Frame::Hidden, PropertyDefinition::Behavior));
    
    // Advanced properties
    props.append(PropertyDefinition("platform", QMetaType::QString, QString(), PropertyDefinition::Advanced));
    props.append(PropertyDefinition("role", QMetaType::Int, Frame::undefined, PropertyDefinition::Advanced));
    
    // Add enum properties
    const QMetaObject* metaObj = &Frame::staticMetaObject;
    for (int i = 0; i < metaObj->enumeratorCount(); ++i) {
        QMetaEnum metaEnum = metaObj->enumerator(i);
        QString enumName = metaEnum.name();
        
        // Map enum names to property names
        if (enumName == "ColorFormat") {
            props.append(PropertyDefinition("colorFormat", metaEnum, Frame::HEX, PropertyDefinition::Appearance));
        }
    }
    
    return props;
}

QStringList Frame::getChildElements() const {
    QStringList childIds;
    
    if (m_elementModel) {
        // Get all elements from the model
        for (int i = 0; i < m_elementModel->rowCount(); ++i) {
            Element* element = m_elementModel->elementAt(i);
            if (element && element->getParentElementId() == getId()) {
                childIds.append(element->getId());
            }
        }
    }
    
    return childIds;
}