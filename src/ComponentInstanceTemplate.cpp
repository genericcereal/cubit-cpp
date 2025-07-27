#include "ComponentInstanceTemplate.h"
#include "ComponentVariant.h"
#include "ComponentVariantTemplate.h"
#include "Component.h"
#include "ElementModel.h"
#include <QDebug>

// FrameComponentInstanceTemplate implementation
FrameComponentInstanceTemplate::FrameComponentInstanceTemplate(const QString &id, QObject *parent)
    : Frame(id, parent), ComponentInstance(id)
{
}

FrameComponentInstanceTemplate::~FrameComponentInstanceTemplate()
{
    m_isDestroying = true;
    disconnectFromSourceElement();
}

void FrameComponentInstanceTemplate::setSourceElementId(const QString &elementId)
{
    if (m_sourceElementId != elementId) {
        disconnectFromSourceElement();
        m_sourceElementId = elementId;
        connectToSourceElement();
        updateComponentInfo();
        emit sourceElementIdChanged();
    }
}

Component* FrameComponentInstanceTemplate::sourceComponent() const
{
    if (!m_isSourceVariant || m_componentId.isEmpty()) {
        return nullptr;
    }
    
    // Look up the component dynamically
    ElementModel* elementModel = qobject_cast<ElementModel*>(parent());
    if (!elementModel) {
        return nullptr;
    }
    
    Element* elem = elementModel->getElementById(m_componentId);
    return qobject_cast<Component*>(elem);
}

void FrameComponentInstanceTemplate::setSourceVariant(Element* variant)
{
    if (!variant) return;
    
    // Only accept variants
    if (!qobject_cast<FrameComponentVariantTemplate*>(variant)) return;
    
    setSourceElementId(variant->getId());
}

QStringList FrameComponentInstanceTemplate::getEditableProperties() const
{
    if (m_isSourceVariant && m_sourceElement) {
        if (auto* variant = qobject_cast<FrameComponentVariantTemplate*>(m_sourceElement.data())) {
            return variant->editableProperties();
        }
    }
    return QStringList();
}

void FrameComponentInstanceTemplate::executeScriptEvent(const QString& eventName, const QVariantMap& eventData)
{
    // Execute the instance's own scripts
    Frame::executeScriptEvent(eventName, eventData);
}

void FrameComponentInstanceTemplate::onSourcePropertyChanged()
{
    if (!m_isDestroying && m_sourceElement && !m_sourceElement.isNull()) {
        updateFromSourceElement();
    }
}

void FrameComponentInstanceTemplate::onSourceGeometryChanged()
{
    if (!m_isDestroying && m_sourceElement && !m_sourceElement.isNull()) {
        updateFromSourceElement();
    }
}

void FrameComponentInstanceTemplate::connectToSourceElement()
{
    if (m_sourceElementId.isEmpty()) return;
    
    // Find the source element in the element model
    ElementModel* elementModel = qobject_cast<ElementModel*>(parent());
    if (!elementModel) {
        return;
    }
    
    Element* element = elementModel->getElementById(m_sourceElementId);
    if (!element) {
        return;
    }
    
    Frame* sourceFrame = qobject_cast<Frame*>(element);
    if (!sourceFrame) {
        return;
    }
    
    m_sourceElement = sourceFrame;
    
    // Check if it's a variant
    m_isSourceVariant = qobject_cast<FrameComponentVariantTemplate*>(sourceFrame) != nullptr;
    
    // Connect to destroyed signal to handle source element deletion
    connect(sourceFrame, &QObject::destroyed, this, [this]() {
        // QPointer will automatically become null, just clear our tracking variables
        m_sourceElementId.clear();
        m_isSourceVariant = false;
        m_componentId.clear();
        emit sourceElementIdChanged();
        emit sourceVariantChanged();
        emit componentIdChanged();
    });
    
    // Connect to all property change signals
    connect(sourceFrame, &Frame::fillChanged, this, &FrameComponentInstanceTemplate::onSourcePropertyChanged, Qt::UniqueConnection);
    connect(sourceFrame, &Frame::borderColorChanged, this, &FrameComponentInstanceTemplate::onSourcePropertyChanged, Qt::UniqueConnection);
    connect(sourceFrame, &Frame::borderWidthChanged, this, &FrameComponentInstanceTemplate::onSourcePropertyChanged, Qt::UniqueConnection);
    connect(sourceFrame, &Frame::borderRadiusChanged, this, &FrameComponentInstanceTemplate::onSourcePropertyChanged, Qt::UniqueConnection);
    connect(sourceFrame, &Frame::overflowChanged, this, &FrameComponentInstanceTemplate::onSourcePropertyChanged, Qt::UniqueConnection);
    
    // Connect to geometry changes
    connect(sourceFrame, &Frame::widthChanged, this, &FrameComponentInstanceTemplate::onSourceGeometryChanged, Qt::UniqueConnection);
    connect(sourceFrame, &Frame::heightChanged, this, &FrameComponentInstanceTemplate::onSourceGeometryChanged, Qt::UniqueConnection);
    
    // Notify about variant change
    emit sourceVariantChanged();
    
    // Defer initial update to avoid issues during construction
    QMetaObject::invokeMethod(this, &FrameComponentInstanceTemplate::updateFromSourceElement, Qt::QueuedConnection);
}

