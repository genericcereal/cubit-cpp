#pragma once
#include <QObject>
#include <QPointF>
#include <QRectF>
#include <QHash>
#include <memory>
#include <vector>

class Element;
class CanvasElement;
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
    
    // Hit testing for hover (excludes selected elements)
    Element* hitTestForHover(const QPointF& point) const;
    Element* hitTestForHover(qreal x, qreal y) const { return hitTestForHover(QPointF(x, y)); }
    
    // Spatial queries
    std::vector<Element*> elementsInRect(const QRectF& rect) const;
    std::vector<Element*> elementsAt(const QPointF& point) const;
    
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
    
    // Cached visual elements for fast hit testing
    mutable std::vector<CanvasElement*> m_visualElements;
    mutable bool m_visualsCacheValid = false;
    
    // Rebuild the visual elements cache
    void rebuildVisualsCache() const;
    
    // Query candidates from spatial index or fallback
    std::vector<Element*> queryCandidates(const QPointF& point) const;
    std::vector<Element*> queryCandidates(const QRectF& rect) const;
    
    // Unified helper for finding topmost element with custom predicate
    template<typename Pred>
    Element* findTopmost(const QPointF& pt, Pred shouldSkip) const;
    
    // Unified helper for finding all elements with custom predicate
    template<typename Pred>
    std::vector<Element*> findAll(const QPointF& pt, Pred shouldSkip) const;
};