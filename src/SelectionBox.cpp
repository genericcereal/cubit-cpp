#include "SelectionBox.h"
#include <QPainter>
#include <QPaintEvent>
#include <QBrush>

SelectionBox::SelectionBox(QWidget *parent)
    : QWidget(parent)
    , active(false)
{
    setAttribute(Qt::WA_TransparentForMouseEvents);
    setAttribute(Qt::WA_TranslucentBackground);
    setWindowFlags(Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint);
    
    selectionPen.setStyle(Qt::DashLine);
    selectionPen.setWidth(1);
    selectionPen.setColor(QColor(0, 120, 215));
    selectionPen.setDashPattern(QVector<qreal>() << 4 << 4);
}

void SelectionBox::startSelection(const QPoint &pos)
{
    startPos = pos;
    currentPos = pos;
    active = true;
    show();
    update();
}

void SelectionBox::updateSelection(const QPoint &pos)
{
    if (!active) return;
    
    currentPos = pos;
    
    QRect rect = getSelectionRect();
    setGeometry(rect);
    update();
}

void SelectionBox::endSelection()
{
    active = false;
    hide();
}

QRect SelectionBox::getSelectionRect() const
{
    return QRect(qMin(startPos.x(), currentPos.x()),
                 qMin(startPos.y(), currentPos.y()),
                 qAbs(currentPos.x() - startPos.x()),
                 qAbs(currentPos.y() - startPos.y()));
}

void SelectionBox::paintEvent(QPaintEvent *)
{
    if (!active) return;
    
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);
    
    painter.setPen(selectionPen);
    painter.setBrush(QBrush(QColor(0, 120, 215, 30)));
    
    painter.drawRect(rect().adjusted(0, 0, -1, -1));
}