#pragma once
#include <QObject>
#include <QString>

class Element : public QObject {
    Q_OBJECT
    Q_PROPERTY(QString elementId READ getId CONSTANT)
    Q_PROPERTY(QString elementType READ getTypeName CONSTANT)
    Q_PROPERTY(QString name READ getName WRITE setName NOTIFY nameChanged)
    Q_PROPERTY(QString parentId READ getParentElementId WRITE setParentElementId NOTIFY parentIdChanged)
    Q_PROPERTY(bool selected READ isSelected WRITE setSelected NOTIFY selectedChanged)
    Q_PROPERTY(bool isVisual READ isVisual CONSTANT)
    
public:
    enum ElementType {
        FrameType,
        TextType,
        VariableType,
        HtmlType,
        NodeType,
        EdgeType
    };
    Q_ENUM(ElementType)

    explicit Element(ElementType type, const QString &id, QObject *parent = nullptr);
    virtual ~Element() = default;
    
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
    
signals:
    void nameChanged();
    void parentIdChanged();
    void selectedChanged();
    void elementChanged();
    
protected:
    ElementType elementType;
    QString elementId;
    QString name;
    QString parentElementId;
    bool selected;
};