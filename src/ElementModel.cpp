#include "ElementModel.h"
#include "Element.h"
#include "CanvasElement.h"
#include "DesignElement.h"
#include "Frame.h"
#include "Text.h"
#include "Shape.h"
#include "platforms/web/WebTextInput.h"
#include "UniqueIdGenerator.h"
#include <QDebug>
#include <QPointer>

ElementModel::ElementModel(QObject *parent)
    : QAbstractListModel(parent)
{
}

ElementModel::~ElementModel()
{
    // Disconnect all source element connections before destruction
    for (const QString& sourceId : m_connectedSourceElements) {
        Element* sourceElement = getElementById(sourceId);
        if (sourceElement) {
            disconnect(sourceElement, nullptr, this, nullptr);
        }
    }
    m_connectedSourceElements.clear();
    
    clear();
}

int ElementModel::rowCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent)
    return m_elements.size();
}

QVariant ElementModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid() || index.row() >= m_elements.size())
        return QVariant();
    
    Element *element = m_elements[index.row()];
    
    switch (role) {
    case ElementRole:
        return QVariant::fromValue(element);
    case ElementIdRole:
        return element->getId();
    case ElementTypeRole:
        return element->getTypeName();
    case NameRole:
        return element->getName();
    case XRole: {
        CanvasElement* canvasElement = qobject_cast<CanvasElement*>(element);
        return canvasElement ? canvasElement->x() : 0.0;
    }
    case YRole: {
        CanvasElement* canvasElement = qobject_cast<CanvasElement*>(element);
        return canvasElement ? canvasElement->y() : 0.0;
    }
    case WidthRole: {
        CanvasElement* canvasElement = qobject_cast<CanvasElement*>(element);
        return canvasElement ? canvasElement->width() : 0.0;
    }
    case HeightRole: {
        CanvasElement* canvasElement = qobject_cast<CanvasElement*>(element);
        return canvasElement ? canvasElement->height() : 0.0;
    }
    case ParentIdRole:
        return element->getParentElementId();
    case SelectedRole:
        return element->isSelected();
    default:
        return QVariant();
    }
}

QHash<int, QByteArray> ElementModel::roleNames() const
{
    QHash<int, QByteArray> roles;
    roles[ElementRole] = "element";
    roles[ElementIdRole] = "elementId";
    roles[ElementTypeRole] = "elementType";
    roles[NameRole] = "name";
    roles[XRole] = "x";
    roles[YRole] = "y";
    roles[WidthRole] = "width";
    roles[HeightRole] = "height";
    roles[ParentIdRole] = "parentId";
    roles[SelectedRole] = "selected";
    return roles;
}

void ElementModel::addElement(Element *element)
{
    if (!element) return;
    
    // Check if element already exists to avoid duplicates
    if (m_elements.contains(element)) {
        qWarning() << "ElementModel::addElement - Element already in model:" << element->getId();
        return;
    }
    
    // Only set parent if element doesn't already have one
    // This is important for global elements that belong to another model
    if (!element->parent()) {
        element->setParent(this);
    }
    
    // If it's a Frame, set the ElementModel so it can connect to our signals
    if (Frame* frame = qobject_cast<Frame*>(element)) {
        frame->setElementModel(this);
    }
    
    // Find the correct insertion position based on parent
    int insertIndex = m_elements.size(); // Default to end
    
    if (!element->getParentElementId().isEmpty()) {
        // Find parent's position
        int parentIndex = -1;
        for (int i = 0; i < m_elements.size(); ++i) {
            if (m_elements[i]->getId() == element->getParentElementId()) {
                parentIndex = i;
                break;
            }
        }
        
        if (parentIndex >= 0) {
            // Find the last child of this parent (or last descendant)
            insertIndex = parentIndex + 1;
            
            // Skip over all descendants of the parent
            while (insertIndex < m_elements.size()) {
                Element* checkElem = m_elements[insertIndex];
                
                // Check if this element is a descendant of the parent
                bool isDescendant = false;
                QString currentParentId = checkElem->getParentElementId();
                
                while (!currentParentId.isEmpty()) {
                    if (currentParentId == m_elements[parentIndex]->getId()) {
                        isDescendant = true;
                        break;
                    }
                    
                    // Find the parent of currentParentId
                    bool found = false;
                    for (int j = 0; j < m_elements.size(); ++j) {
                        if (m_elements[j]->getId() == currentParentId) {
                            currentParentId = m_elements[j]->getParentElementId();
                            found = true;
                            break;
                        }
                    }
                    
                    if (!found) break;
                }
                
                if (isDescendant) {
                    insertIndex++;
                } else {
                    break;
                }
            }
        }
    }
    
    beginInsertRows(QModelIndex(), insertIndex, insertIndex);
    m_elements.insert(insertIndex, element);
    connectElement(element);
    endInsertRows();
    
    emit elementAdded(element);
    emit elementChanged();
}

