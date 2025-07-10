#pragma once
#include "DesignElement.h"
#include <QFont>
#include <QColor>

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
    PositionType m_position = Relative;
};