void FrameComponentInstanceTemplate::disconnectFromSourceElement()
{
    if (m_sourceElement) {
        disconnect(m_sourceElement, nullptr, this, nullptr);
    }
    m_isSourceVariant = false;
    m_componentId.clear();
}

void FrameComponentInstanceTemplate::updateFromSourceElement()
{
    if (m_isDestroying || !m_sourceElement || m_sourceElement.isNull()) return;
    
    // Block signals to prevent infinite loops
    blockSignals(true);
    
    // Copy all properties from source
    setFill(m_sourceElement->fill());
    setBorderColor(m_sourceElement->borderColor());
    setBorderWidth(m_sourceElement->borderWidth());
    setBorderRadius(m_sourceElement->borderRadius());
    setOverflow(m_sourceElement->overflow());
    
    // Copy size (but not position)
    setWidth(m_sourceElement->width());
    setHeight(m_sourceElement->height());
    
    blockSignals(false);
}

void FrameComponentInstanceTemplate::updateComponentInfo()
{
    m_componentId.clear();
    
    if (!m_isSourceVariant || !m_sourceElement) {
        emit componentIdChanged();
        return;
    }
    
    // Find the component that owns this variant
    ElementModel* elementModel = qobject_cast<ElementModel*>(parent());
    if (!elementModel) {
        emit componentIdChanged();
        return;
    }
    
    // Search all components for one that contains this variant
    QList<Element*> allElements = elementModel->getAllElements();
    for (Element* elem : allElements) {
        Component* comp = qobject_cast<Component*>(elem);
        if (comp && comp->variants().contains(m_sourceElement)) {
            m_componentId = comp->getId();
            
            break;
        }
    }
    
    emit componentIdChanged();
}

// TextComponentInstanceTemplate implementation
TextComponentInstanceTemplate::TextComponentInstanceTemplate(const QString &id, QObject *parent)
    : Text(id, parent), ComponentInstance(id)
{
}

TextComponentInstanceTemplate::~TextComponentInstanceTemplate()
{
    m_isDestroying = true;
    disconnectFromSourceElement();
}

void TextComponentInstanceTemplate::setSourceElementId(const QString &elementId)
{
    if (m_sourceElementId != elementId) {
        disconnectFromSourceElement();
        m_sourceElementId = elementId;
        connectToSourceElement();
        updateComponentInfo();
        emit sourceElementIdChanged();
    }
}

Component* TextComponentInstanceTemplate::sourceComponent() const
{
    if (!m_isSourceVariant || m_componentId.isEmpty()) {
        return nullptr;
    }
    
    // Look up the component dynamically
    ElementModel* elementModel = qobject_cast<ElementModel*>(parent());
    if (!elementModel) {
        return nullptr;
    }
    
    Element* elem = elementModel->getElementById(m_componentId);
    return qobject_cast<Component*>(elem);
}

void TextComponentInstanceTemplate::setSourceVariant(Element* variant)
{
    if (!variant) return;
    
    // Only accept variants
    if (!qobject_cast<TextComponentVariantTemplate*>(variant)) return;
    
    setSourceElementId(variant->getId());
}

QStringList TextComponentInstanceTemplate::getEditableProperties() const
{
    if (m_isSourceVariant && m_sourceElement) {
        if (auto* variant = qobject_cast<TextComponentVariantTemplate*>(m_sourceElement.data())) {
            return variant->editableProperties();
        }
    }
    return QStringList();
}

