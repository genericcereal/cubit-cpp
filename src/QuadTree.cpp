#include "QuadTree.h"
#include "Element.h"
#include "CanvasElement.h"
#include <QDebug>

QuadTree::QuadTree(const QRectF& bounds, int nodeCapacity)
    : m_nodeCapacity(nodeCapacity)
{
    m_root = std::make_unique<Node>(bounds);
}

bool QuadTree::insert(Element* element)
{
    if (!element || !element->isVisual()) {
        return false;
    }
    
    CanvasElement* canvasElement = qobject_cast<CanvasElement*>(element);
    if (!canvasElement) {
        return false;
    }
    
    QRectF elementBounds = canvasElement->rect();
    return insertIntoNode(m_root.get(), element, elementBounds);
}

bool QuadTree::remove(Element* element)
{
    if (!element) {
        return false;
    }
    
    return removeFromNode(m_root.get(), element);
}

QList<Element*> QuadTree::query(const QPointF& point) const
{
    QList<Element*> result;
    if (m_root) {
        queryNode(m_root.get(), point, result);
    }
    return result;
}

QList<Element*> QuadTree::query(const QRectF& rect) const
{
    QList<Element*> result;
    if (m_root) {
        queryNode(m_root.get(), rect, result);
    }
    return result;
}

void QuadTree::clear()
{
    if (m_root) {
        clearNode(m_root.get());
    }
}

void QuadTree::rebuild(const QList<Element*>& elements)
{
    QRectF bounds = m_root ? m_root->bounds : QRectF();
    clear();
    m_root = std::make_unique<Node>(bounds);
    
    for (Element* element : elements) {
        insert(element);
    }
}

QuadTree::Stats QuadTree::getStats() const
{
    Stats stats;
    if (m_root) {
        getStatsForNode(m_root.get(), stats, 0);
    }
    return stats;
}

QRectF QuadTree::getBounds() const
{
    if (m_root) {
        return m_root->bounds;
    }
    return QRectF();
}

void QuadTree::subdivide(Node* node)
{
    if (!node->isLeaf()) {
        return; // Already subdivided
    }
    
    qreal x = node->bounds.x();
    qreal y = node->bounds.y();
    qreal halfWidth = node->bounds.width() / 2.0;
    qreal halfHeight = node->bounds.height() / 2.0;
    
    node->northWest = std::make_unique<Node>(QRectF(x, y, halfWidth, halfHeight));
    node->northEast = std::make_unique<Node>(QRectF(x + halfWidth, y, halfWidth, halfHeight));
    node->southWest = std::make_unique<Node>(QRectF(x, y + halfHeight, halfWidth, halfHeight));
    node->southEast = std::make_unique<Node>(QRectF(x + halfWidth, y + halfHeight, halfWidth, halfHeight));
}

bool QuadTree::insertIntoNode(Node* node, Element* element, const QRectF& elementBounds, int depth)
{
    if (!node || !node->bounds.contains(elementBounds)) {
        return false; // Element doesn't fit in this node
    }
    
    // If we're at max depth or this is a leaf with space, insert here
    if (depth >= MAX_DEPTH || (node->isLeaf() && node->elements.size() < static_cast<size_t>(m_nodeCapacity))) {
        node->elements.push_back(element);
        return true;
    }
    
    // If this is a leaf that's full, subdivide
    if (node->isLeaf()) {
        subdivide(node);
        
        // Redistribute existing elements
        std::vector<Element*> elementsToRedistribute;
        elementsToRedistribute.swap(node->elements);
        
        for (Element* existingElement : elementsToRedistribute) {
            if (existingElement->isVisual()) {
                CanvasElement* canvasElem = qobject_cast<CanvasElement*>(existingElement);
                if (canvasElem) {
                    QRectF existingBounds = canvasElem->rect();
                    int quadrant = getQuadrant(node->bounds, existingBounds);
                    
                    if (quadrant != -1) {
                        // Element fits entirely in one quadrant
                        Node* child = nullptr;
                        switch (quadrant) {
                            case 0: child = node->northWest.get(); break;
                            case 1: child = node->northEast.get(); break;
                            case 2: child = node->southWest.get(); break;
                            case 3: child = node->southEast.get(); break;
                        }
                        if (child) {
                            insertIntoNode(child, existingElement, existingBounds, depth + 1);
                        }
                    } else {
                        // Element spans multiple quadrants, keep at this level
                        node->elements.push_back(existingElement);
                    }
                }
            }
        }
    }
    
    // Try to insert the new element into a child quadrant
    int quadrant = getQuadrant(node->bounds, elementBounds);
    if (quadrant != -1 && !node->isLeaf()) {
        Node* child = nullptr;
        switch (quadrant) {
            case 0: child = node->northWest.get(); break;
            case 1: child = node->northEast.get(); break;
            case 2: child = node->southWest.get(); break;
            case 3: child = node->southEast.get(); break;
        }
        if (child) {
            return insertIntoNode(child, element, elementBounds, depth + 1);
        }
    }
    
    // Element spans multiple quadrants, store at this level
    node->elements.push_back(element);
    return true;
}

