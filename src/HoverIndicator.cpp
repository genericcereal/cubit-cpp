#include "HoverIndicator.h"
#include <QPainter>
#include <QPen>
#include <QDebug>

HoverIndicator::HoverIndicator(QWidget *parent) : QWidget(parent) {
    // Make the widget transparent and ensure it doesn't capture mouse events
    setAttribute(Qt::WA_TransparentForMouseEvents);
    setAttribute(Qt::WA_TranslucentBackground);
    setWindowFlags(Qt::FramelessWindowHint);
    
    // Ensure no margins or padding
    setContentsMargins(0, 0, 0, 0);
    
    // Set size policy to match frame behavior
    setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
}

void HoverIndicator::setGeometry(const QRectF &rect) {
    // Convert QRectF to QRect using proper rounding
    // This ensures subpixel precision is maintained as much as possible
    qDebug() << "HoverIndicator::setGeometry - input rect:" << rect 
             << "toRect():" << rect.toRect()
             << "current geometry:" << geometry();
    QWidget::setGeometry(rect.toRect());
    qDebug() << "HoverIndicator::setGeometry - after setGeometry, geometry:" << geometry();
}

void HoverIndicator::paintEvent(QPaintEvent*) {
    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing, false);

    // 1) Figure out the current view-scale from the painter's transform:
    const qreal scaleX = p.transform().m11();   // assume uniform X/Y
    const qreal penWidth = 1.0 / scaleX;        // in *widget* units, so scaled → 1px on screen

    // 2) Build a *non-cosmetic* pen at that width:
    QPen pen(Qt::blue, penWidth, Qt::SolidLine, Qt::SquareCap, Qt::MiterJoin);
    pen.setCosmetic(false);
    p.setPen(pen);
    p.setBrush(Qt::NoBrush);

    // 3) Offset by half the pen width so the stroke lies entirely inside the widget:
    const qreal half = penWidth * 0.5;
    QRectF r(half, half,
             width()  - penWidth,
             height() - penWidth);

    // 4) (Optional) fill a translucent background
    QColor bg(255,255,255,100);
    p.fillRect(rect(), bg);

    // 5) Draw your perfectly-aligned border
    p.drawRect(r);
}