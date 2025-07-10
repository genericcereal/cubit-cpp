#include "HitTestService.h"
#include "Element.h"
#include "CanvasElement.h"
#include "DesignElement.h"
#include "ScriptElement.h"
#include "ElementModel.h"
#include "QuadTree.h"
#include "Component.h"
#include "ComponentVariant.h"
#include "ComponentInstance.h"
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
        // In variant mode, we need to rebuild when editing element changes
        if (m_canvasType == CanvasType::Variant) {
            rebuildSpatialIndex();
        }
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
    insertElement(element);
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

bool HitTestService::isInAnyComponentVariants(Element* element) const
{
    if (!element || !m_elementModel) return false;
    
    // Check all elements to find Components
    QList<Element*> allElements = m_elementModel->getAllElements();
    for (Element* el : allElements) {
        Component* component = qobject_cast<Component*>(el);
        if (component) {
            // Check if element is in this component's variants
            const QList<Element*>& variants = component->variants();
            if (variants.contains(element)) {
                return true;
            }
            
            // Also check if element is a descendant of any variant
            for (Element* variant : variants) {
                QList<Element*> descendants = m_elementModel->getChildrenRecursive(variant->getId());
                if (descendants.contains(element)) {
                    return true;
                }
            }
        }
    }
    
    return false;
}

bool HitTestService::shouldTestElement(Element* element) const
{
    if (!element) return false;
    
    // Special handling for Component elements - they should never be hit-testable on canvas
    // Components only appear in the ElementList, not as visual elements on canvas
    if (qobject_cast<Component*>(element)) {
        return false;
    }
    
    // Check if mouse events are disabled for this element
    if (element->isVisual()) {
        CanvasElement* canvasElement = qobject_cast<CanvasElement*>(element);
        if (canvasElement && !canvasElement->mouseEventsEnabled()) {
            return false;
        }
    }
    
    // Non-visual elements (except Components which are already handled) are never hit-testable
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
        // In design mode, test:
        // 1. Design elements (Frame, Text) that are NOT in any component's variants
        // 2. ComponentInstance elements
        
        // Check if it's a ComponentInstance
        if (qobject_cast<ComponentInstance*>(element)) {
            return true;
        }
        
        // For other elements, they must be design elements
        if (!canvasElement->isDesignElement()) {
            return false;
        }
        
        // Check if this element is a ComponentVariant (should not be hit-testable in design mode)
        if (qobject_cast<ComponentVariant*>(element)) {
            return false;
        }
        
        // Skip elements that are in any component's variants array
        if (isInAnyComponentVariants(element)) {
            return false;
        }
        
        return true;
    }
    else if (m_canvasType == CanvasType::Script) {
        // In script mode, only test script elements (nodes and edges)
        return canvasElement->isScriptElement();
    }
    else if (m_canvasType == CanvasType::Variant) {
        // In variant mode, only test elements that belong to the editing component's variants
        if (!canvasElement->isDesignElement()) {
            return false;
        }
        
        // Check if we have an editing component
        if (m_editingElement) {
            Component* component = qobject_cast<Component*>(m_editingElement);
            if (component) {
                // Test elements that are in this component's variants array or descendants of variants
                const QList<Element*>& variants = component->variants();
                
                // Check if element is directly in variants
                if (variants.contains(element)) {
                    return true;
                }
                
                // Check if element is a descendant of any variant
                if (m_elementModel) {
                    for (Element* variant : variants) {
                        QList<Element*> descendants = m_elementModel->getChildrenRecursive(variant->getId());
                        if (descendants.contains(element)) {
                            return true;
                        }
                    }
                }
            }
        }
        
        // If no editing component, don't test any elements
        return false;
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