#include "AICommandDispatcher.h"
#include "Application.h"
#include "CanvasController.h"
#include "HitTestService.h"
#include "ElementModel.h"
#include "SelectionManager.h"
#include "Frame.h"
#include "Text.h"
#include "Node.h"
#include "Edge.h"
#include "commands/CreateDesignElementCommand.h"
#include "commands/DeleteElementsCommand.h"
#include "commands/MoveElementsCommand.h"
#include "commands/ResizeElementCommand.h"
#include "commands/SetPropertyCommand.h"
#include "Project.h"
#include "UniqueIdGenerator.h"
#include <QJsonDocument>
#include <QJsonValue>
#include <QDebug>
#include <QUuid>
#include <QCoreApplication>
#include <QColor>

AICommandDispatcher::AICommandDispatcher(Application *app, QObject *parent)
    : QObject(parent), m_application(app)
{
}

void AICommandDispatcher::setTargetProject(Project* project)
{
    m_targetProject = project;
}

void AICommandDispatcher::executeCommand(const QJsonObject &command)
{
    if (!validateCommand(command))
    {
        emit commandError("Invalid command structure");
        return;
    }

    QString type = command["type"].toString();
    QString description = command.contains("description") ? command["description"].toString() : QString("Executing %1 command").arg(type);
    QJsonObject params = command.contains("params") ? command["params"].toObject() : QJsonObject();

    try
    {
        if (type == "createElement")
        {
            executeCreateElement(command);
        }
        else if (type == "deleteElement")
        {
            executeDeleteElement(command);
        }
        else if (type == "moveElement")
        {
            executeMoveElement(command);
        }
        else if (type == "resizeElement")
        {
            executeResizeElement(command);
        }
        else if (type == "setProperty")
        {
            executeSetProperty(command);
        }
        else if (type == "selectElement")
        {
            executeSelectElement(command);
        }
        else
        {
            emit commandError(QString("Unknown command type: %1").arg(type));
            return;
        }

        emit commandExecuted(description, true);
    }
    catch (const std::exception &e)
    {
        emit commandError(QString("Command execution failed: %1").arg(e.what()));
    }
}

void AICommandDispatcher::executeCommands(const QJsonArray &commands)
{
    // Keep track of newly created elements in this batch
    QMap<QString, QString> tempIdMapping; // temp ID -> actual ID
    bool hasCreatedElements = false;

    for (int i = 0; i < commands.size(); ++i)
    {
        const QJsonValue &value = commands[i];
        if (!value.isObject())
            continue;

        QJsonObject command = value.toObject();
        
        // Track if we're creating elements
        QString type = command["type"].toString();
        if (type == "createElement") {
            hasCreatedElements = true;
        }

        // Replace any temp IDs in the command with actual IDs
        if (command.contains("elementId"))
        {
            QString elementId = command["elementId"].toString();
            if (tempIdMapping.contains(elementId))
            {
                qDebug() << "Replacing tempId" << elementId << "with actual ID" << tempIdMapping[elementId];
                command["elementId"] = tempIdMapping[elementId];
            }
            else
            {
                qDebug() << "ElementId" << elementId << "not found in tempIdMapping. Available mappings:" << tempIdMapping.keys();
            }
        }

        if (command.contains("params"))
        {
            QJsonObject params = command["params"].toObject();
            if (params.contains("parentId"))
            {
                QString parentId = params["parentId"].toString();
                if (tempIdMapping.contains(parentId))
                {
                    params["parentId"] = tempIdMapping[parentId];
                }
            }
            command["params"] = params;
        }

        // Execute the command
        executeCommand(command);

        // If this was a createElement command, capture the created element's ID
        if (command["type"].toString() == "createElement")
        {
            // Get the most recently selected element (which should be the one we just created)
            auto selected = activeSelectionManager()->selectedElements();
            if (!selected.isEmpty())
            {
                Element *createdElement = selected.first();

                // If the command had a tempId, map it to the actual ID
                QJsonObject params = command["params"].toObject();
                if (params.contains("tempId"))
                {
                    QString tempId = params["tempId"].toString();
                    tempIdMapping[tempId] = createdElement->getId();
                    qDebug() << "Mapped tempId" << tempId << "to actual ID" << createdElement->getId();
                }
            }
        }

        // Process events to ensure the element is fully registered
        QCoreApplication::processEvents();
    }
    
    // If we created any elements, force a spatial index rebuild
    // This ensures all programmatically created frames are immediately clickable
    if (hasCreatedElements) {
        auto controller = activeController();
        if (controller && controller->hitTestService()) {
            controller->hitTestService()->rebuildSpatialIndex();
            qDebug() << "AICommandDispatcher: Forced spatial index rebuild after batch element creation";
        }
    }
}