void TextComponentInstanceTemplate::executeScriptEvent(const QString& eventName, const QVariantMap& eventData)
{
    // Execute the instance's own scripts
    Text::executeScriptEvent(eventName, eventData);
}

void TextComponentInstanceTemplate::onSourcePropertyChanged()
{
    if (!m_isDestroying && m_sourceElement && !m_sourceElement.isNull()) {
        updateFromSourceElement();
    }
}

void TextComponentInstanceTemplate::onSourceGeometryChanged()
{
    if (!m_isDestroying && m_sourceElement && !m_sourceElement.isNull()) {
        updateFromSourceElement();
    }
}

void TextComponentInstanceTemplate::connectToSourceElement()
{
    if (m_sourceElementId.isEmpty()) return;
    
    // Find the source element in the element model
    ElementModel* elementModel = qobject_cast<ElementModel*>(parent());
    if (!elementModel) return;
    
    Element* element = elementModel->getElementById(m_sourceElementId);
    Text* sourceText = qobject_cast<Text*>(element);
    if (!sourceText) return;
    
    m_sourceElement = sourceText;
    
    // Check if it's a variant
    m_isSourceVariant = qobject_cast<TextComponentVariantTemplate*>(sourceText) != nullptr;
    
    // Connect to destroyed signal to handle source element deletion
    connect(sourceText, &QObject::destroyed, this, [this]() {
        // QPointer will automatically become null, just clear our tracking variables
        m_sourceElementId.clear();
        m_isSourceVariant = false;
        m_componentId.clear();
        emit sourceElementIdChanged();
        emit sourceVariantChanged();
        emit componentIdChanged();
    });
    
    // Connect to all property change signals
    connect(sourceText, &Text::contentChanged, this, &TextComponentInstanceTemplate::onSourcePropertyChanged, Qt::UniqueConnection);
    connect(sourceText, &Text::fontChanged, this, &TextComponentInstanceTemplate::onSourcePropertyChanged, Qt::UniqueConnection);
    connect(sourceText, &Text::colorChanged, this, &TextComponentInstanceTemplate::onSourcePropertyChanged, Qt::UniqueConnection);
    
    // Connect to geometry changes
    connect(sourceText, &Text::widthChanged, this, &TextComponentInstanceTemplate::onSourceGeometryChanged, Qt::UniqueConnection);
    connect(sourceText, &Text::heightChanged, this, &TextComponentInstanceTemplate::onSourceGeometryChanged, Qt::UniqueConnection);
    
    // Notify about variant change
    emit sourceVariantChanged();
    
    // Defer initial update to avoid issues during construction
    QMetaObject::invokeMethod(this, &TextComponentInstanceTemplate::updateFromSourceElement, Qt::QueuedConnection);
}

void TextComponentInstanceTemplate::disconnectFromSourceElement()
{
    if (m_sourceElement) {
        disconnect(m_sourceElement, nullptr, this, nullptr);
    }
    m_isSourceVariant = false;
    m_componentId.clear();
}

void TextComponentInstanceTemplate::updateFromSourceElement()
{
    if (m_isDestroying || !m_sourceElement || m_sourceElement.isNull()) return;
    
    // Block signals to prevent infinite loops
    blockSignals(true);
    
    // Copy all properties from source
    setContent(m_sourceElement->content());
    setFont(m_sourceElement->font());
    setColor(m_sourceElement->color());
    
    // Copy size (but not position)
    setWidth(m_sourceElement->width());
    setHeight(m_sourceElement->height());
    
    blockSignals(false);
}

void TextComponentInstanceTemplate::updateComponentInfo()
{
    m_componentId.clear();
    
    if (!m_isSourceVariant || !m_sourceElement) {
        emit componentIdChanged();
        return;
    }
    
    // Find the component that owns this variant
    ElementModel* elementModel = qobject_cast<ElementModel*>(parent());
    if (!elementModel) {
        emit componentIdChanged();
        return;
    }
    
    // Search all components for one that contains this variant
    QList<Element*> allElements = elementModel->getAllElements();
    for (Element* elem : allElements) {
        Component* comp = qobject_cast<Component*>(elem);
        if (comp && comp->variants().contains(m_sourceElement)) {
            m_componentId = comp->getId();
            
            break;
        }
    }
    
    emit componentIdChanged();
}

// ShapeComponentInstanceTemplate implementation
ShapeComponentInstanceTemplate::ShapeComponentInstanceTemplate(const QString &id, QObject *parent)
    : Shape(id, parent), ComponentInstance(id)
{
}

