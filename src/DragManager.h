#pragma once
#include <QObject>
#include <QPointF>
#include <QList>

class Element;
class SelectionManager;
class ElementModel;
class CanvasElement;

class DragManager : public QObject {
    Q_OBJECT
    
    Q_PROPERTY(bool isDragging READ isDragging NOTIFY isDraggingChanged)
    
public:
    explicit DragManager(QObject *parent = nullptr);
    ~DragManager() = default;
    
    void setSelectionManager(SelectionManager* selectionManager) { m_selectionManager = selectionManager; }
    void setElementModel(ElementModel* elementModel) { m_elementModel = elementModel; }
    
    // Drag operations
    bool startDrag(Element* element, const QPointF& startPos);
    void updateDrag(const QPointF& currentPos);
    void endDrag();
    
    // Handle click vs drag selection logic
    void handleClickSelection(Element* clickedElement);
    
    // Properties
    bool isDragging() const { return m_isDragging; }
    bool hasDraggedMinDistance() const { return m_hasDraggedMinDistance; }
    Element* dragElement() const { return m_dragElement; }
    bool wasClick() const { return !m_hasDraggedMinDistance; }
    
signals:
    void isDraggingChanged();
    void dragStarted();
    void dragUpdated();
    void dragEnded();
    void dragCompleted(const QList<Element*>& elements, const QPointF& totalDelta);
    
private:
    struct ElementDragState {
        Element* element;
        QPointF originalPosition;
    };
    
    SelectionManager* m_selectionManager = nullptr;
    ElementModel* m_elementModel = nullptr;
    
    bool m_isDragging = false;
    Element* m_dragElement = nullptr;
    QPointF m_dragStartPos;
    QPointF m_totalDelta;
    bool m_hasDraggedMinDistance = false;
    QList<ElementDragState> m_draggedElements;
    
    static constexpr qreal DRAG_THRESHOLD = 3.0; // pixels
    
public:
    QPointF totalDelta() const { return m_totalDelta; }
    QList<Element*> draggedElements() const;
};