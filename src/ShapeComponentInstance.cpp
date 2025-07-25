#include "ShapeComponentInstance.h"
#include "ShapeComponentVariant.h"
#include "Component.h"
#include "Element.h"
#include "Application.h"
#include "Scripts.h"

// Verify ShapeComponentVariant inherits from Element
static_assert(std::is_base_of<Element, ShapeComponentVariant>::value, "ShapeComponentVariant must inherit from Element");

ShapeComponentInstance::ShapeComponentInstance(const QString &id, QObject *parent)
    : Shape(id, parent)
    , ComponentInstance(id)
{
    elementType = Element::ShapeComponentInstanceType;
    // Component instances should have their own scripts
    initializeScripts(true);
}

ShapeComponentInstance::~ShapeComponentInstance() = default;

void ShapeComponentInstance::setInstanceOf(const QString &componentId)
{
    if (m_instanceOf != componentId) {
        m_instanceOf = componentId;
        emit instanceOfChanged();
    }
}

void ShapeComponentInstance::setSourceVariant(Element* variant)
{
    if (m_sourceVariant != variant) {
        m_connections.clear();
        m_sourceVariant = variant;
        
        if (m_sourceVariant) {
            connectToVariant();
            updateFromVariant();
        }
        
        emit sourceVariantChanged();
    }
}

QStringList ShapeComponentInstance::getEditableProperties() const
{
    if (auto* shapeVariant = qobject_cast<ShapeComponentVariant*>(m_sourceVariant)) {
        return shapeVariant->editableProperties();
    }
    return QStringList();
}

void ShapeComponentInstance::executeScriptEvent(const QString& eventName, const QVariantMap& eventData)
{
    // Component instances execute their own scripts
    DesignElement::executeScriptEvent(eventName, eventData);
}

void ShapeComponentInstance::setShapeType(ShapeType type)
{
    m_overriddenProperties.insert("shapeType");
    Shape::setShapeType(type);
}

void ShapeComponentInstance::setEdgeWidth(qreal width)
{
    m_overriddenProperties.insert("edgeWidth");
    Shape::setEdgeWidth(width);
}

void ShapeComponentInstance::setEdgeColor(const QColor &color)
{
    m_overriddenProperties.insert("edgeColor");
    Shape::setEdgeColor(color);
}

void ShapeComponentInstance::setFillColor(const QColor &color)
{
    m_overriddenProperties.insert("fillColor");
    Shape::setFillColor(color);
}


bool ShapeComponentInstance::setProperty(const char *name, const QVariant &value)
{
    QString propName(name);
    
    // Track property overrides for editable properties
    if (auto* shapeVariant = qobject_cast<ShapeComponentVariant*>(m_sourceVariant)) {
        if (shapeVariant->editableProperties().contains(propName)) {
            m_overriddenProperties.insert(propName);
        }
    }
    
    return Shape::setProperty(name, value);
}

void ShapeComponentInstance::connectToVariant()
{
    auto* shapeVariant = qobject_cast<ShapeComponentVariant*>(m_sourceVariant);
    if (!shapeVariant) return;
    
    // Connect to variant property changes
    m_connections.add(
        connect(shapeVariant, &QObject::destroyed,
                this, [this]() { setSourceVariant(nullptr); })
    );
    
    // Connect to specific shape property changes
    m_connections.add(
        connect(shapeVariant, &Shape::shapeTypeChanged,
                this, &ShapeComponentInstance::onVariantPropertyChanged)
    );
    m_connections.add(
        connect(shapeVariant, &Shape::edgeWidthChanged,
                this, &ShapeComponentInstance::onVariantPropertyChanged)
    );
    m_connections.add(
        connect(shapeVariant, &Shape::edgeColorChanged,
                this, &ShapeComponentInstance::onVariantPropertyChanged)
    );
    m_connections.add(
        connect(shapeVariant, &Shape::fillColorChanged,
                this, &ShapeComponentInstance::onVariantPropertyChanged)
    );
}

void ShapeComponentInstance::updateFromVariant()
{
    auto* shapeVariant = qobject_cast<ShapeComponentVariant*>(m_sourceVariant);
    if (!shapeVariant) return;
    
    // Only update properties that haven't been overridden
    if (!m_overriddenProperties.contains("shapeType")) {
        Shape::setShapeType(shapeVariant->shapeType());
    }
    if (!m_overriddenProperties.contains("edgeWidth")) {
        Shape::setEdgeWidth(shapeVariant->edgeWidth());
    }
    if (!m_overriddenProperties.contains("edgeColor")) {
        Shape::setEdgeColor(shapeVariant->edgeColor());
    }
    if (!m_overriddenProperties.contains("fillColor")) {
        Shape::setFillColor(shapeVariant->fillColor());
    }
}

void ShapeComponentInstance::onVariantPropertyChanged()
{
    updateFromVariant();
}