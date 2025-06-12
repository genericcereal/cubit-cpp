#include "ElementModel.h"
#include "Element.h"
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
        return element->x();
    case YRole:
        return element->y();
    case WidthRole:
        return element->width();
    case HeightRole:
        return element->height();
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
    for (Element *element : m_elements) {
        disconnectElement(element);
        element->deleteLater();
    }
    m_elements.clear();
    endResetModel();
}

QString ElementModel::generateId()
{
    return UniqueIdGenerator::generate16DigitId();
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
    disconnect(element, &Element::elementChanged, this, &ElementModel::onElementChanged);
    disconnect(element, &Element::selectedChanged, this, &ElementModel::onElementChanged);
}

int ElementModel::findElementIndex(const QString &elementId) const
{
    for (int i = 0; i < m_elements.size(); ++i) {
        if (m_elements[i]->getId() == elementId)
            return i;
    }
    return -1;
}