bool QuadTree::removeFromNode(Node* node, Element* element)
{
    if (!node) {
        return false;
    }
    
    // Check this node's elements
    auto it = std::find(node->elements.begin(), node->elements.end(), element);
    if (it != node->elements.end()) {
        node->elements.erase(it);
        return true;
    }
    
    // Check child nodes
    if (!node->isLeaf()) {
        return removeFromNode(node->northWest.get(), element) ||
               removeFromNode(node->northEast.get(), element) ||
               removeFromNode(node->southWest.get(), element) ||
               removeFromNode(node->southEast.get(), element);
    }
    
    return false;
}

void QuadTree::queryNode(const Node* node, const QPointF& point, QList<Element*>& result) const
{
    if (!node || !node->bounds.contains(point)) {
        return;
    }
    
    // Check elements at this node
    for (Element* element : node->elements) {
        if (element && element->isVisual()) {
            CanvasElement* canvasElement = qobject_cast<CanvasElement*>(element);
            if (canvasElement && canvasElement->containsPoint(point)) {
                result.append(element);
            }
        }
    }
    
    // Check child nodes
    if (!node->isLeaf()) {
        queryNode(node->northWest.get(), point, result);
        queryNode(node->northEast.get(), point, result);
        queryNode(node->southWest.get(), point, result);
        queryNode(node->southEast.get(), point, result);
    }
}

void QuadTree::queryNode(const Node* node, const QRectF& rect, QList<Element*>& result) const
{
    if (!node || !node->bounds.intersects(rect)) {
        return;
    }
    
    // Check elements at this node
    for (Element* element : node->elements) {
        if (element && element->isVisual()) {
            CanvasElement* canvasElement = qobject_cast<CanvasElement*>(element);
            if (canvasElement && rect.intersects(canvasElement->rect())) {
                result.append(element);
            }
        }
    }
    
    // Check child nodes
    if (!node->isLeaf()) {
        queryNode(node->northWest.get(), rect, result);
        queryNode(node->northEast.get(), rect, result);
        queryNode(node->southWest.get(), rect, result);
        queryNode(node->southEast.get(), rect, result);
    }
}

void QuadTree::clearNode(Node* node)
{
    if (!node) {
        return;
    }
    
    node->elements.clear();
    
    if (!node->isLeaf()) {
        clearNode(node->northWest.get());
        clearNode(node->northEast.get());
        clearNode(node->southWest.get());
        clearNode(node->southEast.get());
        
        node->northWest.reset();
        node->northEast.reset();
        node->southWest.reset();
        node->southEast.reset();
    }
}

void QuadTree::getStatsForNode(const Node* node, Stats& stats, int depth) const
{
    if (!node) {
        return;
    }
    
    stats.totalNodes++;
    stats.totalElements += static_cast<int>(node->elements.size());
    stats.maxDepth = std::max(stats.maxDepth, depth);
    
    if (!node->isLeaf()) {
        getStatsForNode(node->northWest.get(), stats, depth + 1);
        getStatsForNode(node->northEast.get(), stats, depth + 1);
        getStatsForNode(node->southWest.get(), stats, depth + 1);
        getStatsForNode(node->southEast.get(), stats, depth + 1);
    }
}

int QuadTree::getQuadrant(const QRectF& bounds, const QRectF& rect) const
{
    qreal centerX = bounds.x() + bounds.width() / 2.0;
    qreal centerY = bounds.y() + bounds.height() / 2.0;
    
    bool inNorth = rect.bottom() <= centerY;
    bool inSouth = rect.top() >= centerY;
    bool inWest = rect.right() <= centerX;
    bool inEast = rect.left() >= centerX;
    
    if (inNorth && inWest) return 0; // NW
    if (inNorth && inEast) return 1; // NE
    if (inSouth && inWest) return 2; // SW
    if (inSouth && inEast) return 3; // SE
    
    return -1; // Spans multiple quadrants
}