#include "HitTestService.h"
#include "Element.h"
#include "CanvasElement.h"
#include "ElementModel.h"
#include "QuadTree.h"
#include <QDebug>
#include <QElapsedTimer>

HitTestService::HitTestService(QObject *parent)
    : QObject(parent)
{
}

HitTestService::~HitTestService() = default;

void HitTestService::setElementModel(ElementModel* elementModel)
{
    if (m_elementModel) {
        // Disconnect old model
        disconnect(m_elementModel, &ElementModel::elementAdded,
                   this, &HitTestService::onElementAdded);
        disconnect(m_elementModel, &ElementModel::elementRemoved,
                   this, &HitTestService::onElementRemoved);
        disconnect(m_elementModel, &ElementModel::elementUpdated,
                   this, &HitTestService::onElementUpdated);
    }
    
    m_elementModel = elementModel;
    
    if (m_elementModel) {
        // Connect to model signals
        connect(m_elementModel, &ElementModel::elementAdded,
                this, &HitTestService::onElementAdded);
        connect(m_elementModel, &ElementModel::elementRemoved,
                this, &HitTestService::onElementRemoved);
        connect(m_elementModel, &ElementModel::elementUpdated,
                this, &HitTestService::onElementUpdated);
        
        // Rebuild spatial index
        rebuildSpatialIndex();
    }
}

void HitTestService::setCanvasType(CanvasType type)
{
    if (m_canvasType != type) {
        m_canvasType = type;
        rebuildSpatialIndex();
    }
}

Element* HitTestService::hitTest(const QPointF& point) const
{
    if (!m_elementModel) return nullptr;
    
    QList<Element*> candidates;
    
    if (m_useQuadTree && m_quadTree) {
        // Use quadtree for efficient spatial query
        candidates = m_quadTree->query(point);
    } else {
        // Fallback to linear search
        candidates = m_elementModel->getAllElements();
    }
    
    qDebug() << "HitTestService::hitTest at" << point << "- found" << candidates.size() << "candidates";
    
    // Test in reverse order (top to bottom) for proper z-ordering
    for (int i = candidates.size() - 1; i >= 0; --i) {
        Element *element = candidates[i];
        
        if (!shouldTestElement(element)) {
            qDebug() << "  Skipping element" << element->getName() << "- shouldTestElement returned false";
            continue;
        }
        
        // Only visual elements can be hit tested
        if (element->isVisual()) {
            CanvasElement* canvasElement = qobject_cast<CanvasElement*>(element);
            if (canvasElement) {
                QRectF elementRect = canvasElement->rect();
                bool contains = canvasElement->containsPoint(point);
                qDebug() << "  Testing element" << element->getName() 
                         << "at" << elementRect 
                         << "- contains:" << contains;
                if (contains) {
                    return element;
                }
            }
        }
    }
    
    qDebug() << "  No element found at point";
    return nullptr;
}

Element* HitTestService::hitTestForHover(const QPointF& point) const
{
    if (!m_elementModel) return nullptr;
    
    QList<Element*> candidates;
    
    if (m_useQuadTree && m_quadTree) {
        // Use quadtree for efficient spatial query
        candidates = m_quadTree->query(point);
    } else {
        // Fallback to linear search
        candidates = m_elementModel->getAllElements();
    }
    
    qDebug() << "HitTestService::hitTestForHover at" << point << "- found" << candidates.size() << "candidates";
    
    // Test in reverse order (top to bottom) for proper z-ordering
    for (int i = candidates.size() - 1; i >= 0; --i) {
        Element *element = candidates[i];
        
        if (!shouldTestElement(element)) {
            qDebug() << "  Skipping element" << element->getName() << "- shouldTestElement returned false";
            continue;
        }
        
        // Skip selected elements for hover
        if (element->isSelected()) {
            qDebug() << "  Skipping element" << element->getName() << "- element is selected";
            continue;
        }
        
        // Only visual elements can be hit tested
        if (element->isVisual()) {
            CanvasElement* canvasElement = qobject_cast<CanvasElement*>(element);
            if (canvasElement) {
                QRectF elementRect = canvasElement->rect();
                bool contains = canvasElement->containsPoint(point);
                qDebug() << "  Testing element" << element->getName() 
                         << "at" << elementRect 
                         << "- contains:" << contains;
                if (contains) {
                    return element;
                }
            }
        }
    }
    
    qDebug() << "  No element found at point";
    return nullptr;
}

QList<Element*> HitTestService::elementsInRect(const QRectF& rect) const
{
    QList<Element*> result;
    
    if (!m_elementModel) return result;
    
    QList<Element*> candidates;
    
    if (m_useQuadTree && m_quadTree) {
        // Use quadtree for efficient spatial query
        candidates = m_quadTree->query(rect);
    } else {
        // Fallback to linear search
        candidates = m_elementModel->getAllElements();
    }
    
    for (Element *element : candidates) {
        if (!shouldTestElement(element)) {
            continue;
        }
        
        // Only visual elements can be selected by rectangle
        if (element->isVisual()) {
            CanvasElement* canvasElement = qobject_cast<CanvasElement*>(element);
            if (canvasElement && rect.intersects(canvasElement->rect())) {
                result.append(element);
            }
        }
    }
    
    return result;
}