ShapeComponentInstanceTemplate::~ShapeComponentInstanceTemplate()
{
    m_isDestroying = true;
    disconnectFromSourceElement();
}

void ShapeComponentInstanceTemplate::setSourceElementId(const QString &elementId)
{
    if (m_sourceElementId != elementId) {
        disconnectFromSourceElement();
        m_sourceElementId = elementId;
        connectToSourceElement();
        updateComponentInfo();
        emit sourceElementIdChanged();
    }
}

Component* ShapeComponentInstanceTemplate::sourceComponent() const
{
    if (!m_isSourceVariant || m_componentId.isEmpty()) {
        return nullptr;
    }
    
    // Look up the component dynamically
    ElementModel* elementModel = qobject_cast<ElementModel*>(parent());
    if (!elementModel) {
        return nullptr;
    }
    
    Element* elem = elementModel->getElementById(m_componentId);
    return qobject_cast<Component*>(elem);
}

void ShapeComponentInstanceTemplate::setSourceVariant(Element* variant)
{
    if (!variant) return;
    
    // Only accept variants
    if (!qobject_cast<ShapeComponentVariantTemplate*>(variant)) return;
    
    setSourceElementId(variant->getId());
}

QStringList ShapeComponentInstanceTemplate::getEditableProperties() const
{
    if (m_isSourceVariant && m_sourceElement) {
        if (auto* variant = qobject_cast<ShapeComponentVariantTemplate*>(m_sourceElement.data())) {
            return variant->editableProperties();
        }
    }
    return QStringList();
}

void ShapeComponentInstanceTemplate::executeScriptEvent(const QString& eventName, const QVariantMap& eventData)
{
    // Execute the instance's own scripts
    Shape::executeScriptEvent(eventName, eventData);
}

void ShapeComponentInstanceTemplate::onSourcePropertyChanged()
{
    if (!m_isDestroying && m_sourceElement && !m_sourceElement.isNull()) {
        updateFromSourceElement();
    }
}

void ShapeComponentInstanceTemplate::onSourceGeometryChanged()
{
    if (!m_isDestroying && m_sourceElement && !m_sourceElement.isNull()) {
        updateFromSourceElement();
    }
}

void ShapeComponentInstanceTemplate::connectToSourceElement()
{
    if (m_sourceElementId.isEmpty()) return;
    
    // Find the source element in the element model
    ElementModel* elementModel = qobject_cast<ElementModel*>(parent());
    if (!elementModel) return;
    
    Element* element = elementModel->getElementById(m_sourceElementId);
    Shape* sourceShape = qobject_cast<Shape*>(element);
    if (!sourceShape) return;
    
    m_sourceElement = sourceShape;
    
    // Check if it's a variant
    m_isSourceVariant = qobject_cast<ShapeComponentVariantTemplate*>(sourceShape) != nullptr;
    
    // Connect to destroyed signal to handle source element deletion
    connect(sourceShape, &QObject::destroyed, this, [this]() {
        // QPointer will automatically become null, just clear our tracking variables
        m_sourceElementId.clear();
        m_isSourceVariant = false;
        m_componentId.clear();
        emit sourceElementIdChanged();
        emit sourceVariantChanged();
        emit componentIdChanged();
    });
    
    // Connect to all property change signals
    connect(sourceShape, &Shape::shapeTypeChanged, this, &ShapeComponentInstanceTemplate::onSourcePropertyChanged, Qt::UniqueConnection);
    connect(sourceShape, &Shape::fillColorChanged, this, &ShapeComponentInstanceTemplate::onSourcePropertyChanged, Qt::UniqueConnection);
    connect(sourceShape, &Shape::edgeColorChanged, this, &ShapeComponentInstanceTemplate::onSourcePropertyChanged, Qt::UniqueConnection);
    connect(sourceShape, &Shape::edgeWidthChanged, this, &ShapeComponentInstanceTemplate::onSourcePropertyChanged, Qt::UniqueConnection);
    
    // Connect to geometry changes
    connect(sourceShape, &Shape::widthChanged, this, &ShapeComponentInstanceTemplate::onSourceGeometryChanged, Qt::UniqueConnection);
    connect(sourceShape, &Shape::heightChanged, this, &ShapeComponentInstanceTemplate::onSourceGeometryChanged, Qt::UniqueConnection);
    
    // Notify about variant change
    emit sourceVariantChanged();
    
    // Defer initial update to avoid issues during construction
    QMetaObject::invokeMethod(this, &ShapeComponentInstanceTemplate::updateFromSourceElement, Qt::QueuedConnection);
}

