#pragma once
#include <QObject>
#include <QString>
#include <QVariant>
#include <QStringList>
#include <memory>

class PropertyRegistry;
class Frame;
class ElementModel;

class Element : public QObject {
    Q_OBJECT
    Q_PROPERTY(QString elementId READ getId CONSTANT)
    Q_PROPERTY(QString elementType READ getTypeName CONSTANT)
    Q_PROPERTY(QString name READ getName WRITE setName NOTIFY nameChanged)
    Q_PROPERTY(QString parentId READ getParentElementId WRITE setParentElementId NOTIFY parentIdChanged)
    Q_PROPERTY(bool selected READ isSelected WRITE setSelected NOTIFY selectedChanged)
    Q_PROPERTY(bool isVisual READ isVisual CONSTANT)
    Q_PROPERTY(bool showInElementList READ showInElementList CONSTANT)
    
public:
    enum ElementType {
        FrameType,
        TextType,
        VariableType,
        NodeType,
        EdgeType,
        ComponentType,
        FrameComponentInstanceType,
        FrameComponentVariantType,
        TextVariantType,
        WebTextInputType,
        WebTextInputComponentInstanceType,
        WebTextInputComponentVariantType,
        ShapeType,
        ShapeComponentInstanceType,
        ShapeComponentVariantType
    };
    Q_ENUM(ElementType)

    explicit Element(ElementType type, const QString &id, QObject *parent = nullptr);
    virtual ~Element();
    
    // Property getters
    QString getId() const { return elementId; }
    ElementType getType() const { return elementType; }
    QString getTypeName() const;
    QString getName() const { return name; }
    
    // Property setters
    void setName(const QString &newName);
    
    // Parent management
    virtual void setParentElementId(const QString &parentId);
    QString getParentElementId() const { return parentElementId; }
    bool hasParent() const { return !parentElementId.isEmpty(); }
    
    // Selection state
    bool isSelected() const { return selected; }
    void setSelected(bool sel);
    
    // Check if this is a visual element that appears on canvas
    virtual bool isVisual() const { return false; }
    
    // Check if this element should be shown in the element list
    virtual bool showInElementList() const { return m_showInElementList; }
    void setShowInElementList(bool show) { m_showInElementList = show; }
    
    // Check if this is a global instance
    bool isGlobalInstance() const { return m_isGlobalInstance; }
    void setIsGlobalInstance(bool isInstance) { m_isGlobalInstance = isInstance; }
    
    // Generic property access using PropertyRegistry
    Q_INVOKABLE virtual QVariant getProperty(const QString& name) const;
    Q_INVOKABLE virtual void setProperty(const QString& name, const QVariant& value);
    Q_INVOKABLE virtual bool hasProperty(const QString& name) const;
    Q_INVOKABLE virtual QStringList propertyNames() const;
    Q_INVOKABLE virtual QVariantList getPropertyMetadata() const;
    
    // Register element properties - to be overridden by subclasses
    virtual void registerProperties() {}
    
signals:
    void nameChanged();
    void parentIdChanged();
    void selectedChanged();
    void elementChanged();
    void propertyChanged(const QString& name, const QVariant& value);
    
protected:
    ElementType elementType;
    QString elementId;
    QString name;
    QString parentElementId;
    bool selected;
    bool m_showInElementList = true;
    
    // Property registry for dynamic properties
    std::unique_ptr<PropertyRegistry> m_properties;
    
    // Helper to trigger layout updates if needed (override in subclasses)
    virtual void triggerLayoutIfNeeded(const QString& /*propertyName*/) {}
    
    // Track if this is a global instance (not the original)
    bool m_isGlobalInstance = false;
    
private:
    // Handle global frame parenting
    void handleGlobalFrameParenting(const QString& oldParentId, const QString& newParentId);
    bool isGlobalFrame(Element* frame) const;
    void createInstancesInAllFrames();
    void removeInstancesFromAllFrames();
    void createInstanceInFrame(Frame* targetFrame, ElementModel* targetModel);
};