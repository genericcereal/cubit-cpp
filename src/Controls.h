#pragma once
#include <QWidget>
#include <QRect>

class QFrame;

class Controls : public QWidget {
    Q_OBJECT
public:
    explicit Controls(QWidget *parent = nullptr);
    
    // Update the position and size of the controls
    void updateGeometry(const QRect &targetRect);
    
    // Get the current control rectangle
    QRect getControlRect() const { return currentRect; }
    
signals:
    // Emitted when the control rectangle changes due to dragging
    void rectChanged(const QRect &newRect);
    
protected:
    // Mouse event handlers
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;
    
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
    
    // Helper to position the bars and joints
    void positionControls(const QRect &rect);
    
    // Current rectangle being controlled
    QRect currentRect;
    
    // Dragging state
    enum DragMode {
        None,
        LeftBar,
        RightBar,
        TopBar,
        BottomBar
    };
    DragMode dragMode;
    QPoint dragStartPos;
    QRect dragStartRect;
    
    // Helper to determine which bar is at a point
    DragMode getBarAt(const QPoint &pos) const;
};