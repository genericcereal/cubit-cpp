#pragma once

#include "Command.h"
#include <QString>
#include <QJsonObject>

class VariableBindingManager;
class VariableBinding;
class Project;

class AssignVariableCommand : public Command
{
    Q_OBJECT

public:
    // Constructor for creating a new binding
    AssignVariableCommand(Project* project,
                         const QString& variableId,
                         const QString& elementId,
                         const QString& propertyName,
                         QObject* parent = nullptr);
    
    // Constructor for removing an existing binding
    AssignVariableCommand(Project* project,
                         const QString& elementId,
                         const QString& propertyName,
                         QObject* parent = nullptr);
    
    ~AssignVariableCommand() override;

    void execute() override;
    void undo() override;
    
    // Serialization for API sync
    QJsonObject toJson() const;
    static AssignVariableCommand* fromJson(const QJsonObject& json, Project* project, QObject* parent = nullptr);

private:
    void syncWithApi();
    
private:
    Project* m_project;
    VariableBindingManager* m_bindingManager;
    QString m_variableId;
    QString m_elementId;
    QString m_propertyName;
    QString m_previousVariableId; // For undo - stores previous binding if any
    bool m_isRemoval; // True if this command removes a binding
};