#include "PenModeHandler.h"
#include "ElementModel.h"
#include "SelectionManager.h"
#include "CommandHistory.h"
#include "Shape.h"
#include "CanvasController.h"
#include "Config.h"
#include "commands/CreateDesignElementCommand.h"
#include "ShapeControlsController.h"
#include <QRectF>
#include <memory>
#include <QDebug>
#include <algorithm>

PenModeHandler::PenModeHandler(ElementModel* model,
                                 SelectionManager* selection,
                                 CommandHistory* history,
                                 std::function<void(CanvasController::Mode)> setMode,
                                 ShapeControlsController* shapeControlsController)
    : m_elementModel(model)
    , m_selectionManager(selection)
    , m_commandHistory(history)
    , m_setModeFunc(setMode)
    , m_shapeControlsController(shapeControlsController)
{
}

void PenModeHandler::onPress(qreal x, qreal y)
{
    // Handle pen creation with clicks
    
    if (!m_commandHistory || !m_elementModel || !m_selectionManager) {
        qWarning() << "PenModeHandler::onPress - Missing dependencies";
        return;
    }
    
    QPointF clickPoint(x, y);
    
    if (!m_isCreatingPen) {
        // Check if there's a selected shape that we should add to
        auto selectedElements = m_selectionManager->selectedElements();
        if (selectedElements.size() == 1) {
            Shape* selectedShape = qobject_cast<Shape*>(selectedElements.first());
            if (selectedShape) {
                // We're adding a new path to an existing shape
                m_currentPen = selectedShape;
                m_isCreatingPen = true;
                
                // Get existing joints from the shape (denormalized)
                m_joints.clear();
                QRectF bounds = selectedShape->rect();
                auto joints = selectedShape->joints();
                for (const QVariant& jointVar : joints) {
                    QVariantMap jointMap = jointVar.toMap();
                    Shape::Joint joint = Shape::Joint::fromVariantMap(jointMap);
                    // Convert from normalized to absolute coordinates
                    joint.position = QPointF(bounds.x() + joint.position.x() * bounds.width(), 
                                           bounds.y() + joint.position.y() * bounds.height());
                    m_joints.append(joint);
                }
                
                // Check if there's a selected joint to connect to
                if (m_shapeControlsController && m_shapeControlsController->selectedJointIndex() >= 0) {
                    int selectedJointIndex = m_shapeControlsController->selectedJointIndex();
                    if (selectedJointIndex < m_joints.size()) {
                        // Connect from the selected joint
                        Shape::Joint connectJoint = m_joints[selectedJointIndex];
                        connectJoint.isPathStart = false;  // It's a continuation from the selected joint
                        m_joints.append(connectJoint);
                        
                        // Now add the clicked point as the next joint
                        addJointToPen(clickPoint);
                        
                        // Clear the joint selection after connecting
                        m_shapeControlsController->setSelectedJointIndex(-1);
                        return;
                    }
                }
                
                // No selected joint - mark that the next joint will start a new path
                m_startNewPath = true;
                // Add the first joint of the new path
                addJointToPen(clickPoint);
                return;
            }
        }
        
        // No selected shape or not a shape - create new pen
        createInitialPen(clickPoint);
    } else {
        // Subsequent clicks - check if we're clicking on a hovered joint
        if (m_shapeControlsController && m_shapeControlsController->hoveredJointIndex() >= 0) {
            int hoveredIndex = m_shapeControlsController->hoveredJointIndex();
            if (hoveredIndex < m_joints.size()) {
                // Add a joint at the position of the hovered joint to create an edge to it
                // QPointF hoveredJointPos = m_joints[hoveredIndex].position;
                // TODO: Implement edge creation to hovered joint
                
                // Add debug to see all current joint positions
                for (int i = 0; i < m_joints.size(); ++i) {
                }
                
                // Check if we're clicking on the most recent joint (trying to create a zero-length edge)
                if (m_joints.size() > 0 && hoveredIndex == m_joints.size() - 1) {
                    return;
                }
                
                // Instead of adding a duplicate joint, add an edge from the last joint to the hovered joint
                int lastJointIndex = m_joints.size() - 1;
                
                m_currentPen->addEdge(lastJointIndex, hoveredIndex);
                
                // Debug: Check current edge count
                
                // Set the hovered joint as the new active joint
                if (m_shapeControlsController) {
                    m_shapeControlsController->setSelectedJointIndex(hoveredIndex);
                    
                    // Keep the preview line active from the new joint position
                    // The preview will automatically update to show from the newly selected joint
                }
                return;
            }
        }
        
        // Normal case - add new joint at click position
        addJointToPen(clickPoint);
    }
}

void PenModeHandler::onMove(qreal x, qreal y)
{
    m_currentMousePos = QPointF(x, y);
    // Mouse position is now handled by QML for preview drawing
}

void PenModeHandler::onRelease(qreal x, qreal y)
{
    // For pen mode, we don't do anything on release
    // Pens are created by discrete clicks, not dragging
    Q_UNUSED(x)
    Q_UNUSED(y)
}

void PenModeHandler::onEnterPressed()
{
    if (m_isCreatingPen) {
        finishPenCreation();
    }
}

