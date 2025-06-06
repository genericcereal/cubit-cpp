#pragma once
#include <QGraphicsItem>
#include <QPointF>
#include <QPen>

class SelectionBox : public QGraphicsItem {
public:
    explicit SelectionBox();
    
    void startSelection(const QPointF &pos);
    void updateSelection(const QPointF &pos);
    void endSelection();
    bool isActive() const { return active; }
    QRectF getSelectionRect() const;
    
    // QGraphicsItem interface
    QRectF boundingRect() const override;
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget) override;
    
private:
    QPointF startPos;
    QPointF currentPos;
    bool active;
    QPen selectionPen;
};