void ShapeComponentInstanceTemplate::disconnectFromSourceElement()
{
    if (m_sourceElement) {
        disconnect(m_sourceElement, nullptr, this, nullptr);
    }
    m_isSourceVariant = false;
    m_componentId.clear();
}

void ShapeComponentInstanceTemplate::updateFromSourceElement()
{
    if (m_isDestroying || !m_sourceElement || m_sourceElement.isNull()) return;
    
    // Block signals to prevent infinite loops
    blockSignals(true);
    
    // Copy all properties from source
    setShapeType(m_sourceElement->shapeType());
    setFillColor(m_sourceElement->fillColor());
    setEdgeColor(m_sourceElement->edgeColor());
    setEdgeWidth(m_sourceElement->edgeWidth());
    
    // Copy size (but not position)
    setWidth(m_sourceElement->width());
    setHeight(m_sourceElement->height());
    
    blockSignals(false);
}

void ShapeComponentInstanceTemplate::updateComponentInfo()
{
    m_componentId.clear();
    
    if (!m_isSourceVariant || !m_sourceElement) {
        emit componentIdChanged();
        return;
    }
    
    // Find the component that owns this variant
    ElementModel* elementModel = qobject_cast<ElementModel*>(parent());
    if (!elementModel) {
        emit componentIdChanged();
        return;
    }
    
    // Search all components for one that contains this variant
    QList<Element*> allElements = elementModel->getAllElements();
    for (Element* elem : allElements) {
        Component* comp = qobject_cast<Component*>(elem);
        if (comp && comp->variants().contains(m_sourceElement)) {
            m_componentId = comp->getId();
            
            break;
        }
    }
    
    emit componentIdChanged();
}

// WebTextInputComponentInstanceTemplate implementation
WebTextInputComponentInstanceTemplate::WebTextInputComponentInstanceTemplate(const QString &id, QObject *parent)
    : WebTextInput(id, parent), ComponentInstance(id)
{
}

WebTextInputComponentInstanceTemplate::~WebTextInputComponentInstanceTemplate()
{
    m_isDestroying = true;
    disconnectFromSourceElement();
}

void WebTextInputComponentInstanceTemplate::setSourceElementId(const QString &elementId)
{
    if (m_sourceElementId != elementId) {
        disconnectFromSourceElement();
        m_sourceElementId = elementId;
        connectToSourceElement();
        updateComponentInfo();
        emit sourceElementIdChanged();
    }
}

Component* WebTextInputComponentInstanceTemplate::sourceComponent() const
{
    if (!m_isSourceVariant || m_componentId.isEmpty()) {
        return nullptr;
    }
    
    // Look up the component dynamically
    ElementModel* elementModel = qobject_cast<ElementModel*>(parent());
    if (!elementModel) {
        return nullptr;
    }
    
    Element* elem = elementModel->getElementById(m_componentId);
    return qobject_cast<Component*>(elem);
}

void WebTextInputComponentInstanceTemplate::setSourceVariant(Element* variant)
{
    if (!variant) return;
    
    // Only accept variants
    if (!qobject_cast<WebTextInputComponentVariantTemplate*>(variant)) return;
    
    setSourceElementId(variant->getId());
}

QStringList WebTextInputComponentInstanceTemplate::getEditableProperties() const
{
    if (m_isSourceVariant && m_sourceElement) {
        if (auto* variant = qobject_cast<WebTextInputComponentVariantTemplate*>(m_sourceElement.data())) {
            return variant->editableProperties();
        }
    }
    return QStringList();
}

void WebTextInputComponentInstanceTemplate::executeScriptEvent(const QString& eventName, const QVariantMap& eventData)
{
    // Execute the instance's own scripts
    WebTextInput::executeScriptEvent(eventName, eventData);
}

void WebTextInputComponentInstanceTemplate::onSourcePropertyChanged()
{
    if (!m_isDestroying && m_sourceElement && !m_sourceElement.isNull()) {
        updateFromSourceElement();
    }
}

void WebTextInputComponentInstanceTemplate::onSourceGeometryChanged()
{
    if (!m_isDestroying && m_sourceElement && !m_sourceElement.isNull()) {
        updateFromSourceElement();
    }
}

