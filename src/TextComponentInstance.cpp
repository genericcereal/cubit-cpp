#include "TextComponentInstance.h"
#include "Component.h"
#include "TextVariant.h"
#include "ElementModel.h"
#include "Application.h"
#include <QDebug>

TextComponentInstance::TextComponentInstance(const QString &id, QObject *parent)
    : Text(id, parent)
    , ComponentInstance(id)
{
    // Set element type
    elementType = Element::FrameComponentInstanceType;
    setName("TextInstance");
    
    // Re-initialize scripts with ComponentInstance flag
    initializeScripts(true);
}

TextComponentInstance::~TextComponentInstance()
{
    m_isDestructing = true;
    disconnectFromVariant();
    disconnectFromComponent();
}

void TextComponentInstance::setInstanceOf(const QString &componentId)
{
    if (m_instanceOf != componentId) {
        m_instanceOf = componentId;
        
        // Disconnect from previous component
        disconnectFromComponent();
        disconnectFromVariant();
        
        // Connect to new component
        connectToComponent();
        
        emit instanceOfChanged();
    }
}

void TextComponentInstance::setSourceVariant(Element* variant)
{
    if (m_sourceVariant == variant) {
        return;
    }
    
    // Verify the variant belongs to our component
    if (variant && m_component) {
        bool found = false;
        for (Element* v : m_component->variants()) {
            if (v == variant) {
                found = true;
                break;
            }
        }
        if (!found) {
            qWarning() << "TextComponentInstance::setSourceVariant - variant does not belong to component";
            return;
        }
    }
    
    // Disconnect from current variant
    disconnectFromVariant();
    
    // Set new variant
    m_sourceVariant = variant;
    
    if (m_sourceVariant) {
        // Sync properties from new variant
        syncPropertiesFromVariant();
        
        // Connect to track changes
        if (TextVariant* textVariant = qobject_cast<TextVariant*>(m_sourceVariant)) {
            // Connect to property changes
            m_variantConnections.add(
                connect(textVariant, &TextVariant::contentChanged,
                        this, &TextComponentInstance::onSourceVariantPropertyChanged,
                        Qt::UniqueConnection)
            );
            m_variantConnections.add(
                connect(textVariant, &TextVariant::fontChanged,
                        this, &TextComponentInstance::onSourceVariantPropertyChanged,
                        Qt::UniqueConnection)
            );
            m_variantConnections.add(
                connect(textVariant, &TextVariant::colorChanged,
                        this, &TextComponentInstance::onSourceVariantPropertyChanged,
                        Qt::UniqueConnection)
            );
        }
    }
    
    emit sourceVariantChanged();
}

QStringList TextComponentInstance::getEditableProperties() const
{
    if (m_sourceVariant) {
        if (TextVariant* textVariant = qobject_cast<TextVariant*>(m_sourceVariant)) {
            return textVariant->editableProperties();
        }
    }
    return QStringList();
}

void TextComponentInstance::setContent(const QString &content)
{
    Text::setContent(content);
    // Don't track content as modified - always sync from variant
}

void TextComponentInstance::setFont(const QFont &font)
{
    Text::setFont(font);
    // Don't track font as modified - always sync from variant
}

void TextComponentInstance::setColor(const QColor &color)
{
    Text::setColor(color);
    m_modifiedProperties.insert("color");
}

void TextComponentInstance::setPosition(PositionType position)
{
    Text::setPosition(position);
    m_modifiedProperties.insert("position");
}

void TextComponentInstance::onSourceVariantPropertyChanged()
{
    // This method is no longer used - we connect directly to specific signals
}

void TextComponentInstance::onComponentVariantsChanged()
{
    if (m_isDestructing) return;
    
    // Re-connect to the appropriate variant
    disconnectFromVariant();
    connectToVariant();
}

