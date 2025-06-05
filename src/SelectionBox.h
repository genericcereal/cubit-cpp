#pragma once
#include <QWidget>
#include <QPoint>
#include <QPen>

class SelectionBox : public QWidget {
    Q_OBJECT
public:
    explicit SelectionBox(QWidget *parent = nullptr);
    
    void startSelection(const QPoint &pos);
    void updateSelection(const QPoint &pos);
    void endSelection();
    bool isActive() const { return active; }
    QRect getSelectionRect() const;
    
protected:
    void paintEvent(QPaintEvent *event) override;
    
private:
    QPoint startPos;
    QPoint currentPos;
    bool active;
    QPen selectionPen;
};