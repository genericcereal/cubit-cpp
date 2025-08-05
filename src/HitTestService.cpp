#include "HitTestService.h"
#include "Element.h"
#include "CanvasElement.h"
#include "DesignElement.h"
#include "ScriptElement.h"
#include "Component.h"
#include "ElementModel.h"
#include "ElementFilterProxy.h"
#include "QuadTree.h"
// #include "ComponentInstanceTemplate.h" // Component system removed
#include "PlatformConfig.h"
#include "Project.h"
#include "CanvasContext.h"
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
    m_visualsCacheValid = false;  // Invalidate cache
    
    if (m_elementModel) {
        // Connect to model signals
        connect(m_elementModel, &ElementModel::elementAdded,
                this, &HitTestService::onElementAdded,
                Qt::UniqueConnection);
        connect(m_elementModel, &ElementModel::elementRemoved,
                this, &HitTestService::onElementRemoved,
                Qt::UniqueConnection);
        connect(m_elementModel, &ElementModel::elementUpdated,
                this, &HitTestService::onElementUpdated,
                Qt::UniqueConnection);
        
        // Rebuild spatial index
        rebuildSpatialIndex();
    }
}

void HitTestService::setCanvasType(CanvasType type)
{
    if (m_canvasType != type) {
        m_canvasType = type;
        m_visualsCacheValid = false;  // Invalidate cache
        rebuildSpatialIndex();
    }
}

void HitTestService::setEditingElement(QObject* editingElement)
{
    if (m_editingElement != editingElement) {
        m_editingElement = editingElement;
        m_visualsCacheValid = false;  // Invalidate cache
        // Rebuild spatial index when editing element changes
        // This is necessary because the set of hit-testable elements changes
        rebuildSpatialIndex();
    }
}

void HitTestService::setCanvasContext(CanvasContext* context)
{
    if (m_canvasContext != context) {
        m_canvasContext = context;
        m_visualsCacheValid = false;  // Invalidate cache
        
        // Let the context configure hit testing if needed
        if (m_canvasContext) {
            m_canvasContext->configureHitTestService(this);
        }
        
        // Don't rebuild spatial index here - let the caller do it
        // after all setup is complete. This prevents rebuilding before
        // contexts have had a chance to add their elements.
        m_needsRebuild = true;
    }
}

void HitTestService::setElementFilterProxy(ElementFilterProxy* filterProxy)
{
    if (m_filterProxy != filterProxy) {
        // Disconnect from old filter proxy
        if (m_filterProxy) {
            disconnect(m_filterProxy, &ElementFilterProxy::filterChanged,
                      this, &HitTestService::rebuildSpatialIndex);
        }
        
        m_filterProxy = filterProxy;
        m_visualsCacheValid = false;  // Invalidate cache
        
        // Connect to new filter proxy
        if (m_filterProxy) {
            connect(m_filterProxy, &ElementFilterProxy::filterChanged,
                   this, &HitTestService::rebuildSpatialIndex,
                   Qt::UniqueConnection);
        }
        
        // Rebuild spatial index with new filter
        rebuildSpatialIndex();
    }
}

Element* HitTestService::hitTest(const QPointF& point) const
{
    if (!m_elementModel) return nullptr;
    
    // Use findDeepest to get the most deeply nested element
    Element* result = findDeepest(point, [this](Element* e) {
        bool skip = !shouldTestElement(e);
        return skip;
    });
    
    return result;
}

Element* HitTestService::hitTestForHover(const QPointF& point) const
{
    if (!m_elementModel) return nullptr;
    
    // Use findDeepest to get the most deeply nested element for hover
    Element* result = findDeepest(point, [this](Element* e) {
        if (!shouldTestElement(e)) {
            return true;
        }
        if (e->isSelected()) {
            return true;
        }
        return false;
    });
    
    return result;
}

