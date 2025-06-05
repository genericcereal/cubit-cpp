#include "PanelLayer.h"
#include "ActionsPanel.h"
#include "Canvas.h"
#include <QMouseEvent>
#include <QApplication>

PanelLayer::PanelLayer(QWidget *parent) : QWidget(parent), canvasRef(nullptr) {
    // PanelLayer should receive mouse events
    setAttribute(Qt::WA_TransparentForMouseEvents, false);
    
    actionsPanel = new ActionsPanel(this);
    // Ensure ActionsPanel can receive events
    actionsPanel->raise();  
}

void PanelLayer::setCanvas(Canvas *canvas) {
    canvasRef = canvas;
    actionsPanel->setCanvas(canvas);
}

void PanelLayer::resizeEvent(QResizeEvent *event) {
    QWidget::resizeEvent(event);

    // Center horizontally, 10px from bottom
    int panelWidth = actionsPanel->width();
    int panelHeight = actionsPanel->height();

    int x = (width() - panelWidth) / 2;
    int y = height() - panelHeight - 10;

    actionsPanel->move(x, y);
}

void PanelLayer::mousePressEvent(QMouseEvent *event) {
    // Check if the click is within the ActionsPanel bounds
    QRect panelRect = actionsPanel->geometry();
    if (!panelRect.contains(event->pos())) {
        // Ignore this event and let it propagate to lower layers
        event->ignore();
        return;
    }
    // Handle events on the ActionsPanel
    QWidget::mousePressEvent(event);
}

void PanelLayer::mouseMoveEvent(QMouseEvent *event) {
    // Always ignore move events to let them propagate to lower layers
    event->ignore();
}

void PanelLayer::mouseReleaseEvent(QMouseEvent *event) {
    // Check if the release is within the ActionsPanel bounds
    QRect panelRect = actionsPanel->geometry();
    if (!panelRect.contains(event->pos())) {
        // Ignore this event and let it propagate to lower layers
        event->ignore();
        return;
    }
    // Handle events on the ActionsPanel
    QWidget::mouseReleaseEvent(event);
}
