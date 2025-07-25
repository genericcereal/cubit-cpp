#include "LineModeHandler.h"
#include "ElementModel.h"
#include "SelectionManager.h"
#include "CommandHistory.h"
#include "Shape.h"
#include "CanvasController.h"
#include "Config.h"
#include "commands/CreateDesignElementCommand.h"
#include <QRectF>
#include <memory>
#include <QDebug>
#include <algorithm>

LineModeHandler::LineModeHandler(ElementModel* model,
                                 SelectionManager* selection,
                                 CommandHistory* history,
                                 std::function<void(CanvasController::Mode)> setMode)
    : m_elementModel(model)
    , m_selectionManager(selection)
    , m_commandHistory(history)
    , m_setModeFunc(setMode)
{
}

void LineModeHandler::onPress(qreal x, qreal y)
{
    // Handle line creation with clicks
    qDebug() << "LineModeHandler::onPress called at" << x << y;
    
    if (!m_commandHistory || !m_elementModel || !m_selectionManager) {
        qWarning() << "LineModeHandler::onPress - Missing dependencies";
        return;
    }
    
    QPointF clickPoint(x, y);
    
    if (!m_isCreatingLine) {
        // First click - start creating the line
        createInitialLine(clickPoint);
    } else {
        // Subsequent clicks - add joints to the existing line
        addJointToLine(clickPoint);
    }
}

void LineModeHandler::onMove(qreal x, qreal y)
{
    // For line mode, we don't do anything on move
    // Lines are created by discrete clicks, not dragging
    Q_UNUSED(x)
    Q_UNUSED(y)
}

void LineModeHandler::onRelease(qreal x, qreal y)
{
    // For line mode, we don't do anything on release
    // Lines are created by discrete clicks, not dragging
    Q_UNUSED(x)
    Q_UNUSED(y)
}

void LineModeHandler::onEscapePressed()
{
    if (m_isCreatingLine) {
        finishLineCreation();
    }
}

void LineModeHandler::createInitialLine(const QPointF& startPoint)
{
    // Clear any previous joints
    m_joints.clear();
    m_joints.append(startPoint);
    
    // Create initial line with minimal size at the start point
    QRectF rect(startPoint.x(), startPoint.y(), 1, 1);
    
    // Create the line element
    auto command = std::make_unique<CreateDesignElementCommand>(
        m_elementModel, m_selectionManager, 
        "shape", rect, QVariant::fromValue(static_cast<int>(Shape::Line)));
    
    // Store raw pointer before moving ownership
    m_currentCommand = command.get();
    m_commandHistory->execute(std::move(command));
    
    // Get the created shape
    auto selectedElements = m_selectionManager->selectedElements();
    if (!selectedElements.isEmpty()) {
        m_currentLine = qobject_cast<Shape*>(selectedElements.first());
        if (m_currentLine) {
            m_isCreatingLine = true;
            qDebug() << "LineModeHandler: Started line creation at" << startPoint;
        }
    }
}

void LineModeHandler::addJointToLine(const QPointF& newPoint)
{
    if (!m_currentLine || !m_isCreatingLine) {
        return;
    }
    
    // Add the new joint
    m_joints.append(newPoint);
    
    // Update the line's bounds to encompass all joints
    updateLineBounds();
    
    // Convert joints to normalized coordinates and update the shape
    QList<QPointF> normalizedJoints;
    QRectF bounds = m_currentLine->rect();
    
    for (const QPointF& joint : m_joints) {
        qreal normalizedX = bounds.width() > 0 ? (joint.x() - bounds.x()) / bounds.width() : 0.0;
        qreal normalizedY = bounds.height() > 0 ? (joint.y() - bounds.y()) / bounds.height() : 0.0;
        normalizedJoints.append(QPointF(normalizedX, normalizedY));
    }
    
    m_currentLine->setJoints(normalizedJoints);
    
}

void LineModeHandler::finishLineCreation()
{
    if (!m_isCreatingLine || !m_currentLine) {
        return;
    }
    
    
    // Ensure we have at least 2 points for a valid line
    if (m_joints.size() < 2) {
        // Add a second point slightly offset from the first
        if (m_joints.size() == 1) {
            QPointF secondPoint = m_joints.first() + QPointF(50, 50);
            m_joints.append(secondPoint);
            updateLineBounds();
            
            // Update normalized joints
            QList<QPointF> normalizedJoints;
            QRectF bounds = m_currentLine->rect();
            
            for (const QPointF& joint : m_joints) {
                qreal normalizedX = bounds.width() > 0 ? (joint.x() - bounds.x()) / bounds.width() : 0.0;
                qreal normalizedY = bounds.height() > 0 ? (joint.y() - bounds.y()) / bounds.height() : 0.0;
                normalizedJoints.append(QPointF(normalizedX, normalizedY));
            }
            
            m_currentLine->setJoints(normalizedJoints);
        }
    }
    
    // Notify the command that creation is complete
    if (m_currentCommand) {
        m_currentCommand->creationCompleted();
        m_currentCommand = nullptr;
    }
    
    // Reset state
    m_isCreatingLine = false;
    m_currentLine = nullptr;
    m_joints.clear();
    
    // Switch back to select mode
    if (m_setModeFunc) {
        m_setModeFunc(CanvasController::Mode::Select);
    }
}

void LineModeHandler::updateLineBounds()
{
    if (!m_currentLine || m_joints.isEmpty()) {
        return;
    }
    
    // Calculate bounding box of all joints
    qreal minX = m_joints.first().x();
    qreal maxX = m_joints.first().x();
    qreal minY = m_joints.first().y();
    qreal maxY = m_joints.first().y();
    
    for (const QPointF& joint : m_joints) {
        minX = qMin(minX, joint.x());
        maxX = qMax(maxX, joint.x());
        minY = qMin(minY, joint.y());
        maxY = qMax(maxY, joint.y());
    }
    
    // Ensure minimum size
    qreal width = qMax(maxX - minX, 1.0);
    qreal height = qMax(maxY - minY, 1.0);
    
    // Update the line element's bounds
    m_currentLine->setRect(QRectF(minX, minY, width, height));
}