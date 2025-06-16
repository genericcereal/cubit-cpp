#pragma once
#include <QRectF>
#include <QPointF>
#include <QList>
#include <memory>
#include <vector>

class Element;
class CanvasElement;

class QuadTree {
public:
    // Constructor with bounds and capacity
    QuadTree(const QRectF& bounds, int nodeCapacity = 4);
    ~QuadTree() = default;
    
    // Insert an element into the quadtree
    bool insert(Element* element);
    
    // Remove an element from the quadtree
    bool remove(Element* element);
    
    // Query elements at a specific point
    QList<Element*> query(const QPointF& point) const;
    
    // Query elements within a rectangle
    QList<Element*> query(const QRectF& rect) const;
    
    // Clear all elements
    void clear();
    
    // Rebuild the entire tree (useful after many changes)
    void rebuild(const QList<Element*>& elements);
    
    // Get statistics for debugging
    struct Stats {
        int totalNodes = 0;
        int totalElements = 0;
        int maxDepth = 0;
    };
    Stats getStats() const;
    
    // Get the bounds of the quadtree
    QRectF getBounds() const;
    
private:
    struct Node {
        QRectF bounds;
        std::vector<Element*> elements;
        std::unique_ptr<Node> northWest;
        std::unique_ptr<Node> northEast;
        std::unique_ptr<Node> southWest;
        std::unique_ptr<Node> southEast;
        
        Node(const QRectF& b) : bounds(b) {}
        bool isLeaf() const { return !northWest; }
    };
    
    std::unique_ptr<Node> m_root;
    int m_nodeCapacity;
    
    // Helper methods
    void subdivide(Node* node);
    bool insertIntoNode(Node* node, Element* element, const QRectF& elementBounds, int depth = 0);
    bool removeFromNode(Node* node, Element* element);
    void queryNode(const Node* node, const QPointF& point, QList<Element*>& result) const;
    void queryNode(const Node* node, const QRectF& rect, QList<Element*>& result) const;
    void clearNode(Node* node);
    void getStatsForNode(const Node* node, Stats& stats, int depth) const;
    
    // Get the quadrant for a rectangle within bounds
    int getQuadrant(const QRectF& bounds, const QRectF& rect) const;
    
    // Maximum depth to prevent infinite recursion
    static constexpr int MAX_DEPTH = 8;
};