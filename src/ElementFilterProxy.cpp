#include "ElementFilterProxy.h"
#include "Element.h"
#include "DesignElement.h"
#include "ScriptElement.h"
#include "Component.h"
#include "Variable.h"
#include "ElementModel.h"
#include <QDebug>

ElementFilterProxy::ElementFilterProxy(QObject *parent)
    : QSortFilterProxyModel(parent)
    , m_viewMode("design")
{
}

QHash<int, QByteArray> ElementFilterProxy::roleNames() const {
    if (sourceModel()) {
        return sourceModel()->roleNames();
    }
    return QSortFilterProxyModel::roleNames();
}

Element* ElementFilterProxy::elementAt(int row) const {
    if (row < 0 || row >= rowCount()) {
        return nullptr;
    }
    
    QModelIndex proxyIndex = index(row, 0);
    if (!proxyIndex.isValid()) {
        return nullptr;
    }
    
    QVariant data = this->data(proxyIndex, Qt::UserRole + 1); // ElementRole
    if (!data.isValid()) {
        return nullptr;
    }
    
    return data.value<Element*>();
}

QString ElementFilterProxy::viewMode() const {
    return m_viewMode;
}

void ElementFilterProxy::setViewMode(const QString& viewMode) {
    if (m_viewMode != viewMode) {
        m_viewMode = viewMode;
        emit viewModeChanged();
        invalidateFilter();
    }
}

QObject* ElementFilterProxy::editingElement() const {
    return m_editingElement;
}

void ElementFilterProxy::setEditingElement(QObject* element) {
    if (m_editingElement != element) {
        m_editingElement = element;
        emit editingElementChanged();
        invalidateFilter();
    }
}

bool ElementFilterProxy::filterComponentsOut() const {
    return m_filterComponentsOut;
}

void ElementFilterProxy::setFilterComponentsOut(bool filter) {
    if (m_filterComponentsOut != filter) {
        m_filterComponentsOut = filter;
        emit filterComponentsOutChanged();
        invalidateFilter();
    }
}

bool ElementFilterProxy::filterComponentsOnly() const {
    return m_filterComponentsOnly;
}

void ElementFilterProxy::setFilterComponentsOnly(bool filter) {
    if (m_filterComponentsOnly != filter) {
        m_filterComponentsOnly = filter;
        emit filterComponentsOnlyChanged();
        invalidateFilter();
    }
}

bool ElementFilterProxy::filterAcceptsRow(int sourceRow, const QModelIndex &sourceParent) const {
    Q_UNUSED(sourceParent)
    
    if (!sourceModel()) {
        qDebug() << "ElementFilterProxy: No source model";
        return false;
    }
    
    QModelIndex index = sourceModel()->index(sourceRow, 0);
    if (!index.isValid()) {
        qDebug() << "ElementFilterProxy: Invalid index for row" << sourceRow;
        return false;
    }
    
    // Get the element from the model data
    QVariant data = sourceModel()->data(index, Qt::UserRole + 1); // ElementRole
    if (!data.isValid()) {
        qDebug() << "ElementFilterProxy: No valid data for row" << sourceRow;
        return false;
    }
    
    Element* element = data.value<Element*>();
    if (!element) {
        qDebug() << "ElementFilterProxy: Null element for row" << sourceRow;
        return false;
    }
    
    bool accepted = shouldShowElementInMode(element);
    // qDebug() << "ElementFilterProxy: Element" << element->getName() << "type" << element->getTypeName() 
    //          << "filterComponentsOut:" << m_filterComponentsOut 
    //          << "filterComponentsOnly:" << m_filterComponentsOnly
    //          << "accepted:" << accepted;
    return accepted;
}

