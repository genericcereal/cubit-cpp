#pragma once
#include <QWidget>
#include <QPainter>

class HoverIndicator : public QWidget {
    Q_OBJECT
public:
    explicit HoverIndicator(QWidget *parent = nullptr);
    
    void setGeometry(const QRect &rect);
    
protected:
    void paintEvent(QPaintEvent *event) override;
};