QList<Element*> HitTestService::elementsAt(const QPointF& point) const
{
    QList<Element*> result;
    
    if (!m_elementModel) return result;
    
    QList<Element*> candidates;
    
    if (m_useQuadTree && m_quadTree) {
        // Use quadtree for efficient spatial query
        candidates = m_quadTree->query(point);
    } else {
        // Fallback to linear search
        candidates = m_elementModel->getAllElements();
    }
    
    // Test all elements at this point (not just the topmost)
    for (Element *element : candidates) {
        if (!shouldTestElement(element)) {
            continue;
        }
        
        if (element->isVisual()) {
            CanvasElement* canvasElement = qobject_cast<CanvasElement*>(element);
            if (canvasElement && canvasElement->containsPoint(point)) {
                result.append(element);
            }
        }
    }
    
    return result;
}

void HitTestService::rebuildSpatialIndex()
{
    if (!m_elementModel) return;
    
    // Calculate bounds for all elements
    QRectF bounds = calculateBounds();
    if (bounds.isEmpty()) {
        // Default bounds if no elements
        bounds = QRectF(0, 0, 10000, 10000);
    }
    
    // Create new quadtree
    m_quadTree = std::make_unique<QuadTree>(bounds);
    m_elementMap.clear();
    
    // Add all appropriate elements to the quadtree
    QList<Element*> elements = m_elementModel->getAllElements();
    for (Element* element : elements) {
        if (shouldTestElement(element)) {
            m_quadTree->insert(element);
            m_elementMap[element->getId()] = element;
        }
    }
    
    m_needsRebuild = false;
    emit spatialIndexRebuilt();
    
    // Debug output
    QuadTree::Stats stats = m_quadTree->getStats();
    qDebug() << "QuadTree rebuilt - Nodes:" << stats.totalNodes 
             << "Elements:" << stats.totalElements 
             << "Depth:" << stats.maxDepth;
}

void HitTestService::insertElement(Element* element)
{
    if (!element || !shouldTestElement(element)) return;
    
    if (m_quadTree) {
        m_quadTree->insert(element);
        m_elementMap[element->getId()] = element;
    }
}

void HitTestService::removeElement(Element* element)
{
    if (!element) return;
    
    if (m_quadTree) {
        m_quadTree->remove(element);
        m_elementMap.remove(element->getId());
    }
}

void HitTestService::updateElement(Element* element)
{
    if (!element || !shouldTestElement(element)) return;
    
    if (m_quadTree) {
        bool needsRebuild = false;
        
        if (element->isVisual()) {
            CanvasElement* canvasElement = qobject_cast<CanvasElement*>(element);
            if (canvasElement) {
                QRectF elementRect = canvasElement->rect();
                qDebug() << "HitTestService::updateElement -" << element->getName() 
                         << "at" << elementRect;
                
                // Check if element is outside current quadtree bounds
                if (!m_quadTree->getBounds().contains(elementRect)) {
                    qDebug() << "Element" << element->getName() << "is outside QuadTree bounds - rebuilding spatial index";
                    needsRebuild = true;
                }
            }
        }
        
        if (needsRebuild) {
            // Element is outside bounds, need to rebuild the entire spatial index
            rebuildSpatialIndex();
        } else {
            // Normal update - remove and re-insert
            m_quadTree->remove(element);
            bool inserted = m_quadTree->insert(element);
            if (!inserted) {
                qDebug() << "Failed to insert element" << element->getName() << "into QuadTree - rebuilding";
                rebuildSpatialIndex();
            }
        }
    }
}

void HitTestService::setUseQuadTree(bool use)
{
    if (m_useQuadTree != use) {
        m_useQuadTree = use;
        if (use) {
            rebuildSpatialIndex();
        }
    }
}

void HitTestService::onElementAdded(Element* element)
{
    insertElement(element);
}

void HitTestService::onElementRemoved(const QString& elementId)
{
    if (Element* element = m_elementMap.value(elementId)) {
        removeElement(element);
    }
}

void HitTestService::onElementUpdated(Element* element)
{
    updateElement(element);
}

QRectF HitTestService::calculateBounds() const
{
    if (!m_elementModel) return QRectF();
    
    QRectF bounds;
    QList<Element*> elements = m_elementModel->getAllElements();
    
    for (Element* element : elements) {
        if (shouldTestElement(element) && element->isVisual()) {
            CanvasElement* canvasElement = qobject_cast<CanvasElement*>(element);
            if (canvasElement) {
                QRectF elementRect = canvasElement->rect();
                if (bounds.isEmpty()) {
                    bounds = elementRect;
                } else {
                    bounds = bounds.united(elementRect);
                }
            }
        }
    }
    
    // Add generous padding to accommodate future movement
    if (!bounds.isEmpty()) {
        bounds.adjust(-500, -500, 500, 500);
    } else {
        // Default bounds if no elements
        bounds = QRectF(-1000, -1000, 2000, 2000);
    }
    
    // Ensure bounds include the origin
    bounds = bounds.united(QRectF(-100, -100, 200, 200));
    
    qDebug() << "HitTestService::calculateBounds - computed bounds:" << bounds;
    
    return bounds;
}

bool HitTestService::shouldTestElement(Element* element) const
{
    if (!element) return false;
    
    // For script canvas, test nodes and edges
    if (m_canvasType == CanvasType::Script) {
        return element->getType() == Element::NodeType || 
               element->getType() == Element::EdgeType;
    }
    // For design canvas, skip nodes and edges
    else if (m_canvasType == CanvasType::Design) {
        return element->getType() != Element::NodeType && 
               element->getType() != Element::EdgeType;
    }
    
    return true;
}