void AICommandDispatcher::executeCreateElement(const QJsonObject &command)
{
    auto controller = activeController();
    if (!controller)
    {
        emit commandError("No active canvas controller");
        return;
    }

    QString elementType = command["elementType"].toString().toLower();
    QJsonObject params = command["params"].toObject();

    double x = params["x"].toDouble(100);
    double y = params["y"].toDouble(100);
    double width = params["width"].toDouble(200);
    double height = params["height"].toDouble(150);

    if (elementType == "frame")
    {
        controller->createFrame(QRectF(x, y, width, height));

        // Apply additional properties directly from params
        auto elements = activeSelectionManager()->selectedElements();
        if (!elements.isEmpty())
        {
            Element *frame = elements.first();

            // Apply properties like fill, backgroundColor, borderColor, etc.
            if (params.contains("fill"))
            {
                frame->setProperty("fill", params["fill"].toVariant());
            }
            if (params.contains("borderColor"))
            {
                frame->setProperty("borderColor", params["borderColor"].toVariant());
            }
            if (params.contains("borderWidth"))
            {
                frame->setProperty("borderWidth", params["borderWidth"].toVariant());
            }
            if (params.contains("borderRadius"))
            {
                frame->setProperty("borderRadius", params["borderRadius"].toVariant());
            }
            if (params.contains("overflow"))
            {
                frame->setProperty("overflow", params["overflow"].toVariant());
            }
        }
        
        // Force spatial index update for single frame creation
        // This ensures the frame is immediately clickable
        if (controller->hitTestService()) {
            controller->hitTestService()->rebuildSpatialIndex();
        }
    }
    else if (elementType == "text")
    {
        // Text must be created inside a frame
        QString parentId = params["parentId"].toString();
        if (parentId.isEmpty())
        {
            // If no parent specified, use the selected frame
            auto selected = activeSelectionManager()->selectedElements();
            for (Element *elem : selected)
            {
                if (auto frame = qobject_cast<Frame *>(elem))
                {
                    parentId = frame->getId();
                    break;
                }
            }
        }

        if (!parentId.isEmpty())
        {
            Element *parent = activeElementModel()->getElementById(parentId);
            if (auto frame = qobject_cast<Frame *>(parent))
            {
                // Check for 'content' first (AI uses this), then fall back to 'text'
                QString text = params["content"].toString();
                if (text.isEmpty())
                {
                    text = params["text"].toString("New Text");
                }

                // Create text element with absolute positioning
                QString textId = UniqueIdGenerator::generate16DigitId();
                Text *textElement = new Text(textId, activeElementModel());
                textElement->setContent(text);
                textElement->setParentElementId(frame->getId());

                // Use absolute coordinates from params
                double textX = params["x"].toDouble(frame->x());
                double textY = params["y"].toDouble(frame->y());
                double textWidth = params["width"].toDouble(frame->width());
                double textHeight = params["height"].toDouble(30);

                textElement->setX(textX);
                textElement->setY(textY);
                textElement->setWidth(textWidth);
                textElement->setHeight(textHeight);

                qDebug() << "AI creating text element:" << textId
                         << "at absolute position" << textX << "," << textY
                         << "inside frame" << frame->getId()
                         << "which is at" << frame->x() << "," << frame->y();

                // Apply text properties
                if (params.contains("textSize"))
                {
                    textElement->setProperty("textSize", params["textSize"].toVariant());
                }

                activeElementModel()->addElement(textElement);

                // Select the text element temporarily for tempId mapping
                // This allows executeCommands() to map the text element's tempId to its actual ID
                activeSelectionManager()->clearSelection();
                activeSelectionManager()->selectElement(textElement);
            }
            else
            {
                emit commandError("Parent must be a Frame for Text elements");
            }
        }
        else
        {
            emit commandError("Text elements require a parent Frame");
        }
    }
    else if (elementType == "node")
    {
        // For script canvas elements
        QString nodeType = params["nodeType"].toString("Operation");
        QString nodeTitle = params["nodeTitle"].toString("New Node");
        controller->createNode(QPointF(x, y), nodeType, nodeTitle);
    }
    else if (elementType == "edge")
    {
        // Edges require source and target nodes
        QString sourceId = params["sourceNodeId"].toString();
        QString targetId = params["targetNodeId"].toString();
        if (!sourceId.isEmpty() && !targetId.isEmpty())
        {
            controller->createEdge(sourceId, targetId);
        }
        else
        {
            emit commandError("Edges require sourceNodeId and targetNodeId");
        }
    }
    else
    {
        emit commandError(QString("Unknown element type: %1").arg(elementType));
    }
}

