#include "ControlLayer.h"
#include "Controls.h"
#include <QLabel>
#include <QMouseEvent>
#include <QShowEvent>
#include <QDebug>

ControlLayer::ControlLayer(QWidget *parent) : QWidget(parent) {
    // ControlLayer should receive mouse events
    setAttribute(Qt::WA_TransparentForMouseEvents, false);
    setMouseTracking(true);  // Enable mouse tracking for hover effects
    
    // Create the controls widget
    controls = new Controls(this);
    
    // For testing, show controls around a sample rectangle
    QRect testRect(300, 200, 200, 200);
    showControls(testRect);
}

void ControlLayer::showControls(const QRect &rect) {
    if (controls) {
        controls->updateGeometry(rect);
        controls->show();
    }
}

void ControlLayer::hideControls() {
    if (controls) {
        controls->hide();
    }
}

void ControlLayer::mousePressEvent(QMouseEvent *event) {
    qDebug() << "ControlLayer::mousePressEvent at" << event->pos();
    
    // Always pass the event to the base class to let child widgets handle it
    QWidget::mousePressEvent(event);
    
    // If the event wasn't accepted by a child widget, ignore it to let it propagate
    if (!event->isAccepted()) {
        event->ignore();
    }
}

void ControlLayer::mouseMoveEvent(QMouseEvent *event) {
    qDebug() << "ControlLayer::mouseMoveEvent at" << event->pos();
    
    // Always pass the event to the base class to let child widgets handle it
    QWidget::mouseMoveEvent(event);
    
    // If the event wasn't accepted by a child widget, ignore it to let it propagate
    if (!event->isAccepted()) {
        event->ignore();
    }
}

void ControlLayer::mouseReleaseEvent(QMouseEvent *event) {
    // Always pass the event to the base class to let child widgets handle it
    QWidget::mouseReleaseEvent(event);
    
    // If the event wasn't accepted by a child widget, ignore it to let it propagate
    if (!event->isAccepted()) {
        event->ignore();
    }
}