bool ElementFilterProxy::shouldShowElementInMode(Element* element) const {
    if (!element) {
        return false;
    }
    
    // Special handling for Component elements - they should only show in ElementList, not on canvas
    // Since DesignElementLayer uses this filter, Components will be filtered out there
    // But ElementList can still show them
    Component* component = qobject_cast<Component*>(element);
    if (component) {
        // Apply component filtering
        if (m_filterComponentsOut) {
            return false; // Filter out components
        }
        if (m_filterComponentsOnly) {
            return m_viewMode == "design"; // Only show components in design mode
        }
        // Components are only shown in design mode, not in script or variant modes
        return m_viewMode == "design";
    }
    
    // If we're only showing components, filter out non-components
    if (m_filterComponentsOnly) {
        return false;
    }
    
    // Non-visual elements (except Components and Variables) are never shown
    if (!element->isVisual()) {
        // Allow Variables to be shown even though they're non-visual
        if (qobject_cast<Variable*>(element)) {
            return true;
        }
        return false;
    }
    
    // Check if it's a canvas element
    CanvasElement* canvasElement = qobject_cast<CanvasElement*>(element);
    if (!canvasElement) {
        return false;
    }
    
    if (m_viewMode == "design") {
        // In design mode, show:
        // 1. Design elements (Frame, Text) that are NOT in any component's variants
        // 2. ComponentInstance elements
        // Note: Component elements are already handled above
        
        // For other elements, they must be design elements
        if (!canvasElement->isDesignElement()) {
            return false;
        }
        
        // Cast to DesignElement to access component-related methods
        DesignElement* designElement = qobject_cast<DesignElement*>(element);
        if (!designElement) {
            return false;
        }
        
        // Check if it's a ComponentInstance
        if (designElement->isComponentInstance()) {
            return true;
        }
        
        // Check if this element is a ComponentVariant (should be hidden in design mode)
        if (designElement->isComponentVariant()) {
            return false;
        }
        
        // Check if this element has a ComponentVariant as an ancestor
        if (ElementModel* model = qobject_cast<ElementModel*>(sourceModel())) {
            Element* current = element;
            int maxDepth = 100; // Prevent infinite loops
            int depth = 0;
            
            while (current && !current->getParentElementId().isEmpty() && depth < maxDepth) {
                // Find the parent element
                Element* parent = model->getElementById(current->getParentElementId());
                if (!parent) {
                    break;
                }
                
                // Check if the parent is a ComponentVariant
                DesignElement* parentDesign = qobject_cast<DesignElement*>(parent);
                if (parentDesign && parentDesign->isComponentVariant()) {
                    return false; // This element is a descendant of a ComponentVariant
                }
                
                current = parent;
                depth++;
            }
        }
        
        // Check if this element is in any component's variants array
        if (sourceModel()) {
            int rowCount = sourceModel()->rowCount();
            for (int i = 0; i < rowCount; ++i) {
                QModelIndex index = sourceModel()->index(i, 0);
                QVariant data = sourceModel()->data(index, Qt::UserRole + 1); // ElementRole
                if (data.isValid()) {
                    Element* el = data.value<Element*>();
                    if (Component* comp = qobject_cast<Component*>(el)) {
                        // Check if our element is in this component's variants
                        const auto& variants = comp->variants();
                        for (Element* variant : variants) {
                            if (variant == element) {
                                return false; // Exclude from design mode
                            }
                        }
                    }
                }
            }
        }
        return true;
    } else if (m_viewMode == "script") {
        // In script mode, show only script elements (nodes and edges)
        return canvasElement->isScriptElement();
    } else if (m_viewMode == "variant") {
        // In variant mode, show only elements that belong to the editing component's variants
        if (!canvasElement->isDesignElement()) {
            return false;
        }
        
        if (Component* component = qobject_cast<Component*>(m_editingElement)) {
            // Check if this element is in the component's variants array or is a descendant of a variant
            const auto& variants = component->variants();
            for (Element* variant : variants) {
                if (variant == element) {
                    return true;
                }
                
                // Also show descendants of variants
                if (ElementModel* model = qobject_cast<ElementModel*>(sourceModel())) {
                    QList<Element*> descendants = model->getChildrenRecursive(variant->getId());
                    if (descendants.contains(element)) {
                        return true;
                    }
                }
            }
        }
        // If no component is being edited or element not in variants, don't show
        return false;
    }
    
    return false;
}