void AICommandDispatcher::executeDeleteElement(const QJsonObject &command)
{
    auto controller = activeController();
    if (!controller)
    {
        emit commandError("No active canvas controller");
        return;
    }

    QString elementId = command["elementId"].toString();
    if (elementId.isEmpty())
    {
        emit commandError("elementId is required for deleteElement command");
        return;
    }

    Element *element = activeElementModel()->getElementById(elementId);
    if (element)
    {
        activeSelectionManager()->clearSelection();
        activeSelectionManager()->selectElement(element);
        controller->deleteSelectedElements();
    }
    else
    {
        emit commandError(QString("Element not found: %1").arg(elementId));
    }
}

void AICommandDispatcher::executeMoveElement(const QJsonObject &command)
{
    auto controller = activeController();
    if (!controller)
    {
        emit commandError("No active canvas controller");
        return;
    }

    QString elementId = command["elementId"].toString();
    double deltaX = command["deltaX"].toDouble();
    double deltaY = command["deltaY"].toDouble();

    Element *element = activeElementModel()->getElementById(elementId);
    if (element)
    {
        QList<Element *> elements = {element};
        controller->moveElements(elements, QPointF(deltaX, deltaY));
    }
    else
    {
        emit commandError(QString("Element not found: %1").arg(elementId));
    }
}

void AICommandDispatcher::executeResizeElement(const QJsonObject &command)
{
    auto controller = activeController();
    if (!controller)
    {
        emit commandError("No active canvas controller");
        return;
    }

    QString elementId = command["elementId"].toString();
    double width = command["width"].toDouble();
    double height = command["height"].toDouble();

    Element *element = activeElementModel()->getElementById(elementId);
    if (auto canvasElement = qobject_cast<CanvasElement *>(element))
    {
        // Get current rectangle as old rect
        QRectF oldRect(canvasElement->x(), canvasElement->y(), canvasElement->width(), canvasElement->height());
        // Keep same position, just change size
        QRectF newRect(canvasElement->x(), canvasElement->y(), width, height);
        controller->resizeElement(canvasElement, oldRect, newRect);
    }
    else
    {
        emit commandError(QString("Element not found or not resizable: %1").arg(elementId));
    }
}

void AICommandDispatcher::executeSetProperty(const QJsonObject &command)
{
    QString elementId = command["elementId"].toString();
    QString property = command["property"].toString();
    QJsonValue value = command["value"];

    qDebug() << "executeSetProperty called - elementId:" << elementId << "property:" << property << "value:" << value;

    if (elementId.isEmpty() || property.isEmpty())
    {
        emit commandError("elementId and property are required for setProperty command");
        return;
    }

    Element *element = activeElementModel()->getElementById(elementId);
    if (element)
    {
        auto controller = activeController();
        if (controller)
        {
            QVariant variantValue = value.toVariant();

            // Special handling for color properties
            if (property == "fill" || property == "borderColor" || property == "color")
            {
                QString colorString = value.toString();
                if (!colorString.isEmpty())
                {
                    QColor color(colorString);
                    if (color.isValid())
                    {
                        variantValue = QVariant::fromValue(color);
                        qDebug() << "Converting color string" << colorString << "to QColor" << color.name();
                    }
                    else
                    {
                        emit commandError(QString("Invalid color value: %1").arg(colorString));
                        return;
                    }
                }
            }

            controller->setElementProperty(element, property, variantValue);
        }
    }
    else
    {
        emit commandError(QString("Element not found: %1").arg(elementId));
    }
}

void AICommandDispatcher::executeSelectElement(const QJsonObject &command)
{
    auto selectionManager = activeSelectionManager();
    if (!selectionManager)
    {
        emit commandError("No active selection manager");
        return;
    }

    QJsonArray elementIds = command["elementIds"].toArray();

    selectionManager->clearSelection();

    for (const QJsonValue &value : elementIds)
    {
        QString elementId = value.toString();
        Element *element = activeElementModel()->getElementById(elementId);
        if (element)
        {
            selectionManager->selectElement(element);
        }
    }
}

CanvasController *AICommandDispatcher::activeController() const
{
    return m_targetProject ? m_targetProject->controller() : nullptr;
}

ElementModel *AICommandDispatcher::activeElementModel() const
{
    return m_targetProject ? m_targetProject->elementModel() : nullptr;
}

SelectionManager *AICommandDispatcher::activeSelectionManager() const
{
    return m_targetProject ? m_targetProject->selectionManager() : nullptr;
}

bool AICommandDispatcher::validateCommand(const QJsonObject &command) const
{
    return command.contains("type") && command["type"].isString();
}