#include "Canvas.h"
#include "PanelLayer.h"
#include "ControlLayer.h"
#include "ElementLayer.h"
#include "ClientRectLayer.h"
#include "ActionsPanel.h"
#include "Element.h"
#include "Frame.h"
#include "Text.h"
#include "Variable.h"
#include "ClientRect.h"
#include <QMouseEvent>

Canvas::Canvas(QWidget *parent) : QWidget(parent), mode("Select"), 
    isCreatingFrame(false), tempFrame(nullptr), tempClientRect(nullptr) {

    setContentsMargins(0, 0, 0, 0);
    setMouseTracking(true);  // Enable mouse tracking for cursor changes

    elementLayer = new ElementLayer(this);
    clientRectLayer = new ClientRectLayer(this);
    controlLayer = new ControlLayer(this);
    panelLayer   = new PanelLayer(this);

    // Set up z-order (bottom to top)
    elementLayer->lower();                        // zIndex 1
    clientRectLayer->stackUnder(controlLayer);    // zIndex 2
    controlLayer->stackUnder(panelLayer);         // zIndex 3
    panelLayer->raise();                          // zIndex 4
    
    // Connect ActionsPanel to Canvas
    panelLayer->setCanvas(this);
    
}

void Canvas::resizeEvent(QResizeEvent *event) {
    QWidget::resizeEvent(event);
    const QSize size = this->size();
    elementLayer->resize(size);
    clientRectLayer->resize(size);
    controlLayer->resize(size);
    panelLayer->resize(size);
}

void Canvas::setMode(const QString &newMode) {
    if (mode != newMode) {
        mode = newMode;
        emit modeChanged(mode);
        
        // Update cursor based on mode
        if (newMode == "Frame") {
            setCursor(Qt::CrossCursor);
        } else {
            setCursor(Qt::ArrowCursor);
        }
        
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
    
    // Create a new text element in the element layer
    Text *text = new Text(textId, elementLayer);
    
    // Position it at a default location or center it
    int textX = (elementLayer->width() - text->width()) / 2;
    int textY = (elementLayer->height() - text->height()) / 2;
    text->move(textX, textY);
    text->show();
    
    // Add to elements list
    elements.append(text);
    
    // Emit signal that a text element was created
    emit elementCreated("Text", text->getName());
    
    // Switch back to Select mode
    setMode("Select");
    
    // Update the ActionsPanel to reflect the mode change
    ActionsPanel *actionsPanel = panelLayer->getActionsPanel();
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
    ActionsPanel *actionsPanel = panelLayer->getActionsPanel();
    if (actionsPanel) {
        actionsPanel->setModeSelection("Select");
    }
}

/*
void Canvas::mousePressEvent(QMouseEvent *event) {
    if (mode == "Frame" && event->button() == Qt::LeftButton) {
        // Check if this event is coming from the ActionsPanel area
        // The ActionsPanel is centered horizontally near the bottom
        ActionsPanel *actionsPanel = panelLayer->getActionsPanel();
        if (actionsPanel) {
            QPoint globalPos = event->globalPos();
            QPoint panelGlobalPos = actionsPanel->mapToGlobal(QPoint(0, 0));
            QRect panelGlobalRect(panelGlobalPos, actionsPanel->size());
            
            // If the click originated from the ActionsPanel area, ignore it
            if (panelGlobalRect.contains(globalPos)) {
                QWidget::mousePressEvent(event);
                return;
            }
        }
        
        // Start frame creation
        isCreatingFrame = true;
        frameStartPos = event->pos();
        
        // Create temporary frame and client rect
        int frameId = elements.size() + 1;
        tempFrame = new Frame(frameId, elementLayer);
        tempFrame->move(frameStartPos);
        tempFrame->resize(0, 0);
        tempFrame->show();
        
        tempClientRect = new ClientRect(clientRectLayer);
        tempClientRect->move(frameStartPos);
        tempClientRect->resize(0, 0);
        tempClientRect->show();
    }
    QWidget::mousePressEvent(event);
}
*/

/*
void Canvas::mouseMoveEvent(QMouseEvent *event) {
    if (isCreatingFrame && tempFrame && tempClientRect) {
        // Calculate frame dimensions based on drag
        QPoint currentPos = event->pos();
        int x = qMin(frameStartPos.x(), currentPos.x());
        int y = qMin(frameStartPos.y(), currentPos.y());
        int width = qAbs(currentPos.x() - frameStartPos.x());
        int height = qAbs(currentPos.y() - frameStartPos.y());
        
        // Update temporary frame and client rect
        tempFrame->move(x, y);
        tempFrame->resize(width, height);
        
        tempClientRect->move(x, y);
        tempClientRect->resize(width, height);
    }
    QWidget::mouseMoveEvent(event);
}
*/

/*
void Canvas::mouseReleaseEvent(QMouseEvent *event) {
    if (isCreatingFrame && event->button() == Qt::LeftButton && tempFrame && tempClientRect) {
        // Finalize frame creation
        isCreatingFrame = false;
        
        // Only create the frame if it has a reasonable size
        if (tempFrame->width() > 10 && tempFrame->height() > 10) {
            // Add to elements list
            elements.append(tempFrame);
            
            // Emit signal that a frame was created
            emit elementCreated("Frame", tempFrame->getName());
            
            // Keep the frame and client rect
            tempFrame = nullptr;
            tempClientRect = nullptr;
        } else {
            // Remove small/invalid frames
            delete tempFrame;
            delete tempClientRect;
            tempFrame = nullptr;
            tempClientRect = nullptr;
        }
        
        // Switch back to Select mode
        setMode("Select");
        
        // Update the ActionsPanel to reflect the mode change
        ActionsPanel *actionsPanel = panelLayer->getActionsPanel();
        if (actionsPanel) {
            actionsPanel->setModeSelection("Select");
        }
    }
    QWidget::mouseReleaseEvent(event);
}
*/