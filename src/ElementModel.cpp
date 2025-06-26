#include "ElementModel.h"
#include "Element.h"
#include "CanvasElement.h"
#include "UniqueIdGenerator.h"

ElementModel::ElementModel(QObject *parent)
    : QAbstractListModel(parent)
{
}

ElementModel::~ElementModel()
{
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
    case XRole:
        if (element->isVisual()) {
            CanvasElement* canvasElement = qobject_cast<CanvasElement*>(element);
            return canvasElement ? canvasElement->x() : 0.0;
        }
        return 0.0;
    case YRole:
        if (element->isVisual()) {
            CanvasElement* canvasElement = qobject_cast<CanvasElement*>(element);
            return canvasElement ? canvasElement->y() : 0.0;
        }
        return 0.0;
    case WidthRole:
        if (element->isVisual()) {
            CanvasElement* canvasElement = qobject_cast<CanvasElement*>(element);
            return canvasElement ? canvasElement->width() : 0.0;
        }
        return 0.0;
    case HeightRole:
        if (element->isVisual()) {
            CanvasElement* canvasElement = qobject_cast<CanvasElement*>(element);
            return canvasElement ? canvasElement->height() : 0.0;
        }
        return 0.0;
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
    
    
    // Set the element's parent to this model so it can find the model later
    element->setParent(this);
    
    beginInsertRows(QModelIndex(), m_elements.size(), m_elements.size());
    m_elements.append(element);
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
    
    for (Element *element : elementsToDelete) {
        if (element) {
            disconnectElement(element);
            element->deleteLater();
        }
    }
    
    endResetModel();
}

QString ElementModel::generateId()
{
    return UniqueIdGenerator::generate16DigitId();
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