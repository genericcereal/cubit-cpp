#pragma once
#include "Element.h"
#include <QFont>
#include <QColor>

class Text : public Element {
    Q_OBJECT
    Q_PROPERTY(QString text READ text WRITE setText NOTIFY textChanged)
    Q_PROPERTY(QFont font READ font WRITE setFont NOTIFY fontChanged)
    Q_PROPERTY(QColor color READ color WRITE setColor NOTIFY colorChanged)
    
public:
    explicit Text(int id, QObject *parent = nullptr);
    
    QString text() const { return m_text; }
    QFont font() const { return m_font; }
    QColor color() const { return m_color; }
    
    void setText(const QString &text);
    void setFont(const QFont &font);
    void setColor(const QColor &color);
    
signals:
    void textChanged();
    void fontChanged();
    void colorChanged();
    
private:
    QString m_text;
    QFont m_font;
    QColor m_color;
};