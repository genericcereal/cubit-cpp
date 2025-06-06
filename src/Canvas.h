#pragma once

#include <QGraphicsView>
#include <QGraphicsScene>
#include <QGraphicsProxyWidget>
#include <QString>
#include <QList>
#include <QPaintEvent>

class Controls;
class Element;
class ClientRect;
class SelectionBox;
class Frame;
class HoverIndicator;

class Canvas : public QGraphicsView {
    Q_OBJECT
public:
    explicit Canvas(QWidget *parent = nullptr);
    virtual ~Canvas() = default;
    
    // Canvas state management
    QString getMode() const { return mode; }
    void setMode(const QString &newMode);
    
    // Element creation
    Frame* createFrameElement(const QPointF &scenePos, bool withText = false);
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
    
    // Pan/zoom support
    QPoint getPanOffset() const { return panOffset; }
    qreal getZoomScale() const { return zoomScale; }
    void resetZoom();
    
    // Control drag management
    void startControlDrag(const QPoint &globalPos);
    void updateControlDrag(const QPoint &globalPos);
    void endControlDrag(const QPoint &globalPos);
    Controls* getControls() const { return controls; }
    
    // Selection management
    QList<Frame*> getSelectedFrames() const;
    
    // Hover management
    QString getHoveredElement() const { return hoveredElement; }
    void setHoveredElement(const QString &elementId);
    
    // Parent-child management
    void setElementParent(int childId, int parentId);
    QList<Element*> getChildElements(int parentId) const;
    Element* getElementById(int id) const;
    Frame* findFrameAtPosition(const QPoint &pos) const;

signals:
    void modeChanged(const QString &newMode);
    void elementCreated(const QString &type, const QString &name);
    void overlaysNeedRaise();  // Signal to raise overlay panels
    void selectionChanged();  // Emitted when selection changes
    void propertiesChanged();  // Emitted when properties of selected elements change
    void hoveredElementChanged(const QString &elementId);  // Emitted when hovered element changes

private slots:
    void onControlsRectChanged(const QRect &newRect);
    void onControlsInnerRectClicked(const QPoint &globalPos);
    void onControlsInnerRectDoubleClicked(const QPoint &globalPos);

protected:
    void resizeEvent(QResizeEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;
    void wheelEvent(QWheelEvent *event) override;
    void paintEvent(QPaintEvent *event) override;

private:
    QGraphicsScene *scene;
    Controls *controls;
    QGraphicsProxyWidget *controlsProxy;
    SelectionBox *selectionBox;
    HoverIndicator *hoverIndicator;
    QGraphicsProxyWidget *hoverIndicatorProxy;
    
    // Canvas state
    QString mode;
    bool isSelecting;  // Track if we're currently drag-selecting
    bool isSimulatingControlDrag;  // Track if we're simulating control dragging
    bool isPanning;  // Track if we're currently panning
    QPointF selectionStartPos;  // Track where selection started
    QPoint lastPanPos;  // Last mouse position during panning
    
    // Element management
    QList<Element*> elements;  // Stores all types of elements (Frame, Text, Variable)
    QList<QString> selectedElements;  // Stores IDs of selected elements
    QString hoveredElement;  // Stores ID of currently hovered element
    
    // TEMPORARY: Keep these for now to avoid compilation errors
    QPoint panOffset{0, 0};
    qreal zoomScale = 1.0;
    
    // Helper methods
    void cancelActiveTextEditing();
    int getNextElementId() const { return elements.size() + 1; }
};