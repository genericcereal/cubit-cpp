#pragma once
#include <QWidget>

class Controls;

class ControlLayer : public QWidget {
    Q_OBJECT
public:
    explicit ControlLayer(QWidget *parent = nullptr);
    
    // Show controls around a specific rectangle
    void showControls(const QRect &rect);
    
    // Hide the controls
    void hideControls();
    
protected:
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;
    
private:
    Controls *controls;
};