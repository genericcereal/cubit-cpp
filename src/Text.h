#pragma once
#include "DesignElement.h"
#include "PropertyDefinition.h"
#include <QFont>
#include <QColor>
#include <QList>

class Text : public DesignElement {
    Q_OBJECT
    Q_PROPERTY(QString content READ content WRITE setContent NOTIFY contentChanged)
    Q_PROPERTY(QFont font READ font WRITE setFont NOTIFY fontChanged)
    Q_PROPERTY(QColor color READ color WRITE setColor NOTIFY colorChanged)
    Q_PROPERTY(bool isEditing READ isEditing WRITE setIsEditing NOTIFY isEditingChanged)
    Q_PROPERTY(PositionType position READ position WRITE setPosition NOTIFY positionChanged)
    
public:
    enum PositionType {
        Relative,
        Absolute,
        Fixed
    };
    Q_ENUM(PositionType)
    explicit Text(const QString &id, QObject *parent = nullptr);
    
    // Static factory support methods
    static QString staticTypeName() { return "text"; }
    static QList<PropertyDefinition> staticPropertyDefinitions();
    
    QString content() const { return m_content; }
    QFont font() const { return m_font; }
    QColor color() const { return m_color; }
    bool isEditing() const { return m_isEditing; }
    PositionType position() const { return m_position; }
    
    void setContent(const QString &content);
    void setFont(const QFont &font);
    void setColor(const QColor &color);
    void setIsEditing(bool editing);
    void setPosition(PositionType position);
    
    // Override geometry setters to trigger parent layout
    void setWidth(qreal w) override;
    void setHeight(qreal h) override;
    void setRect(const QRectF &rect) override;
    
    // Override to provide Text-specific property definitions
    QList<PropertyDefinition> propertyDefinitions() const override;
    
    // Register Text properties with PropertyRegistry
    void registerProperties() override;
    
    // Override property access for custom handling
    QVariant getProperty(const QString& name) const override;
    void setProperty(const QString& name, const QVariant& value) override;
    
    Q_INVOKABLE void exitEditMode();
    
signals:
    void contentChanged();
    void fontChanged();
    void colorChanged();
    void isEditingChanged();
    void positionChanged();
    
private:
    QString m_content;
    QFont m_font;
    QColor m_color;
    bool m_isEditing = false;
    PositionType m_position = Absolute;
};