#include "SelectionBox.h"
#include <QPainter>
#include <QStyleOptionGraphicsItem>
#include <QBrush>
#include <QGraphicsScene>

SelectionBox::SelectionBox()
    : active(false)
{
    setZValue(2000);  // High z-value to ensure it's on top
    
    selectionPen.setStyle(Qt::DashLine);
    selectionPen.setWidth(1);
    selectionPen.setColor(QColor(0, 120, 215));
    selectionPen.setDashPattern(QVector<qreal>() << 4 << 4);
}

void SelectionBox::startSelection(const QPointF &pos)
{
    startPos = pos;
    currentPos = pos;
    active = true;
    setPos(0, 0);  // Position at scene origin
    prepareGeometryChange();
    setVisible(true);
    update();
}

void SelectionBox::updateSelection(const QPointF &pos)
{
    if (!active) return;
    
    currentPos = pos;
    prepareGeometryChange();  // Notify scene of bounds change
    update();
}

void SelectionBox::endSelection()
{
    // Get the current bounds before deactivating
    QRectF bounds = boundingRect();
    
    active = false;
    startPos = QPointF();
    currentPos = QPointF();
    prepareGeometryChange();  // Notify scene that our bounds changed
    setVisible(false);
    
    // Force the scene to update the area where the selection box was
    if (scene()) {
        scene()->update(bounds);  // Update in scene coordinates since we're at origin
    }
}

QRectF SelectionBox::getSelectionRect() const
{
    return QRectF(qMin(startPos.x(), currentPos.x()),
                  qMin(startPos.y(), currentPos.y()),
                  qAbs(currentPos.x() - startPos.x()),
                  qAbs(currentPos.y() - startPos.y()));
}

QRectF SelectionBox::boundingRect() const
{
    if (!active) return QRectF();
    // Since we're positioned at origin, return the full selection rect
    QRectF rect = getSelectionRect();
    // Expand by 2 pixels to ensure the pen is fully included
    return rect.adjusted(-2, -2, 2, 2);
}

void SelectionBox::paint(QPainter *painter, const QStyleOptionGraphicsItem *, QWidget *)
{
    if (!active) return;
    
    painter->setRenderHint(QPainter::Antialiasing);
    
    painter->setPen(selectionPen);
    painter->setBrush(QBrush(QColor(0, 120, 215, 30)));
    
    // Draw the selection rectangle
    painter->drawRect(getSelectionRect());
}