void ElementModel::removeElement(const QString &elementId)
{
    int index = findElementIndex(elementId);
    if (index < 0) return;
    
    beginRemoveRows(QModelIndex(), index, index);
    Element *element = m_elements.takeAt(index);
    disconnectElement(element);
    endRemoveRows();
    
    emit elementRemoved(elementId);
    emit elementChanged();
    element->deleteLater();
}

void ElementModel::removeElementWithoutDelete(Element *element)
{
    if (!element) return;
    
    int index = m_elements.indexOf(element);
    if (index < 0) return;
    
    beginRemoveRows(QModelIndex(), index, index);
    m_elements.removeAt(index);
    disconnectElement(element);
    endRemoveRows();
    
    emit elementRemoved(element->getId());
    emit elementChanged();
    // Note: We do NOT call deleteLater() here - element is owned elsewhere
}

Element* ElementModel::getElementById(const QString &elementId) const
{
    for (Element *element : m_elements) {
        if (element->getId() == elementId)
            return element;
    }
    return nullptr;
}

Element* ElementModel::elementAt(int index) const
{
    if (index >= 0 && index < m_elements.size()) {
        return m_elements.at(index);
    }
    return nullptr;
}

void ElementModel::clear()
{
    if (m_elements.isEmpty()) return;
    
    beginResetModel();
    
    // Make a copy to avoid issues if elements are deleted during iteration
    QList<Element*> elementsToDelete = m_elements;
    m_elements.clear();
    
    // End the model reset before deleting elements
    // This ensures QML views are notified that the model is empty
    // before we start destroying the actual objects
    endResetModel();
    
    // Now delete the elements after QML has been notified
    for (Element *element : elementsToDelete) {
        if (element) {
            disconnectElement(element);
            element->deleteLater();
        }
    }
}

QString ElementModel::generateId()
{
    return UniqueIdGenerator::generate16DigitId();
}

void ElementModel::resolveParentRelationships() {
    // Go through all elements and resolve their parent pointers
    for (Element* element : m_elements) {
        if (!element) continue;
        
        QString parentId = element->getParentElementId();
        if (!parentId.isEmpty()) {
            // Try to resolve the parent again
            if (CanvasElement* canvasElement = qobject_cast<CanvasElement*>(element)) {
                // Force re-resolution by setting the parent ID again
                canvasElement->setParentElementId(parentId);
            }
        }
    }
}

void ElementModel::removeElement(Element *element)
{
    if (!element) return;
    removeElement(element->getId());
}

QList<Element*> ElementModel::getChildrenRecursive(const QString &parentId) const
{
    QList<Element*> children;
    
    // Find all direct children
    for (Element *element : m_elements) {
        if (element && element->getParentElementId() == parentId) {
            children.append(element);
            
            // Recursively get children of this child
            QList<Element*> grandChildren = getChildrenRecursive(element->getId());
            children.append(grandChildren);
        }
    }
    
    return children;
}

