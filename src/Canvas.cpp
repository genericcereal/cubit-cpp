#include "Canvas.h"
#include "Frame.h"
#include "Text.h"
#include "Element.h"
#include <QPainter>
#include <QDebug>

Canvas::Canvas(QWidget *parent) : CanvasBase(parent) {
    // Base class handles initialization
}

void Canvas::paintEvent(QPaintEvent *) {
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);
    
    // Clear background
    painter.fillRect(rect(), QColor(242, 242, 242));
    
    // Render all elements
    for (Element *element : elements) {
        if (!element->isVisible()) continue;
        
        if (element->getType() == Element::FrameType) {
            Frame *frame = qobject_cast<Frame*>(element);
            if (frame) {
                renderFrame(painter, frame->geometry(), frame->getColor());
            }
        } else if (element->getType() == Element::TextType) {
            Text *text = qobject_cast<Text*>(element);
            if (text) {
                renderText(painter, text->getText(), text->pos(), text->font());
            }
        }
    }
}

void Canvas::render() {
    update();  // Triggers paintEvent
}

void Canvas::renderFrame(QPainter &painter, const QRect &rect, const QColor &color) {
    painter.fillRect(rect, color);
    
    // Draw border
    painter.setPen(QPen(color.darker(120), 1));
    painter.drawRect(rect.adjusted(0, 0, -1, -1));
}

void Canvas::renderText(QPainter &painter, const QString &text, const QPoint &pos, const QFont &font) {
    painter.setFont(font);
    painter.setPen(Qt::black);
    painter.drawText(pos, text);
}