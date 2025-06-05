#include "Canvas.h"
#include "ActionsPanel.h"
#include "Controls.h"
#include "Element.h"
#include "Frame.h"
#include "Text.h"
#include "Variable.h"
#include "ClientRect.h"
#include <QMouseEvent>
#include <QApplication>

Canvas::Canvas(QWidget *parent) : QWidget(parent), mode("Select"), 
    isCreatingFrame(false), tempFrame(nullptr), tempClientRect(nullptr) {

    setContentsMargins(0, 0, 0, 0);
    setMouseTracking(true);  // Enable mouse tracking for cursor changes
    
    // Create UI controls directly on Canvas
    controls = new Controls(this);
    controls->hide();  // Initially hidden
    
    actionsPanel = new ActionsPanel(this);
    actionsPanel->setCanvas(this);

    // Ensure controls and actions panel are on top
    controls->raise();
    actionsPanel->raise();
    
    // For testing, show controls around a sample rectangle
    QRect testRect(300, 200, 200, 200);
    showControls(testRect);
}

void Canvas::resizeEvent(QResizeEvent *event) {
    QWidget::resizeEvent(event);
    
    // Position ActionsPanel at bottom center
    int panelWidth = actionsPanel->width();
    int panelHeight = actionsPanel->height();
    int x = (width() - panelWidth) / 2;
    int y = height() - panelHeight - 10;
    actionsPanel->move(x, y);
}

void Canvas::showControls(const QRect &rect) {
    if (controls) {
        controls->updateGeometry(rect);
        controls->show();
        controls->raise();
    }
}

void Canvas::hideControls() {
    if (controls) {
        controls->hide();
    }
}

void Canvas::setMode(const QString &newMode) {
    if (mode != newMode) {
        mode = newMode;
        emit modeChanged(mode);
        
        // Handle mode-specific actions
        if (newMode == "Text") {
            // When Text mode is selected, create a text element immediately
            createText();
        } else if (newMode == "Variable") {
            // When Variable mode is selected, create a variable immediately
            createVariable();
        }
    }
}

void Canvas::createFrame() {
    // Frame creation now happens via mouse drag events
    // This method is kept for API compatibility but does nothing
}

void Canvas::createText() {
    // Generate ID based on number of elements + 1
    int textId = elements.size() + 1;
    
    // Create a new text element with Canvas as parent
    Text *text = new Text(textId, this);
    
    // Position it at a default location or center it
    int textX = (width() - text->width()) / 2;
    int textY = (height() - text->height()) / 2;
    text->move(textX, textY);
    text->show();
    
    // Add to elements list
    elements.append(text);
    
    // Emit signal that a text element was created
    emit elementCreated("Text", text->getName());
    
    // Switch back to Select mode
    setMode("Select");
    
    // Update the ActionsPanel to reflect the mode change
    if (actionsPanel) {
        actionsPanel->setModeSelection("Select");
    }
}

void Canvas::createVariable() {
    // Generate ID based on number of elements + 1
    int variableId = elements.size() + 1;
    
    // Create a new variable - NOTE: Variables don't appear on canvas
    Variable *variable = new Variable(variableId, nullptr);
    
    // Add to elements list
    elements.append(variable);
    
    // Emit signal that a variable was created
    emit elementCreated("Variable", variable->getName());
    
    // Switch back to Select mode
    setMode("Select");
    
    // Update the ActionsPanel to reflect the mode change
    if (actionsPanel) {
        actionsPanel->setModeSelection("Select");
    }
}

void Canvas::mousePressEvent(QMouseEvent *event) {
    // Use normal event propagation
    QWidget::mousePressEvent(event);
}

void Canvas::mouseMoveEvent(QMouseEvent *event) {
    // Use normal event propagation
    QWidget::mouseMoveEvent(event);
}

void Canvas::mouseReleaseEvent(QMouseEvent *event) {
    // Use normal event propagation
    QWidget::mouseReleaseEvent(event);
}