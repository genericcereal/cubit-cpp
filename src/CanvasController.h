#pragma once
#include <QObject>
#include <QPointF>
#include "Element.h"
class ElementModel;
class SelectionManager;

class CanvasController : public QObject {
    Q_OBJECT
    Q_PROPERTY(QString mode READ mode WRITE setMode NOTIFY modeChanged)
    Q_PROPERTY(QString canvasType READ canvasType WRITE setCanvasType NOTIFY canvasTypeChanged)
    
public:
    explicit CanvasController(QObject *parent = nullptr);
    
    // Mode management
    QString mode() const { return m_mode; }
    Q_INVOKABLE void setMode(const QString &mode);
    
    // Canvas type management
    QString canvasType() const { return m_canvasType; }
    Q_INVOKABLE void setCanvasType(const QString &type);
    
    // Element model and selection manager
    Q_INVOKABLE void setElementModel(ElementModel *model) { m_elementModel = model; }
    Q_INVOKABLE void setSelectionManager(SelectionManager *manager) { m_selectionManager = manager; }
    
    // Helper methods
    Q_INVOKABLE Element* hitTest(qreal x, qreal y);
    
public slots:
    // Mouse handling
    void handleMousePress(qreal x, qreal y);
    void handleMouseMove(qreal x, qreal y);
    void handleMouseRelease(qreal x, qreal y);
    
    // Element creation
    void createElement(const QString &type, qreal x, qreal y, qreal width = 200, qreal height = 150);
    Q_INVOKABLE void createNode(qreal x, qreal y, const QString &title = "Node", const QString &color = "");
    
    // Selection
    void selectElementsInRect(const QRectF &rect);
    void selectAll();
    void deleteSelectedElements();
    
signals:
    void modeChanged();
    void canvasTypeChanged();
    void elementCreated(Element *element);
    
private:
    QString m_mode;
    QString m_canvasType;
    ElementModel *m_elementModel;
    SelectionManager *m_selectionManager;
    
    // Drag state
    bool m_isDragging;
    QPointF m_dragStartPos;
    QPointF m_dragCurrentPos;
    Element *m_dragElement;
    bool m_hasDraggedMinDistance;  // Track if we've moved enough to be considered a drag
    
    // Multi-selection drag state
    struct ElementDragState {
        Element *element;
        QPointF originalPosition;
    };
    QList<ElementDragState> m_draggedElements;
    
    // Helper methods
    void startDrag(qreal x, qreal y);
    void updateDrag(qreal x, qreal y);
    void endDrag();
};