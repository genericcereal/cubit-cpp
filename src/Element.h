#pragma once
#include <QObject>
#include <QString>
#include <QPointF>
#include <QSizeF>
#include <QRectF>

class Element : public QObject {
    Q_OBJECT
    Q_PROPERTY(QString elementId READ getId CONSTANT)
    Q_PROPERTY(QString elementType READ getTypeName CONSTANT)
    Q_PROPERTY(QString name READ getName WRITE setName NOTIFY nameChanged)
    Q_PROPERTY(qreal x READ x WRITE setX NOTIFY xChanged)
    Q_PROPERTY(qreal y READ y WRITE setY NOTIFY yChanged)
    Q_PROPERTY(qreal width READ width WRITE setWidth NOTIFY widthChanged)
    Q_PROPERTY(qreal height READ height WRITE setHeight NOTIFY heightChanged)
    Q_PROPERTY(QString parentId READ getParentElementId WRITE setParentElementId NOTIFY parentIdChanged)
    Q_PROPERTY(bool selected READ isSelected WRITE setSelected NOTIFY selectedChanged)
    
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
    virtual QString getName() const { return name; }
    
    qreal x() const { return canvasPosition.x(); }
    qreal y() const { return canvasPosition.y(); }
    qreal width() const { return canvasSize.width(); }
    qreal height() const { return canvasSize.height(); }
    
    QRectF rect() const { return QRectF(canvasPosition, canvasSize); }
    
    // Hit testing - can be overridden for custom shapes
    virtual bool containsPoint(const QPointF &point) const;
    
    // Property setters
    virtual void setName(const QString &newName);
    void setX(qreal x);
    void setY(qreal y);
    void setWidth(qreal w);
    void setHeight(qreal h);
    void setRect(const QRectF &rect);
    
    // Parent management
    void setParentElementId(const QString &parentId);
    QString getParentElementId() const { return parentElementId; }
    bool hasParent() const { return !parentElementId.isEmpty(); }
    
    // Selection state
    bool isSelected() const { return selected; }
    void setSelected(bool sel);
    
signals:
    void nameChanged();
    void xChanged();
    void yChanged();
    void widthChanged();
    void heightChanged();
    void parentIdChanged();
    void selectedChanged();
    void elementChanged();
    void geometryChanged();  // Emitted when x, y, width, or height change
    
protected:
    ElementType elementType;
    QString elementId;
    QString parentElementId;
    QString name;
    QPointF canvasPosition;
    QSizeF canvasSize;
    bool selected;
};