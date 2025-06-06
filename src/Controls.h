#pragma once
#include <QWidget>
#include <QRect>
#include <QTimer>

class QFrame;

class Controls : public QWidget {
    Q_OBJECT
public:
    // Dragging state
    enum DragMode {
        None,
        LeftBar,
        RightBar,
        TopBar,
        BottomBar,
        TopLeftJoint,
        TopRightJoint,
        BottomLeftJoint,
        BottomRightJoint,
        InnerRect
    };
    
    explicit Controls(QWidget *parent = nullptr);
    
    // Update the position and size of the controls
    void updateGeometry(const QRect &targetRect);
    
    // Set the canvas pan offset and zoom scale
    void setPanOffset(const QPoint &offset) { panOffset = offset; }
    void setZoomScale(qreal scale) { zoomScale = scale; }
    
    // Get the current control rectangle
    QRect getControlRect() const { return currentRect; }
    
    // Start dragging programmatically
    void startDragMode(DragMode mode, const QPoint &globalStartPos);
    
    // Update drag position programmatically
    void updateDragPosition(const QPoint &globalPos);
    
    // End drag programmatically
    void endDrag();
    
signals:
    // Emitted when the control rectangle changes due to dragging
    void rectChanged(const QRect &newRect);
    
    // Emitted when the inner rectangle is clicked (not dragged)
    void innerRectClicked(const QPoint &globalPos);
    
    // Emitted when the inner rectangle is double-clicked
    void innerRectDoubleClicked(const QPoint &globalPos);
    
protected:
    // Mouse event handlers
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;
    void mouseDoubleClickEvent(QMouseEvent *event) override;
    
private:
    // The four control bars (edges)
    QFrame *leftBar;
    QFrame *rightBar;
    QFrame *topBar;
    QFrame *bottomBar;
    
    // The four corner joints
    QFrame *topLeftJoint;
    QFrame *topRightJoint;
    QFrame *bottomLeftJoint;
    QFrame *bottomRightJoint;
    
    // Inner rectangle (yellow with transparency)
    QFrame *innerRect;
    
    // Helper to position the bars and joints
    void positionControls(const QRect &rect);
    
    // Current rectangle being controlled
    QRect currentRect;
    
    // Dragging state
    DragMode dragMode;
    QPoint dragStartPos;
    QRect dragStartRect;
    bool hasDragged;  // Track if mouse has moved during drag
    
    // Helper to determine which bar is at a point
    DragMode getBarAt(const QPoint &pos) const;
    
    // Canvas pan offset and zoom scale
    QPoint panOffset;
    qreal zoomScale = 1.0;
    
    // Double-click detection
    QTimer *singleClickTimer;
    QPoint pendingClickPos;
};