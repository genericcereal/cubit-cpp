#pragma once

#include <QWidget>
#include <QString>
#include <QList>
#include <QPaintEvent>

class Controls;
class Element;
class ClientRect;
class SelectionBox;

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
    
    // Pan management
    QPoint getPanOffset() const { return panOffset; }
    void setPanOffset(const QPoint &offset);
    void setPanOffset(int x, int y) { setPanOffset(QPoint(x, y)); }
    
    // Control drag management
    void startControlDrag(const QPoint &globalPos);
    void updateControlDrag(const QPoint &globalPos);
    void endControlDrag(const QPoint &globalPos);
    Controls* getControls() const { return controls; }

signals:
    void modeChanged(const QString &newMode);
    void elementCreated(const QString &type, const QString &name);
    void overlaysNeedRaise();  // Signal to raise overlay panels

private slots:
    void onControlsRectChanged(const QRect &newRect);
    void onControlsInnerRectClicked(const QPoint &globalPos);

protected:
    void resizeEvent(QResizeEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;
    void wheelEvent(QWheelEvent *event) override;
    void paintEvent(QPaintEvent *event) override;

private:
    Controls *controls;
    SelectionBox *selectionBox;
    
    // Canvas state
    QString mode;
    bool isSelecting;  // Track if we're currently drag-selecting
    
    // Element management
    QList<Element*> elements;  // Stores all types of elements (Frame, Text, Variable)
    QList<QString> selectedElements;  // Stores IDs of selected elements
    
    // Pan state
    QPoint panOffset;  // Canvas pan offset (x, y)
};