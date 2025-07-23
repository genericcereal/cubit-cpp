#ifndef AICOMMANDDISPATCHER_H
#define AICOMMANDDISPATCHER_H

#include <QObject>
#include <QJsonObject>
#include <QJsonArray>
#include <memory>

class CanvasController;
class ElementModel;
class SelectionManager;
class Application;
class Project;

class AICommandDispatcher : public QObject
{
    Q_OBJECT

public:
    explicit AICommandDispatcher(Application* app, QObject* parent = nullptr);

    // Set the target project for AI commands
    void setTargetProject(Project* project);

    // Execute a single command
    void executeCommand(const QJsonObject& command);
    
    // Execute multiple commands in sequence
    void executeCommands(const QJsonArray& commands);

signals:
    void commandExecuted(const QString& commandType, bool success);
    void commandError(const QString& error);

private:
    Application* m_application;
    Project* m_targetProject = nullptr;
    
    // Command execution methods
    void executeCreateElement(const QJsonObject& params);
    void executeDeleteElement(const QJsonObject& params);
    void executeMoveElement(const QJsonObject& params);
    void executeResizeElement(const QJsonObject& params);
    void executeSetProperty(const QJsonObject& params);
    void executeSelectElement(const QJsonObject& params);
    
    // Helper methods
    CanvasController* activeController() const;
    ElementModel* activeElementModel() const;
    SelectionManager* activeSelectionManager() const;
    
    // Validate command structure
    bool validateCommand(const QJsonObject& command) const;
};

#endif // AICOMMANDDISPATCHER_H