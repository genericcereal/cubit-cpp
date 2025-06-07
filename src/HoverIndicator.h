#pragma once
#include <QWidget>
#include <QPainter>
#include <QRectF>

class HoverIndicator : public QWidget {
    Q_OBJECT
public:
    explicit HoverIndicator(QWidget *parent = nullptr);
    
    void setGeometry(const QRectF &rect);
    
protected:
    void paintEvent(QPaintEvent *event) override;
};