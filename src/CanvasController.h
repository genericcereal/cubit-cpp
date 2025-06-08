#pragma once
#include <QObject>
#include <QPointF>
#include "Element.h"
class ElementModel;
class SelectionManager;

class CanvasController : public QObject {
    Q_OBJECT
    Q_PROPERTY(QString mode READ mode WRITE setMode NOTIFY modeChanged)
    
public:
    explicit CanvasController(QObject *parent = nullptr);
    
    // Mode management
    QString mode() const { return m_mode; }
    Q_INVOKABLE void setMode(const QString &mode);
    
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
    
    // Selection
    void selectElementsInRect(const QRectF &rect);
    void selectAll();
    void deleteSelectedElements();
    
signals:
    void modeChanged();
    void elementCreated(Element *element);
    
private:
    QString m_mode;
    ElementModel *m_elementModel;
    SelectionManager *m_selectionManager;
    
    // Drag state
    bool m_isDragging;
    QPointF m_dragStartPos;
    QPointF m_dragCurrentPos;
    Element *m_dragElement;
    
    // Helper methods
    void startDrag(qreal x, qreal y);
    void updateDrag(qreal x, qreal y);
    void endDrag();
};