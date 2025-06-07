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

void HoverIndicator::paintEvent(QPaintEvent*) {
  QPainter p(this);
  p.setRenderHint(QPainter::Antialiasing, true);

  // width=0 ⇒ cosmetic ⇒ exactly 1 device-pixel wide
  QPen pen(Qt::blue, 0);
  pen.setCosmetic(true);
  p.setPen(pen);
  p.setBrush(Qt::NoBrush);

  // Grab the full painter transform (including the view’s zoom)
  QTransform t = p.transform();
  qreal sx = t.m11();
  qreal sy = t.m22();
  qreal dx = 0.5 / sx;
  qreal dy = 0.5 / sy;

  // Inset by (dx,dy) ⇒ both edges land on pixel centers
  QRectF r(dx, dy,
           width()  - 2*dx,
           height() - 2*dy);
  p.drawRect(r);
}