#pragma once
#include <QObject>
#include <QPointF>
#include <QRectF>
#include <QList>
#include <QHash>
#include <memory>

class Element;
class ElementModel;
class QuadTree;

class HitTestService : public QObject {
    Q_OBJECT
    
public:
    enum class CanvasType {
        Design,
        Script
    };
    Q_ENUM(CanvasType)
    
    explicit HitTestService(QObject *parent = nullptr);
    ~HitTestService();
    
    void setElementModel(ElementModel* elementModel);
    void setCanvasType(CanvasType type);
    
    // Hit testing
    Element* hitTest(const QPointF& point) const;
    Element* hitTest(qreal x, qreal y) const { return hitTest(QPointF(x, y)); }
    
    // Spatial queries
    QList<Element*> elementsInRect(const QRectF& rect) const;
    QList<Element*> elementsAt(const QPointF& point) const;
    
    // Spatial index management
    void rebuildSpatialIndex();
    void insertElement(Element* element);
    void removeElement(Element* element);
    void updateElement(Element* element);
    
    // Performance monitoring
    bool isUsingQuadTree() const { return m_useQuadTree; }
    void setUseQuadTree(bool use);
    
signals:
    void elementHit(Element* element);
    void spatialIndexRebuilt();
    
private slots:
    void onElementAdded(Element* element);
    void onElementRemoved(const QString& elementId);
    void onElementUpdated(Element* element);
    
private:
    ElementModel* m_elementModel = nullptr;
    CanvasType m_canvasType = CanvasType::Design;
    mutable std::unique_ptr<QuadTree> m_quadTree;
    bool m_useQuadTree = true;
    bool m_needsRebuild = false;
    
    // Helper to filter elements based on canvas type
    bool shouldTestElement(Element* element) const;
    
    // Get bounds for the quadtree based on all elements
    QRectF calculateBounds() const;
    
    // Track elements for removal
    mutable QHash<QString, Element*> m_elementMap;
};