void PenModeHandler::createInitialPen(const QPointF& startPoint)
{
    // Clear any previous joints
    m_joints.clear();
    Shape::Joint firstJoint;
    firstJoint.position = startPoint;
    firstJoint.isPathStart = true; // First joint of a new shape is always a path start
    m_joints.append(firstJoint);
    
    // Create initial pen with minimal size at the start point
    QRectF rect(startPoint.x(), startPoint.y(), 1, 1);
    
    // Create the pen element (no parent for pen shapes)
    auto command = std::make_unique<CreateDesignElementCommand>(
        m_elementModel, m_selectionManager, 
        "shape", rect, QVariant::fromValue(static_cast<int>(Shape::Pen)), QString());
    
    // Store raw pointer before moving ownership
    m_currentCommand = command.get();
    m_commandHistory->execute(std::move(command));
    
    // Get the created shape
    auto selectedElements = m_selectionManager->selectedElements();
    if (!selectedElements.isEmpty()) {
        m_currentPen = qobject_cast<Shape*>(selectedElements.first());
        if (m_currentPen) {
            m_isCreatingPen = true;
            
            // Set the first joint as selected
            if (m_shapeControlsController) {
                m_shapeControlsController->setSelectedJointIndex(0);
            }
        }
    }
}

void PenModeHandler::addJointToPen(const QPointF& newPoint)
{
    if (!m_currentPen || !m_isCreatingPen) {
        return;
    }
    
    // Add the new joint
    Shape::Joint newJoint;
    newJoint.position = newPoint;
    newJoint.isPathStart = m_startNewPath;
    m_joints.append(newJoint);
    
    // Reset the flag after using it
    m_startNewPath = false;
    
    // Update the pen's bounds to encompass all joints
    updatePenBounds();
    
    // Convert joints to normalized coordinates and update the shape
    QList<Shape::Joint> normalizedJoints;
    QRectF bounds = m_currentPen->rect();
    
    for (const Shape::Joint& joint : m_joints) {
        Shape::Joint normalizedJoint = joint;
        normalizedJoint.position.setX(bounds.width() > 0 ? (joint.position.x() - bounds.x()) / bounds.width() : 0.0);
        normalizedJoint.position.setY(bounds.height() > 0 ? (joint.position.y() - bounds.y()) / bounds.height() : 0.0);
        normalizedJoints.append(normalizedJoint);
    }
    
    m_currentPen->setJoints(normalizedJoints);
    
    // For pen shapes, also create consecutive edges
    if (m_currentPen->shapeType() == Shape::Pen && m_joints.size() >= 2) {
        // Add edge from the currently selected joint (or previous joint if none selected) to the new joint
        int selectedJointIndex = m_shapeControlsController ? m_shapeControlsController->selectedJointIndex() : -1;
        int fromIndex = (selectedJointIndex >= 0 && selectedJointIndex < m_joints.size() - 1) 
            ? selectedJointIndex 
            : m_joints.size() - 2;
        int toIndex = m_joints.size() - 1;
        m_currentPen->addEdge(fromIndex, toIndex);
    }
    
    for (int i = 0; i < normalizedJoints.size(); ++i) {
    }
    
    // Set the last joint as selected in the shape controls controller
    if (m_shapeControlsController) {
        int newSelectedIndex = m_joints.size() - 1;
        m_shapeControlsController->setSelectedJointIndex(newSelectedIndex);
    }
}

void PenModeHandler::finishPenCreation()
{
    if (!m_isCreatingPen || !m_currentPen) {
        return;
    }
    
    
    // Ensure we have at least 2 points for a valid pen
    if (m_joints.size() < 2) {
        // Add a second point slightly offset from the first
        if (m_joints.size() == 1) {
            Shape::Joint secondJoint;
            secondJoint.position = m_joints.first().position + QPointF(50, 50);
            secondJoint.isPathStart = false; // It's a continuation of the first point
            m_joints.append(secondJoint);
            updatePenBounds();
            
            // Update normalized joints
            QList<Shape::Joint> normalizedJoints;
            QRectF bounds = m_currentPen->rect();
            
            for (const Shape::Joint& joint : m_joints) {
                Shape::Joint normalizedJoint = joint;
                normalizedJoint.position.setX(bounds.width() > 0 ? (joint.position.x() - bounds.x()) / bounds.width() : 0.0);
                normalizedJoint.position.setY(bounds.height() > 0 ? (joint.position.y() - bounds.y()) / bounds.height() : 0.0);
                normalizedJoints.append(normalizedJoint);
            }
            
            m_currentPen->setJoints(normalizedJoints);
        }
    }
    
    // Notify the command that creation is complete (only if we created a new shape)
    if (m_currentCommand) {
        m_currentCommand->creationCompleted();
        m_currentCommand = nullptr;
    }
    
    // Reset state
    m_isCreatingPen = false;
    m_currentPen = nullptr;
    m_joints.clear();
    
    // Clear selected joint
    if (m_shapeControlsController) {
        m_shapeControlsController->setSelectedJointIndex(-1);
    }
    
    // Switch back to select mode
    if (m_setModeFunc) {
        m_setModeFunc(CanvasController::Mode::Select);
    }
}

void PenModeHandler::updatePenBounds()
{
    if (!m_currentPen || m_joints.isEmpty()) {
        return;
    }
    
    // Calculate bounding box of all joints
    qreal minX = m_joints.first().position.x();
    qreal maxX = m_joints.first().position.x();
    qreal minY = m_joints.first().position.y();
    qreal maxY = m_joints.first().position.y();
    
    for (const Shape::Joint& joint : m_joints) {
        minX = qMin(minX, joint.position.x());
        maxX = qMax(maxX, joint.position.x());
        minY = qMin(minY, joint.position.y());
        maxY = qMax(maxY, joint.position.y());
    }
    
    // Ensure minimum size
    qreal width = qMax(maxX - minX, 1.0);
    qreal height = qMax(maxY - minY, 1.0);
    
    // Update the pen element's bounds
    m_currentPen->setRect(QRectF(minX, minY, width, height));
}