void ElementModel::onElementChanged()
{
    Element *element = qobject_cast<Element*>(sender());
    if (!element) return;
    
    int index = m_elements.indexOf(element);
    if (index >= 0) {
        QModelIndex modelIndex = createIndex(index, 0);
        emit dataChanged(modelIndex, modelIndex);
        emit elementUpdated(element);
        emit elementChanged();
    }
}

void ElementModel::connectElement(Element *element)
{
    connect(element, &Element::elementChanged, this, &ElementModel::onElementChanged);
    connect(element, &Element::selectedChanged, this, &ElementModel::onElementChanged);
    
    // Check if this element is an instance (has a sourceId)
    if (DesignElement* designElement = qobject_cast<DesignElement*>(element)) {
        QString sourceId = designElement->instanceOf();
        if (!sourceId.isEmpty()) {
            // Find the source element and connect to its property changes
            Element* sourceElement = getElementById(sourceId);
            if (sourceElement) {
                connectSourceElement(sourceElement);
            }
        }
    }
}

void ElementModel::disconnectElement(Element *element)
{
    if (!element) return;
    
    // Use QObject::disconnect which is safer during destruction
    QObject::disconnect(element, &Element::elementChanged, this, &ElementModel::onElementChanged);
    QObject::disconnect(element, &Element::selectedChanged, this, &ElementModel::onElementChanged);
}

int ElementModel::findElementIndex(const QString &elementId) const
{
    for (int i = 0; i < m_elements.size(); ++i) {
        if (m_elements[i]->getId() == elementId)
            return i;
    }
    return -1;
}

void ElementModel::reorderElement(Element *element, int newIndex)
{
    if (!element) return;
    
    int currentIndex = m_elements.indexOf(element);
    if (currentIndex < 0 || currentIndex == newIndex) return;
    
    // Clamp newIndex to valid range
    newIndex = qBound(0, newIndex, m_elements.size() - 1);
    
    // Perform the reorder
    beginResetModel();
    m_elements.move(currentIndex, newIndex);
    endResetModel();
    
    emit elementChanged();
    
}

void ElementModel::refresh()
{
    // Force the model to refresh all views
    beginResetModel();
    endResetModel();
    emit elementChanged();
}

