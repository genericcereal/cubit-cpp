#include "ResizeElementCommand.h"
#include "../CanvasElement.h"
#include <QDebug>

ResizeElementCommand::ResizeElementCommand(CanvasElement* element, const QRectF& oldRect, const QRectF& newRect, QObject *parent)
    : Command(parent)
    , m_element(element)
    , m_oldRect(oldRect)
    , m_newRect(newRect)
    , m_firstExecute(true)
{
    setDescription(QString("Resize element to %1x%2")
                   .arg(newRect.width())
                   .arg(newRect.height()));
}

ResizeElementCommand::~ResizeElementCommand()
{
}

void ResizeElementCommand::execute()
{
    if (!m_element) return;

    if (m_firstExecute) {
        // First execution - element is already at new size from resize operation
        // Just mark as executed
        m_firstExecute = false;
    } else {
        // Redo - resize element to new rect
        m_element->setRect(m_newRect);
    }

}

void ResizeElementCommand::undo()
{
    if (!m_element) return;

    // Resize element back to original rect
    m_element->setRect(m_oldRect);

}

bool ResizeElementCommand::mergeWith(ResizeElementCommand* other)
{
    if (!other) return false;

    // Can only merge if we're resizing the same element
    if (m_element != other->m_element) return false;

    // Update the new rect to the latest resize
    m_newRect = other->m_newRect;

    // Update description
    setDescription(QString("Resize element to %1x%2")
                   .arg(m_newRect.width())
                   .arg(m_newRect.height()));

    return true;
}