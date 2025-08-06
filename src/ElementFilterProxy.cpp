#include "ElementFilterProxy.h"
#include "Element.h"
#include "DesignElement.h"
#include "ScriptElement.h"
#include "Variable.h"
#include "Component.h"
#include "ElementModel.h"
#include "PlatformConfig.h"
#include "Project.h"
#include <QDebug>

ElementFilterProxy::ElementFilterProxy(QObject *parent)
    : QSortFilterProxyModel(parent)
    , m_viewMode("design")
{
}

void ElementFilterProxy::refreshFilter()
{
    invalidateFilter();
    emit filterChanged();
}

void ElementFilterProxy::setSourceModel(QAbstractItemModel *sourceModel)
{
    // Disconnect from previous model
    if (this->sourceModel()) {
        if (ElementModel* elementModel = qobject_cast<ElementModel*>(this->sourceModel())) {
            // Disconnect from all components
            QList<Element*> allElements = elementModel->getAllElements();
            for (Element* el : allElements) {
                if (ComponentElement* comp = qobject_cast<ComponentElement*>(el)) {
                    disconnect(comp, &ComponentElement::elementsChanged,
                              this, &ElementFilterProxy::refreshFilter);
                }
            }
            
            // Disconnect from model changes
            disconnect(elementModel, &ElementModel::elementAdded,
                      this, &ElementFilterProxy::refreshFilter);
            disconnect(elementModel, &ElementModel::elementRemoved,
                      this, &ElementFilterProxy::refreshFilter);
        }
    }
    
    // Set the new source model
    QSortFilterProxyModel::setSourceModel(sourceModel);
    
    // Connect to new model
    if (sourceModel) {
        if (ElementModel* elementModel = qobject_cast<ElementModel*>(sourceModel)) {
            // Connect to all components
            QList<Element*> allElements = elementModel->getAllElements();
            for (Element* el : allElements) {
                if (ComponentElement* comp = qobject_cast<ComponentElement*>(el)) {
                    connect(comp, &ComponentElement::elementsChanged,
                           this, &ElementFilterProxy::refreshFilter);
                }
            }
            
            // Connect to model changes to handle new components
            connect(elementModel, &ElementModel::elementAdded,
                   this, [this](Element* element) {
                       // If a new component is added, connect to its elementsChanged signal
                       if (ComponentElement* comp = qobject_cast<ComponentElement*>(element)) {
                           connect(comp, &ComponentElement::elementsChanged,
                                  this, &ElementFilterProxy::refreshFilter);
                       }
                       refreshFilter();
                   });
            connect(elementModel, &ElementModel::elementRemoved,
                   this, &ElementFilterProxy::refreshFilter);
        }
    }
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

bool ElementFilterProxy::shouldShowElement(Element* element) const {
    return shouldShowElementInMode(element);
}

bool ElementFilterProxy::filterAcceptsRow(int sourceRow, const QModelIndex &sourceParent) const {
    Q_UNUSED(sourceParent)
    
    if (!sourceModel()) {
        return false;
    }
    
    QModelIndex index = sourceModel()->index(sourceRow, 0);
    if (!index.isValid()) {
        return false;
    }
    
    // Get the element from the model data
    QVariant data = sourceModel()->data(index, Qt::UserRole + 1); // ElementRole
    if (!data.isValid()) {
        return false;
    }
    
    Element* element = data.value<Element*>();
    if (!element) {
        return false;
    }
    
    bool accepted = shouldShowElementInMode(element);
    //          << "filterComponentsOut:" << m_filterComponentsOut 
    //          << "filterComponentsOnly:" << m_filterComponentsOnly
    //          << "accepted:" << accepted;
    return accepted;
}

bool ElementFilterProxy::shouldShowElementInMode(Element* element) const {
    if (!element) {
        return false;
    }
    
    // If we're editing a component, only show elements that belong to that component
    if (m_editingElement) {
        ComponentElement* component = qobject_cast<ComponentElement*>(m_editingElement);
        if (component) {
            // Only show elements that are in the component's elements list
            bool inComponent = component->elements().contains(element);
            if (!inComponent && element == component) {
                // Don't filter out the component itself when editing it
                return false;
            }
            return inComponent;
        }
    }
    
    // Check if this element belongs to any component (and we're not editing that component)
    if (ElementModel* model = qobject_cast<ElementModel*>(sourceModel())) {
        QList<Element*> allElements = model->getAllElements();
        for (Element* el : allElements) {
            if (ComponentElement* comp = qobject_cast<ComponentElement*>(el)) {
                if (comp->elements().contains(element)) {
                    // This element belongs to a component, don't show it in main view
                    return false;
                }
            }
        }
    }
    
    // Special handling for ComponentElement - check by element type
    if (element->getTypeName() == "ComponentElement") {
        // Apply component filtering
        if (m_filterComponentsOut) {
            return false; // Filter out components
        }
        if (m_filterComponentsOnly) {
            return m_viewMode == "design"; // Only show components in design mode
        }
        // Components are only shown in design mode, not in script modes
        return m_viewMode == "design";
    }
    
    // If we're only showing components, filter out non-components
    if (m_filterComponentsOnly) {
        return false;
    }
    
    // Check if element should be shown in element list
    if (!element->showInElementList()) {
        return false;
    }
    
    // Check if element is frozen (should not appear in element list)
    DesignElement* designElement = qobject_cast<DesignElement*>(element);
    if (designElement && designElement->isFrozen()) {
        return false;
    }
    
    // Handle Variables specially (they are non-visual)
    if (Variable* var = qobject_cast<Variable*>(element)) {
        // Element variables (scope="element") only show in script mode
        if (var->variableScope() == "element") {
            return m_viewMode == "script";
        }
        // Global variables (scope="global") show in both design and script modes
        return m_viewMode == "design" || m_viewMode == "script";
    }
    
    // Non-visual elements (excluding Variables which we handled above) are never shown
    CanvasElement* canvasElement = qobject_cast<CanvasElement*>(element);
    if (!canvasElement) {
        return false;
    }
    
    // Script elements (Nodes and Edges) should NEVER appear in ElementList
    // They are only viewable on the script canvas itself
    if (canvasElement->isScriptElement()) {
        return false;
    }
    
    if (m_viewMode == "design") {
        // In design mode, show:
        // 1. Design elements (Frame, Text)
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
        
        // Check if it's an instance
        if (designElement->isInstance()) {
            return true;
        }
        
        // Component variants no longer exist, so no check needed
        
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
                
                // Component variants no longer exist, so no parent check needed
                
                current = parent;
                depth++;
            }
        }
        
        // Component variants no longer exist, so no variants array check needed
        
        // Platform global elements no longer used
        
        return true;
    } else if (m_viewMode == "script") {
        // In script mode, ElementList shows nothing since script elements were already filtered out
        // Script elements (Nodes and Edges) are only viewable on the script canvas itself
        return false;
    }
    
    return false;
}