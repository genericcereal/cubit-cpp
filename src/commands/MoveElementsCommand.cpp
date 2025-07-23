#include "MoveElementsCommand.h"
#include "../Element.h"
#include "../CanvasElement.h"
#include "../Application.h"
#include "../Project.h"
#include "../ProjectApiClient.h"
#include "../ElementModel.h"
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
    
    // Log to console
    qDebug() << QString("Moved %1 element%2 by delta (%3, %4)")
                .arg(m_moves.size())
                .arg(m_moves.size() == 1 ? "" : "s")
                .arg(m_totalDelta.x())
                .arg(m_totalDelta.y());
    
    // Sync with API after the move
    syncWithAPI();
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

void MoveElementsCommand::syncWithAPI()
{
    if (m_moves.isEmpty()) {
        return;
    }

    // Get project from any element's model
    CanvasElement* firstElement = m_moves.first().element;
    if (!firstElement) {
        return;
    }

    // Navigate up to find the project (through ElementModel)
    ElementModel* model = nullptr;
    QObject* parent = firstElement->parent();
    while (parent) {
        model = qobject_cast<ElementModel*>(parent);
        if (model) break;
        parent = parent->parent();
    }

    if (!model) {
        return;
    }

    Project* project = qobject_cast<Project*>(model->parent());
    if (!project) {
        return;
    }

    // Get the Application instance and its API client
    Application* app = Application::instance();
    if (!app) {
        return;
    }

    ProjectApiClient* apiClient = app->projectApiClient();
    if (!apiClient) {
        return;
    }

    // Get the project's API ID
    QString apiProjectId = project->id();
    
    // Create array of element IDs
    QJsonArray elementIds;
    for (const ElementMove& move : m_moves) {
        elementIds.append(move.element->getId());
    }
    
    // Create delta data
    QJsonObject deltaData;
    deltaData["dx"] = m_totalDelta.x();
    deltaData["dy"] = m_totalDelta.y();
    
    // Sync with API
    apiClient->syncMoveElements(apiProjectId, elementIds, deltaData);
    
    qDebug() << "MoveElementsCommand: Syncing element move with API for project" << apiProjectId 
             << "delta:" << m_totalDelta << "elements:" << elementIds.size();
}