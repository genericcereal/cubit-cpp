#include "HoverIndicator.h"
#include <QPainter>
#include <QPen>

HoverIndicator::HoverIndicator(QWidget *parent) : QWidget(parent) {
    // Make the widget transparent and ensure it doesn't capture mouse events
    setAttribute(Qt::WA_TransparentForMouseEvents);
    setAttribute(Qt::WA_TranslucentBackground);
    setWindowFlags(Qt::FramelessWindowHint);
}

void HoverIndicator::setGeometry(const QRectF &rect) {
    // Convert QRectF to QRect using proper rounding
    // This ensures subpixel precision is maintained as much as possible
    QWidget::setGeometry(rect.toRect());
}

void HoverIndicator::paintEvent(QPaintEvent*) {
  QPainter p(this);
  p.setRenderHint(QPainter::Antialiasing, false);  // Disable antialiasing for crisp lines

  // Use a cosmetic pen that's exactly 1 device pixel wide
  QPen pen(Qt::blue, 0);
  pen.setCosmetic(true);
  p.setPen(pen);
  p.setBrush(Qt::NoBrush);

  // Draw the border to align exactly with the widget edges
  // Using QRectF for more precise positioning
  // The 0.5 offset ensures the stroke is centered on the pixel boundary
  QRectF r(0.5, 0.5, width() - 1.0, height() - 1.0);
  p.drawRect(r);
}