#pragma once
#include "DesignElement.h"
#include <QColor>

class WebTextInput : public DesignElement {
    Q_OBJECT
    Q_PROPERTY(QString placeholder READ placeholder WRITE setPlaceholder NOTIFY placeholderChanged)
    Q_PROPERTY(QString value READ value WRITE setValue NOTIFY valueChanged)
    Q_PROPERTY(QColor borderColor READ borderColor WRITE setBorderColor NOTIFY borderColorChanged)
    Q_PROPERTY(qreal borderWidth READ borderWidth WRITE setBorderWidth NOTIFY borderWidthChanged)
    Q_PROPERTY(qreal borderRadius READ borderRadius WRITE setBorderRadius NOTIFY borderRadiusChanged)
    Q_PROPERTY(bool isEditing READ isEditing WRITE setIsEditing NOTIFY isEditingChanged)
    Q_PROPERTY(PositionType position READ position WRITE setPosition NOTIFY positionChanged)
    
public:
    enum PositionType {
        Relative,
        Absolute,
        Fixed
    };
    Q_ENUM(PositionType)
    explicit WebTextInput(const QString &id, QObject *parent = nullptr);
    
    QString placeholder() const { return m_placeholder; }
    QString value() const { return m_value; }
    QColor borderColor() const { return m_borderColor; }
    qreal borderWidth() const { return m_borderWidth; }
    qreal borderRadius() const { return m_borderRadius; }
    bool isEditing() const { return m_isEditing; }
    PositionType position() const { return m_position; }
    
    void setPlaceholder(const QString &placeholder);
    void setValue(const QString &value);
    void setBorderColor(const QColor &color);
    void setBorderWidth(qreal width);
    void setBorderRadius(qreal radius);
    void setIsEditing(bool editing);
    void setPosition(PositionType position);
    
    // Override geometry setters to trigger parent layout
    void setWidth(qreal w) override;
    void setHeight(qreal h) override;
    void setRect(const QRectF &rect) override;
    
    // Override to pass current value to script events
    Q_INVOKABLE void executeScriptEvent(const QString& eventName, const QVariantMap& eventData = QVariantMap()) override;
    
signals:
    void placeholderChanged();
    void valueChanged();
    void borderColorChanged();
    void borderWidthChanged();
    void borderRadiusChanged();
    void isEditingChanged();
    void positionChanged();
    
private:
    QString m_placeholder = "Enter text...";
    QString m_value;
    QColor m_borderColor{170, 170, 170};  // #aaaaaa
    qreal m_borderWidth = 1.0;
    qreal m_borderRadius = 4.0;
    bool m_isEditing = false;
    PositionType m_position = Absolute;
};