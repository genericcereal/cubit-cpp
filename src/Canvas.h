#pragma once

#include "CanvasBase.h"
#include <QPaintEvent>

class Canvas : public CanvasBase {
    Q_OBJECT
public:
    explicit Canvas(QWidget *parent = nullptr);
    
    // Override render from CanvasBase
    void render() override;
    
    // Get rendering type
    QString getRenderingType() const { return "CPU"; }

protected:
    void paintEvent(QPaintEvent *event) override;

private:
    // Helper methods for CPU rendering
    void renderFrame(QPainter &painter, const QRect &rect, const QColor &color);
    void renderText(QPainter &painter, const QString &text, const QPoint &pos, const QFont &font);
};