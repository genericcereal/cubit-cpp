#ifndef BOXSHADOW_H
#define BOXSHADOW_H

#include <QObject>
#include <QColor>
#include <QMetaType>

class BoxShadow {
    Q_GADGET
    Q_PROPERTY(qreal offsetX MEMBER offsetX)
    Q_PROPERTY(qreal offsetY MEMBER offsetY)
    Q_PROPERTY(qreal blurRadius MEMBER blurRadius)
    Q_PROPERTY(qreal spreadRadius MEMBER spreadRadius)
    Q_PROPERTY(QColor color MEMBER color)
    Q_PROPERTY(bool enabled MEMBER enabled)
    
public:
    qreal offsetX = 0;
    qreal offsetY = 0;
    qreal blurRadius = 0;
    qreal spreadRadius = 0;
    QColor color = QColor(0, 0, 0, 77); // 30% black
    bool enabled = false;
    
    bool operator==(const BoxShadow& other) const {
        return offsetX == other.offsetX &&
               offsetY == other.offsetY &&
               blurRadius == other.blurRadius &&
               spreadRadius == other.spreadRadius &&
               color == other.color &&
               enabled == other.enabled;
    }
    
    bool operator!=(const BoxShadow& other) const {
        return !(*this == other);
    }
};

Q_DECLARE_METATYPE(BoxShadow)

#endif // BOXSHADOW_H