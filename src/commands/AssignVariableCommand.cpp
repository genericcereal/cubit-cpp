#include "AssignVariableCommand.h"
#include "VariableBinding.h"
#include "Project.h"
#include "Application.h"
#include "ProjectApiClient.h"
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QDebug>

AssignVariableCommand::AssignVariableCommand(Project* project,
                                           const QString& variableId,
                                           const QString& elementId,
                                           const QString& propertyName,
                                           QObject* parent)
    : Command(parent)
    , m_project(project)
    , m_bindingManager(project ? project->bindingManager() : nullptr)
    , m_variableId(variableId)
    , m_elementId(elementId)
    , m_propertyName(propertyName)
    , m_isRemoval(false)
{
    if (m_bindingManager) {
        // Check if there's an existing binding
        m_previousVariableId = m_bindingManager->getBoundVariableId(elementId, propertyName);
    }
    
    QString desc = QString("Assign variable %1 to %2.%3")
                       .arg(variableId)
                       .arg(elementId)
                       .arg(propertyName);
    setDescription(desc);
}

AssignVariableCommand::AssignVariableCommand(Project* project,
                                           const QString& elementId,
                                           const QString& propertyName,
                                           QObject* parent)
    : Command(parent)
    , m_project(project)
    , m_bindingManager(project ? project->bindingManager() : nullptr)
    , m_elementId(elementId)
    , m_propertyName(propertyName)
    , m_isRemoval(true)
{
    if (m_bindingManager) {
        // Store the current binding for undo
        m_previousVariableId = m_bindingManager->getBoundVariableId(elementId, propertyName);
    }
    
    QString desc = QString("Remove variable binding from %1.%2")
                       .arg(elementId)
                       .arg(propertyName);
    setDescription(desc);
}

AssignVariableCommand::~AssignVariableCommand() = default;

void AssignVariableCommand::execute()
{
    if (!m_bindingManager) {
        qWarning() << "AssignVariableCommand::execute - No binding manager available";
        return;
    }
    
    if (m_isRemoval || m_variableId.isEmpty()) {
        // Remove binding
        m_bindingManager->removeBinding(m_elementId, m_propertyName);
    } else {
        // Create binding
        m_bindingManager->createBinding(m_variableId, m_elementId, m_propertyName);
    }
    
    // Sync with API
    syncWithApi();
}

void AssignVariableCommand::undo()
{
    if (!m_bindingManager) {
        qWarning() << "AssignVariableCommand::undo - No binding manager available";
        return;
    }
    
    if (m_previousVariableId.isEmpty()) {
        // There was no previous binding, remove the current one
        m_bindingManager->removeBinding(m_elementId, m_propertyName);
    } else {
        // Restore previous binding
        m_bindingManager->createBinding(m_previousVariableId, m_elementId, m_propertyName);
    }
    
    // Sync with API
    syncWithApi();
}

QJsonObject AssignVariableCommand::toJson() const
{
    QJsonObject json;
    json["type"] = "AssignVariable";
    json["variableId"] = m_variableId;
    json["elementId"] = m_elementId;
    json["propertyName"] = m_propertyName;
    json["isRemoval"] = m_isRemoval;
    
    if (!m_previousVariableId.isEmpty()) {
        json["previousVariableId"] = m_previousVariableId;
    }
    
    return json;
}

AssignVariableCommand* AssignVariableCommand::fromJson(const QJsonObject& json, Project* project, QObject* parent)
{
    QString elementId = json["elementId"].toString();
    QString propertyName = json["propertyName"].toString();
    bool isRemoval = json["isRemoval"].toBool();
    
    AssignVariableCommand* command = nullptr;
    
    if (isRemoval) {
        command = new AssignVariableCommand(project, elementId, propertyName, parent);
    } else {
        QString variableId = json["variableId"].toString();
        command = new AssignVariableCommand(project, variableId, elementId, propertyName, parent);
    }
    
    // Restore previous variable ID if present
    if (json.contains("previousVariableId")) {
        command->m_previousVariableId = json["previousVariableId"].toString();
    }
    
    return command;
}

void AssignVariableCommand::syncWithApi()
{
    if (!m_project) {
        qWarning() << "AssignVariableCommand::syncWithApi - No project available";
        return;
    }
    
    // Get all current bindings from the binding manager
    QVariantList allBindings = m_bindingManager->serialize();
    
    // Create the API payload
    QJsonObject payload;
    payload["projectId"] = m_project->id();
    payload["bindings"] = QJsonArray::fromVariantList(allBindings);
    
    // Get the API client from Application
    Application* app = qobject_cast<Application*>(m_project->parent());
    if (!app) {
        qWarning() << "AssignVariableCommand::syncWithApi - No application parent";
        return;
    }
    
    ProjectApiClient* apiClient = app->projectApiClient();
    if (!apiClient) {
        qWarning() << "AssignVariableCommand::syncWithApi - No API client available";
        return;
    }
    
    // TODO: Add syncVariableBindings method to ProjectApiClient for syncing bindings
    // For now, API sync is not implemented
    // apiClient->syncVariableBindings(m_project->id(), allBindings);
}