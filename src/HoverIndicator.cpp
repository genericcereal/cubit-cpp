#include "HoverIndicator.h"
#include <QPainter>
#include <QPen>

HoverIndicator::HoverIndicator(QWidget *parent) : QWidget(parent) {
    // Make the widget transparent and ensure it doesn't capture mouse events
    setAttribute(Qt::WA_TransparentForMouseEvents);
    setAttribute(Qt::WA_TranslucentBackground);
    setWindowFlags(Qt::FramelessWindowHint);
}

void HoverIndicator::setGeometry(const QRect &rect) {
    QWidget::setGeometry(rect);
}

void HoverIndicator::paintEvent(QPaintEvent* /*event*/)
{
    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing, true);

    QPen pen(Qt::blue);
    pen.setWidth(1);
    pen.setCosmetic(true);            // ensure it’s always exactly 1 device‐pixel wide
    p.setPen(pen);
    p.setBrush(Qt::NoBrush);

    // Draw on half‐pixel coordinates:
    //
    //   • The widget’s logical rect() is [0,0 .. width()-1,height()-1].
    //   • If we draw a QRectF(0.5,0.5, width()-1, height()-1), 
    //     all four edges lie exactly on pixel‐centers.
    //
    QRectF r(0.5, 0.5,
             qreal(width())  - 1.0,
             qreal(height()) - 1.0);
    p.drawRect(r);
}