void ElementModel::connectSourceElement(Element* sourceElement)
{
    if (!sourceElement) return;
    
    QString sourceId = sourceElement->getId();
    
    // Check if already connected
    if (m_connectedSourceElements.contains(sourceId)) {
        return;
    }
    
    // Mark as connected
    m_connectedSourceElements.insert(sourceId);
    
    // Use QPointer for safety
    QPointer<Element> safeSource = sourceElement;
    QPointer<ElementModel> safeModel = this;
    
    // Connect to property changes based on element type
    // For CanvasElement properties (width, height only - position changes handled by move command)
    if (CanvasElement* canvasElement = qobject_cast<CanvasElement*>(sourceElement)) {
        connect(canvasElement, &CanvasElement::widthChanged,
                this, [safeSource, safeModel]() {
                    if (safeSource && safeModel) {
                        safeModel->syncInstancesFromSource(safeSource);
                    }
                });
                
        connect(canvasElement, &CanvasElement::heightChanged,
                this, [safeSource, safeModel]() {
                    if (safeSource && safeModel) {
                        safeModel->syncInstancesFromSource(safeSource);
                    }
                });
    }
    
    // For DesignElement anchor properties
    if (DesignElement* designElement = qobject_cast<DesignElement*>(sourceElement)) {
        // Only connect to anchor state changes (enabled/disabled)
        connect(designElement, &DesignElement::leftAnchoredChanged,
                this, [safeSource, safeModel]() {
                    if (safeSource && safeModel) {
                        safeModel->syncInstancesFromSource(safeSource);
                    }
                });
                
        connect(designElement, &DesignElement::rightAnchoredChanged,
                this, [safeSource, safeModel]() {
                    if (safeSource && safeModel) {
                        safeModel->syncInstancesFromSource(safeSource);
                    }
                });
                
        connect(designElement, &DesignElement::topAnchoredChanged,
                this, [safeSource, safeModel]() {
                    if (safeSource && safeModel) {
                        safeModel->syncInstancesFromSource(safeSource);
                    }
                });
                
        connect(designElement, &DesignElement::bottomAnchoredChanged,
                this, [safeSource, safeModel]() {
                    if (safeSource && safeModel) {
                        safeModel->syncInstancesFromSource(safeSource);
                    }
                });
    }
    
    // For Frame-specific properties
    if (Frame* frame = qobject_cast<Frame*>(sourceElement)) {
        connect(frame, &Frame::fillChanged,
                this, [safeSource, safeModel]() {
                    if (safeSource && safeModel) {
                        safeModel->syncInstancesFromSource(safeSource);
                    }
                });
                
        connect(frame, &Frame::borderColorChanged,
                this, [safeSource, safeModel]() {
                    if (safeSource && safeModel) {
                        safeModel->syncInstancesFromSource(safeSource);
                    }
                });
                
        connect(frame, &Frame::borderWidthChanged,
                this, [safeSource, safeModel]() {
                    if (safeSource && safeModel) {
                        safeModel->syncInstancesFromSource(safeSource);
                    }
                });
                
        connect(frame, &Frame::borderRadiusChanged,
                this, [safeSource, safeModel]() {
                    if (safeSource && safeModel) {
                        safeModel->syncInstancesFromSource(safeSource);
                    }
                });
    }
    
    // For Text-specific properties
    if (Text* text = qobject_cast<Text*>(sourceElement)) {
        connect(text, &Text::contentChanged,
                this, [safeSource, safeModel]() {
                    if (safeSource && safeModel) {
                        safeModel->syncInstancesFromSource(safeSource);
                    }
                });
                
        connect(text, &Text::fontChanged,
                this, [safeSource, safeModel]() {
                    if (safeSource && safeModel) {
                        safeModel->syncInstancesFromSource(safeSource);
                    }
                });
                
        connect(text, &Text::colorChanged,
                this, [safeSource, safeModel]() {
                    if (safeSource && safeModel) {
                        safeModel->syncInstancesFromSource(safeSource);
                    }
                });
    }
}

void ElementModel::disconnectSourceElement(Element* sourceElement)
{
    if (!sourceElement) return;
    
    QString sourceId = sourceElement->getId();
    m_connectedSourceElements.remove(sourceId);
    
    // Disconnect all connections from this source element
    disconnect(sourceElement, nullptr, this, nullptr);
}

void ElementModel::syncInstancesFromSource(Element* sourceElement)
{
    if (!sourceElement) return;
    
    QString sourceId = sourceElement->getId();
    
    // Find all instances with this source ID
    for (Element* element : m_elements) {
        if (DesignElement* designElement = qobject_cast<DesignElement*>(element)) {
            if (designElement->instanceOf() == sourceId) {
                // Update properties from source
                QStringList propNames = sourceElement->propertyNames();
                
                for (const QString& propName : propNames) {
                    // Skip properties that shouldn't be synced
                    if (propName != "elementId" && propName != "parentId" && 
                        propName != "selected" && propName != "x" && propName != "y") {
                        
                        QVariant value = sourceElement->getProperty(propName);
                        element->setProperty(propName, value);
                    }
                }
                
                // Update size properties if applicable
                if (CanvasElement* sourceCanvas = qobject_cast<CanvasElement*>(sourceElement)) {
                    if (CanvasElement* instanceCanvas = qobject_cast<CanvasElement*>(element)) {
                        instanceCanvas->setWidth(sourceCanvas->width());
                        instanceCanvas->setHeight(sourceCanvas->height());
                    }
                }
            }
        }
    }
}

void ElementModel::onSourceElementPropertyChanged()
{
    // This slot is called when a source element's property changes
    Element* sourceElement = qobject_cast<Element*>(sender());
    if (sourceElement) {
        syncInstancesFromSource(sourceElement);
    }
}