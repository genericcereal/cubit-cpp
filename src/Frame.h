#pragma once
#include "Element.h"
#include "Config.h"
#include <QString>
#include <QColor>

class Frame : public Element {
    Q_OBJECT
public:
    explicit Frame(int id, QWidget *parent = nullptr);
    
    // Override Element virtual methods
    int getId() const override { return elementId; }
    QString getName() const override { return frameName; }
    
    // Get frame color
    QColor getColor() const { 
        return QColor(Config::Colors::FRAME_R, 
                     Config::Colors::FRAME_G, 
                     Config::Colors::FRAME_B); 
    }
    
    // Overflow property
    QString getOverflow() const { return overflow; }
    void setOverflow(const QString& value);
    
    // Override visual update
    void updateParentVisualState() override;

signals:
    void overflowChanged();
    
protected:
    // Override paint event to handle clipping
    void paintEvent(QPaintEvent *event) override;
    // Override resize event to update clipping
    void resizeEvent(QResizeEvent *event) override;

private:
    QString frameName;
    QString overflow;
    
    void updateOverflowClipping();
};