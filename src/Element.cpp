#include "Element.h"
#include "ElementModel.h"
#include <QDebug>

Element::Element(ElementType type, const QString &id, QObject *parent)
    : QObject(parent)
    , elementType(type)
    , elementId(id)
    , parentElementId()
    , selected(false)
{
}

QString Element::getTypeName() const
{
    switch (elementType) {
        case FrameType:
            return "Frame";
        case TextType:
            return "Text";
        case VariableType:
            return "Variable";
        case NodeType:
            return "Node";
        case EdgeType:
            return "Edge";
        case ComponentType:
            return "Component";
        case FrameComponentInstanceType:
            return "FrameComponentInstance";
        case FrameComponentVariantType:
            return "FrameComponentVariant";
        case TextVariantType:
            return "TextVariant";
        default:
            return "Unknown";
    }
}

void Element::setName(const QString &newName)
{
    if (name != newName) {
        name = newName;
        emit nameChanged();
        emit elementChanged();
    }
}

void Element::setParentElementId(const QString &parentId)
{
    if (parentElementId != parentId) {
        QString oldParentId = parentElementId;
        parentElementId = parentId;
        
        // If we're being parented (not unparented), reorder in the element list
        if (!parentId.isEmpty()) {
            // Try to get the ElementModel (which should be our parent if we're in one)
            ElementModel* model = qobject_cast<ElementModel*>(parent());
            if (model) {
                // Find the parent element
                Element* parentElement = model->getElementById(parentId);
                if (parentElement) {
                    // Find the parent's index
                    int parentIndex = -1;
                    for (int i = 0; i < model->rowCount(); i++) {
                        if (model->elementAt(i) == parentElement) {
                            parentIndex = i;
                            break;
                        }
                    }
                    
                    if (parentIndex >= 0) {
                        // Find the last child of this parent
                        int lastChildIndex = parentIndex;
                        for (int i = parentIndex + 1; i < model->rowCount(); i++) {
                            Element* elem = model->elementAt(i);
                            if (elem) {
                                // Check if this element is a descendant of the parent
                                Element* current = elem;
                                bool isDescendant = false;
                                while (current && !current->getParentElementId().isEmpty()) {
                                    if (current->getParentElementId() == parentId) {
                                        isDescendant = true;
                                        lastChildIndex = i;
                                        break;
                                    }
                                    // Move up the hierarchy
                                    current = model->getElementById(current->getParentElementId());
                                }
                                if (!isDescendant) {
                                    // We've reached an element that's not a descendant, so stop
                                    break;
                                }
                            }
                        }
                        
                        // Move this element to after the last child
                        model->reorderElement(this, lastChildIndex + 1);
                        qDebug() << "Element" << elementId << "moved below parent" << parentId 
                                 << "at index" << (lastChildIndex + 1);
                    }
                }
            }
        }
        
        emit parentIdChanged();
        emit elementChanged();
    }
}

void Element::setSelected(bool sel)
{
    if (selected != sel) {
        selected = sel;
        emit selectedChanged();
    }
}