void TextComponentInstance::connectToComponent()
{
    if (m_instanceOf.isEmpty()) return;
    
    // Find the component in the element model
    Application* app = Application::instance();
    if (!app || !app->activeCanvas() || !app->activeCanvas()->elementModel()) return;
    
    ElementModel* model = app->activeCanvas()->elementModel();
    Element* element = model->getElementById(m_instanceOf);
    
    if (Component* component = qobject_cast<Component*>(element)) {
        m_component = component;
        
        // Connect to variants changed signal
        m_componentConnections.add(
            connect(m_component, &Component::variantsChanged,
                    this, &TextComponentInstance::onComponentVariantsChanged,
                    Qt::UniqueConnection)
        );
        
        // Initial variant connection
        connectToVariant();
    }
}

void TextComponentInstance::disconnectFromComponent()
{
    m_componentConnections.clear();
    m_component = nullptr;
}

void TextComponentInstance::connectToVariant()
{
    if (!m_component) {
        return;
    }
    
    QList<Element*> variants = m_component->variants();
    
    if (!variants.isEmpty()) {
        // If we don't already have a source variant, use the first one
        if (!m_sourceVariant) {
            m_sourceVariant = variants.first();
        } else {
            // Verify that our current source variant still exists in the component
            bool found = false;
            for (Element* variant : variants) {
                if (variant == m_sourceVariant) {
                    found = true;
                    break;
                }
            }
            if (!found) {
                // Our variant was removed, fall back to first available
                m_sourceVariant = variants.first();
            }
        }
        
        // Clear modified properties when connecting to a new variant
        m_modifiedProperties.clear();
        
        // Sync initial properties
        syncPropertiesFromVariant();
        
        // Connect to property changes
        if (m_sourceVariant) {
            // For TextVariant, connect directly to contentChanged signal
            if (TextVariant* textVariant = qobject_cast<TextVariant*>(m_sourceVariant)) {
                // Note: Can't use Qt::UniqueConnection with lambdas, so we'll just use a regular connection
                QMetaObject::Connection conn = connect(textVariant, &TextVariant::contentChanged,
                        this, [this, textVariant]() {
                            setContent(textVariant->content());
                        });
                
                if (conn) {
                    m_variantConnections.add(conn);
                }
                
                // Also connect other properties
                QMetaObject::Connection fontConn = connect(textVariant, &TextVariant::fontChanged,
                        this, [this, textVariant]() {
                            // Always update font from variant
                            Text::setFont(textVariant->font());
                        });
                
                if (fontConn) {
                    m_variantConnections.add(fontConn);
                }
                
                m_variantConnections.add(
                    connect(textVariant, &TextVariant::colorChanged,
                            this, [this, textVariant]() {
                                if (!m_modifiedProperties.contains("color")) {
                                    // Call the base class setColor directly to avoid marking as modified
                                    Text::setColor(textVariant->color());
                                }
                            }, Qt::UniqueConnection)
                );
            }
        }
        
        emit sourceVariantChanged();
    }
}

void TextComponentInstance::disconnectFromVariant()
{
    m_variantConnections.clear();
    m_sourceVariant = nullptr;
}

void TextComponentInstance::syncPropertiesFromVariant()
{
    if (!m_sourceVariant) return;
    
    if (TextVariant* textVariant = qobject_cast<TextVariant*>(m_sourceVariant)) {
        // Sync Text-specific properties
        // Always sync content from variant
        setContent(textVariant->content());
        if (!m_modifiedProperties.contains("font")) {
            Text::setFont(textVariant->font());
        }
        if (!m_modifiedProperties.contains("color")) {
            Text::setColor(textVariant->color());
        }
        if (!m_modifiedProperties.contains("position")) {
            Text::setPosition(textVariant->position());
        }
        
        // Note: We don't sync position (x, y) as instances maintain their own position
        // Only sync size
        setWidth(textVariant->width());
        setHeight(textVariant->height());
    }
}

void TextComponentInstance::executeScriptEvent(const QString& eventName)
{
    // ComponentInstances should execute their own scripts, not their source variant's scripts
    // Call the parent class implementation directly
    Text::executeScriptEvent(eventName);
}