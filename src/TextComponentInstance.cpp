#include "TextComponentInstance.h"
#include "Component.h"
#include "TextVariant.h"
#include "ElementModel.h"
#include "Application.h"
#include <QDebug>

TextComponentInstance::TextComponentInstance(const QString &id, QObject *parent)
    : Text(id, parent)
{
    // Set element type
    elementType = Element::FrameComponentInstanceType;
    setName("TextInstance");
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
    m_modifiedProperties.insert("font");
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
        qDebug() << "TextComponentInstance::connectToVariant - no component for instance" << getId();
        return;
    }
    
    qDebug() << "TextComponentInstance::connectToVariant - connecting instance" << getId();
    qDebug() << "  Component ID:" << m_component->getId();
    
    // For now, just use the first variant
    QList<Element*> variants = m_component->variants();
    qDebug() << "  Component has" << variants.size() << "variants";
    
    if (!variants.isEmpty()) {
        m_sourceVariant = variants.first();
        qDebug() << "  Using variant" << m_sourceVariant->getId() << "type:" << m_sourceVariant->getTypeName();
        
        // Clear modified properties when connecting to a new variant
        m_modifiedProperties.clear();
        
        // Sync initial properties
        syncPropertiesFromVariant();
        
        // Connect to property changes
        if (m_sourceVariant) {
            // For TextVariant, connect directly to contentChanged signal
            if (TextVariant* textVariant = qobject_cast<TextVariant*>(m_sourceVariant)) {
                qDebug() << "  Connecting to TextVariant contentChanged signal";
                // Note: Can't use Qt::UniqueConnection with lambdas, so we'll just use a regular connection
                QMetaObject::Connection conn = connect(textVariant, &TextVariant::contentChanged,
                        this, [this, textVariant]() {
                            qDebug() << "TextComponentInstance - received contentChanged signal from variant" << textVariant->getId();
                            qDebug() << "  New content:" << textVariant->content();
                            qDebug() << "  Current instance content:" << this->content();
                            setContent(textVariant->content());
                            qDebug() << "  After update, instance content:" << this->content();
                        });
                
                if (conn) {
                    m_variantConnections.add(conn);
                    qDebug() << "  Successfully connected contentChanged signal";
                } else {
                    qDebug() << "  FAILED to connect contentChanged signal";
                }
                
                // Also connect other properties
                m_variantConnections.add(
                    connect(textVariant, &TextVariant::fontChanged,
                            this, [this, textVariant]() {
                                if (!m_modifiedProperties.contains("font")) {
                                    setFont(textVariant->font());
                                }
                            }, Qt::UniqueConnection)
                );
                
                m_variantConnections.add(
                    connect(textVariant, &TextVariant::colorChanged,
                            this, [this, textVariant]() {
                                if (!m_modifiedProperties.contains("color")) {
                                    setColor(textVariant->color());
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
        qDebug() << "TextComponentInstance::syncPropertiesFromVariant - syncing from variant" << textVariant->getId();
        qDebug() << "  Variant content:" << textVariant->content();
        
        // Sync Text-specific properties
        // Always sync content from variant
        setContent(textVariant->content());
        qDebug() << "  Set content to:" << content();
        if (!m_modifiedProperties.contains("font")) {
            setFont(textVariant->font());
            m_modifiedProperties.remove("font");
        }
        if (!m_modifiedProperties.contains("color")) {
            setColor(textVariant->color());
            m_modifiedProperties.remove("color");
        }
        if (!m_modifiedProperties.contains("position")) {
            setPosition(textVariant->position());
            m_modifiedProperties.remove("position");
        }
        
        // Sync geometry
        setRect(QRectF(textVariant->x(), textVariant->y(), 
                      textVariant->width(), textVariant->height()));
    }
}