void WebTextInputComponentInstanceTemplate::connectToSourceElement()
{
    if (m_sourceElementId.isEmpty()) return;
    
    // Find the source element in the element model
    ElementModel* elementModel = qobject_cast<ElementModel*>(parent());
    if (!elementModel) return;
    
    Element* element = elementModel->getElementById(m_sourceElementId);
    WebTextInput* sourceWebTextInput = qobject_cast<WebTextInput*>(element);
    if (!sourceWebTextInput) return;
    
    m_sourceElement = sourceWebTextInput;
    
    // Check if it's a variant
    m_isSourceVariant = qobject_cast<WebTextInputComponentVariantTemplate*>(sourceWebTextInput) != nullptr;
    
    // Connect to destroyed signal to handle source element deletion
    connect(sourceWebTextInput, &QObject::destroyed, this, [this]() {
        // QPointer will automatically become null, just clear our tracking variables
        m_sourceElementId.clear();
        m_isSourceVariant = false;
        m_componentId.clear();
        emit sourceElementIdChanged();
        emit sourceVariantChanged();
        emit componentIdChanged();
    });
    
    // Connect to all property change signals
    connect(sourceWebTextInput, &WebTextInput::valueChanged, this, &WebTextInputComponentInstanceTemplate::onSourcePropertyChanged, Qt::UniqueConnection);
    connect(sourceWebTextInput, &WebTextInput::placeholderChanged, this, &WebTextInputComponentInstanceTemplate::onSourcePropertyChanged, Qt::UniqueConnection);
    connect(sourceWebTextInput, &WebTextInput::borderColorChanged, this, &WebTextInputComponentInstanceTemplate::onSourcePropertyChanged, Qt::UniqueConnection);
    connect(sourceWebTextInput, &WebTextInput::borderWidthChanged, this, &WebTextInputComponentInstanceTemplate::onSourcePropertyChanged, Qt::UniqueConnection);
    connect(sourceWebTextInput, &WebTextInput::borderRadiusChanged, this, &WebTextInputComponentInstanceTemplate::onSourcePropertyChanged, Qt::UniqueConnection);
    
    // Connect to geometry changes
    connect(sourceWebTextInput, &WebTextInput::widthChanged, this, &WebTextInputComponentInstanceTemplate::onSourceGeometryChanged, Qt::UniqueConnection);
    connect(sourceWebTextInput, &WebTextInput::heightChanged, this, &WebTextInputComponentInstanceTemplate::onSourceGeometryChanged, Qt::UniqueConnection);
    
    // Notify about variant change
    emit sourceVariantChanged();
    
    // Defer initial update to avoid issues during construction
    QMetaObject::invokeMethod(this, &WebTextInputComponentInstanceTemplate::updateFromSourceElement, Qt::QueuedConnection);
}

void WebTextInputComponentInstanceTemplate::disconnectFromSourceElement()
{
    if (m_sourceElement) {
        disconnect(m_sourceElement, nullptr, this, nullptr);
    }
    m_isSourceVariant = false;
    m_componentId.clear();
}

void WebTextInputComponentInstanceTemplate::updateFromSourceElement()
{
    if (m_isDestroying || !m_sourceElement || m_sourceElement.isNull()) return;
    
    // Block signals to prevent infinite loops
    blockSignals(true);
    
    // Copy all properties from source
    setValue(m_sourceElement->value());
    setPlaceholder(m_sourceElement->placeholder());
    setBorderColor(m_sourceElement->borderColor());
    setBorderWidth(m_sourceElement->borderWidth());
    setBorderRadius(m_sourceElement->borderRadius());
    
    // Copy size (but not position)
    setWidth(m_sourceElement->width());
    setHeight(m_sourceElement->height());
    
    blockSignals(false);
}

void WebTextInputComponentInstanceTemplate::updateComponentInfo()
{
    m_componentId.clear();
    
    if (!m_isSourceVariant || !m_sourceElement) {
        emit componentIdChanged();
        return;
    }
    
    // Find the component that owns this variant
    ElementModel* elementModel = qobject_cast<ElementModel*>(parent());
    if (!elementModel) {
        emit componentIdChanged();
        return;
    }
    
    // Search all components for one that contains this variant
    QList<Element*> allElements = elementModel->getAllElements();
    for (Element* elem : allElements) {
        Component* comp = qobject_cast<Component*>(elem);
        if (comp && comp->variants().contains(m_sourceElement)) {
            m_componentId = comp->getId();
            
            break;
        }
    }
    
    emit componentIdChanged();
}