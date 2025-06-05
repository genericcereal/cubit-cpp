#pragma once

#include <QWidget>
#include <QString>
#include <QList>
#include <QPaintEvent>

class Controls;
class Element;
class ClientRect;

class Canvas : public QWidget {
    Q_OBJECT
public:
    explicit Canvas(QWidget *parent = nullptr);
    virtual ~Canvas() = default;
    
    // Canvas state management
    QString getMode() const { return mode; }
    void setMode(const QString &newMode);
    
    // Element creation
    void createFrame();
    void createText();
    void createVariable();
    
    // Control management
    void showControls(const QRect &rect);
    void hideControls();
    
    // Selection management
    void selectElement(const QString &elementId, bool addToSelection = false);
    void updateControlsVisibility();
    
    // Rendering
    void render();
    
    // Get rendering type
    QString getRenderingType() const { return "CPU"; }

signals:
    void modeChanged(const QString &newMode);
    void elementCreated(const QString &type, const QString &name);
    void overlaysNeedRaise();  // Signal to raise overlay panels

protected:
    void resizeEvent(QResizeEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;
    void paintEvent(QPaintEvent *event) override;

private:
    // Helper methods for CPU rendering
    void renderFrame(QPainter &painter, const QRect &rect, const QColor &color);
    void renderText(QPainter &painter, const QString &text, const QPoint &pos, const QFont &font);
    
    Controls *controls;
    
    // Canvas state
    QString mode;
    
    // Element management
    QList<Element*> elements;  // Stores all types of elements (Frame, Text, Variable)
    QList<QString> selectedElements;  // Stores IDs of selected elements
};