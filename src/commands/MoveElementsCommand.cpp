#include "MoveElementsCommand.h"
#include "../Element.h"
#include "../CanvasElement.h"
#include <QDebug>

MoveElementsCommand::MoveElementsCommand(const QList<Element*>& elements, const QPointF& delta, QObject *parent)
    : Command(parent)
    , m_totalDelta(delta)
    , m_firstExecute(true)
{
    // Store original positions for all visual elements
    for (Element* element : elements) {
        if (element && element->isVisual()) {
            CanvasElement* canvasElement = qobject_cast<CanvasElement*>(element);
            if (canvasElement) {
                ElementMove move;
                move.element = canvasElement;
                move.originalPosition = QPointF(canvasElement->x(), canvasElement->y());
                move.newPosition = move.originalPosition + delta;
                m_moves.append(move);
            }
        }
    }

    setDescription(QString("Move %1 element%2")
                   .arg(m_moves.size())
                   .arg(m_moves.size() == 1 ? "" : "s"));
}

MoveElementsCommand::~MoveElementsCommand()
{
}

void MoveElementsCommand::execute()
{
    if (m_firstExecute) {
        // First execution - elements are already at new position from drag operation
        // Just mark as executed
        m_firstExecute = false;
    } else {
        // Redo - move elements to new position
        for (const ElementMove& move : m_moves) {
            move.element->setX(move.newPosition.x());
            move.element->setY(move.newPosition.y());
        }
    }

}

void MoveElementsCommand::undo()
{
    // Move elements back to original position
    for (const ElementMove& move : m_moves) {
        move.element->setX(move.originalPosition.x());
        move.element->setY(move.originalPosition.y());
    }

}

bool MoveElementsCommand::mergeWith(MoveElementsCommand* other)
{
    if (!other) return false;

    // Can only merge if we're moving the same elements
    if (m_moves.size() != other->m_moves.size()) return false;

    for (int i = 0; i < m_moves.size(); ++i) {
        if (m_moves[i].element != other->m_moves[i].element) {
            return false;
        }
    }

    // Merge the deltas
    m_totalDelta += other->m_totalDelta;

    // Update new positions
    for (int i = 0; i < m_moves.size(); ++i) {
        m_moves[i].newPosition = m_moves[i].originalPosition + m_totalDelta;
    }

    // Update description
    setDescription(QString("Move %1 element%2")
                   .arg(m_moves.size())
                   .arg(m_moves.size() == 1 ? "" : "s"));

    return true;
}