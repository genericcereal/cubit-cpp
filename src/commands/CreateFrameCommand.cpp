#include "CreateFrameCommand.h"
#include "../Frame.h"
#include "../ElementModel.h"
#include "../SelectionManager.h"
#include <QDebug>

CreateFrameCommand::CreateFrameCommand(ElementModel* model, SelectionManager* selectionManager,
                                       const QRectF& rect, QObject *parent)
    : Command(parent)
    , m_elementModel(model)
    , m_selectionManager(selectionManager)
    , m_rect(rect)
    , m_frame(nullptr)
{
    setDescription(QString("Create Frame at (%1, %2)")
                   .arg(rect.x())
                   .arg(rect.y()));
}

CreateFrameCommand::~CreateFrameCommand()
{
    // If the frame exists but is not in the model (undone state), delete it
    if (m_frame && !m_elementModel->getAllElements().contains(m_frame)) {
        delete m_frame;
    }
}

void CreateFrameCommand::execute()
{
    if (!m_elementModel) return;

    if (!m_frame) {
        // First execution - create the frame
        m_frameId = m_elementModel->generateId();
        m_frame = new Frame(m_frameId);
        m_frame->setRect(m_rect);
    }

    // Add to model
    m_elementModel->addElement(m_frame);

    // Select the newly created frame
    if (m_selectionManager) {
        m_selectionManager->selectOnly(m_frame);
    }

}

void CreateFrameCommand::undo()
{
    if (!m_elementModel || !m_frame) return;

    // Clear selection if this frame is selected
    if (m_selectionManager && m_selectionManager->hasSelection() &&
        m_selectionManager->selectedElements().contains(m_frame)) {
        m_selectionManager->clearSelection();
    }

    // Remove from model (but don't delete - we might redo)
    m_elementModel->removeElement(m_frame->getId());

}