std::vector<Element*> HitTestService::elementsInRect(const QRectF& rect) const
{
    if (!m_elementModel) return std::vector<Element*>();
    
    std::vector<Element*> result;
    auto candidates = queryCandidates(rect);
    
    for (Element* element : candidates) {
        if (!shouldTestElement(element)) {
            continue;
        }
        
        // Only visual elements can be selected by rectangle
        if (element->isVisual()) {
            CanvasElement* canvasElement = qobject_cast<CanvasElement*>(element);
            if (canvasElement && rect.intersects(canvasElement->cachedBounds())) {
                result.push_back(element);
            }
        }
    }
    
    return result;
}

std::vector<Element*> HitTestService::elementsAt(const QPointF& point) const
{
    if (!m_elementModel) return std::vector<Element*>();
    
    return findAll(point, [this](Element* e) {
        return !shouldTestElement(e);
    });
}

void HitTestService::rebuildSpatialIndex()
{
    if (!m_elementModel) return;
    
    // First rebuild the visuals cache
    rebuildVisualsCache();
    
    // Calculate bounds for all elements
    QRectF bounds = calculateBounds();
    if (bounds.isEmpty()) {
        // Default bounds if no elements
        bounds = QRectF(0, 0, 10000, 10000);
    }
    
    // Create new quadtree
    m_quadTree = std::make_unique<QuadTree>(bounds);
    m_elementMap.clear();
    
    // Add all visual elements to the quadtree
    for (CanvasElement* ce : m_visualElements) {
        m_quadTree->insert(ce);
        m_elementMap[ce->getId()] = ce;
    }
    
    m_needsRebuild = false;
    emit spatialIndexRebuilt();
    
    // Spatial index rebuilt
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
                const QRectF& elementRect = canvasElement->cachedBounds();
                
                // Check if element is outside current quadtree bounds
                if (!m_quadTree->getBounds().contains(elementRect)) {
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
    m_visualsCacheValid = false;  // Invalidate cache
    
    // For programmatically created elements, we need to ensure the spatial index
    // is properly updated. Instead of just inserting, rebuild if needed.
    if (m_needsRebuild || !m_quadTree) {
        rebuildSpatialIndex();
    } else {
        insertElement(element);
    }
}

void HitTestService::onElementRemoved(const QString& elementId)
{
    m_visualsCacheValid = false;  // Invalidate cache
    if (Element* element = m_elementMap.value(elementId)) {
        removeElement(element);
    }
}

void HitTestService::onElementUpdated(Element* element)
{
    m_visualsCacheValid = false;  // Invalidate cache
    updateElement(element);
}

QRectF HitTestService::calculateBounds() const
{
    if (!m_elementModel) return QRectF();
    
    // Ensure visual cache is up to date
    if (!m_visualsCacheValid) {
        rebuildVisualsCache();
    }
    
    QRectF bounds;
    
    for (CanvasElement* ce : m_visualElements) {
        const QRectF& elementRect = ce->cachedBounds();
        if (bounds.isEmpty()) {
            bounds = elementRect;
        } else {
            bounds = bounds.united(elementRect);
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
    
    return bounds;
}


bool HitTestService::shouldTestElement(Element* element) const
{
    if (!element) return false;
    
    // If we have an ElementFilterProxy, use it to determine visibility
    // This ensures hit testing is consistent with what's shown in the ElementList
    if (m_filterProxy) {
        // Use the public method to check if element should be shown
        if (!m_filterProxy->shouldShowElement(element)) {
            return false;
        }
    }
    
    // If we have a canvas context, let it decide
    if (m_canvasContext) {
        return m_canvasContext->shouldIncludeInHitTest(element);
    }
    
    // If ElementFilterProxy is not set, fall back to basic checks
    // Check if mouse events are disabled for this element
    if (element->isVisual()) {
        CanvasElement* canvasElement = qobject_cast<CanvasElement*>(element);
        if (canvasElement && !canvasElement->mouseEventsEnabled()) {
            return false;
        }
    }
    
    // Non-visual elements are never hit-testable
    if (!element->isVisual()) {
        return false;
    }
    
    // Check if it's a canvas element
    CanvasElement* canvasElement = qobject_cast<CanvasElement*>(element);
    if (!canvasElement) {
        return false;
    }
    
    // Apply view mode filtering (matching ElementFilterProxy logic)
    if (m_canvasType == CanvasType::Design) {
        // In design mode, test design elements
        if (!canvasElement->isDesignElement()) {
            return false;
        }
        return true;
    }
    else if (m_canvasType == CanvasType::Script) {
        // In script mode, only test script elements (nodes and edges)
        return canvasElement->isScriptElement();
    }
    
    return false;
}

void HitTestService::rebuildVisualsCache() const
{
    if (!m_elementModel) {
        m_visualElements.clear();
        m_visualsCacheValid = true;
        return;
    }
    
    // Clear existing cache
    m_visualElements.clear();
    
    // Reserve space for efficiency
    QList<Element*> allElements = m_elementModel->getAllElements();
    m_visualElements.reserve(allElements.size());
    
    // Build cache of visual elements that should be tested
    for (Element* e : allElements) {
        if (shouldTestElement(e) && e->isVisual()) {
            // Since we already checked isVisual(), static_cast is safe
            m_visualElements.push_back(static_cast<CanvasElement*>(e));
        }
    }
    
    m_visualsCacheValid = true;
}

std::vector<Element*> HitTestService::queryCandidates(const QPointF& point) const
{
    if (!m_visualsCacheValid) {
        rebuildVisualsCache();
    }
    
    if (m_useQuadTree && m_quadTree) {
        return m_quadTree->query(point);
    } else {
        // Return visual elements as Element* list for compatibility
        std::vector<Element*> result;
        result.reserve(m_visualElements.size());
        for (CanvasElement* ce : m_visualElements) {
            result.push_back(ce);
        }
        return result;
    }
}

std::vector<Element*> HitTestService::queryCandidates(const QRectF& rect) const
{
    if (!m_visualsCacheValid) {
        rebuildVisualsCache();
    }
    
    if (m_useQuadTree && m_quadTree) {
        return m_quadTree->query(rect);
    } else {
        // Return visual elements as Element* list for compatibility
        std::vector<Element*> result;
        result.reserve(m_visualElements.size());
        for (CanvasElement* ce : m_visualElements) {
            result.push_back(ce);
        }
        return result;
    }
}

// Template implementations
template<typename Pred>
Element* HitTestService::findTopmost(const QPointF& pt, Pred shouldSkip) const {
    auto candidates = queryCandidates(pt);
    
    // Test in reverse order (top to bottom) for proper z-ordering
    for (int i = int(candidates.size()) - 1; i >= 0; --i) {
        Element* e = candidates.at(i);
        
        if (shouldSkip(e)) continue;
        
        // If we're using cached visuals, we know it's already a CanvasElement
        // and it's visual, so we can skip those checks
        CanvasElement* ce = static_cast<CanvasElement*>(e);
        if (!ce->containsPoint(pt)) continue;
        
        return e;
    }
    return nullptr;
}

template<typename Pred>
std::vector<Element*> HitTestService::findAll(const QPointF& pt, Pred shouldSkip) const {
    std::vector<Element*> result;
    auto candidates = queryCandidates(pt);
    
    for (Element* e : candidates) {
        if (shouldSkip(e)) continue;
        
        // If we're using cached visuals, we know it's already a CanvasElement
        CanvasElement* ce = static_cast<CanvasElement*>(e);
        if (ce->containsPoint(pt)) {
            result.push_back(e);
        }
    }
    
    return result;
}

template<typename Pred>
Element* HitTestService::findDeepest(const QPointF& pt, Pred shouldSkip) const {
    if (!m_elementModel) return nullptr;
    
    // Get all elements that contain the point
    auto allAtPoint = findAll(pt, shouldSkip);
    if (allAtPoint.empty()) return nullptr;
    
    // Find the deepest element (one that has no children containing the point)
    Element* deepest = nullptr;
    int maxDepth = -1;
    
    for (Element* e : allAtPoint) {
        // Calculate depth of this element
        int depth = 0;
        Element* current = e;
        while (current && !current->getParentElementId().isEmpty()) {
            current = m_elementModel->getElementById(current->getParentElementId());
            depth++;
        }
        
        // Check if this is deeper than our current deepest
        if (depth > maxDepth) {
            maxDepth = depth;
            deepest = e;
        }
    }
    
    return deepest;
}