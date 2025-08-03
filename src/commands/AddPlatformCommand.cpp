#include "AddPlatformCommand.h"
#include "Project.h"
#include "Application.h"
#include "ProjectApiClient.h"
#include <QDebug>

AddPlatformCommand::AddPlatformCommand(Project* project, const QString& platformName, QObject *parent)
    : Command(parent)
    , m_project(project)
    , m_platformName(platformName)
{
    setDescription(QString("Add %1 platform").arg(platformName));
}

AddPlatformCommand::~AddPlatformCommand()
{
}

void AddPlatformCommand::execute()
{
    if (!m_project) {
        qWarning() << "AddPlatformCommand: Project is null";
        return;
    }

    // Add the platform to the project directly (avoid infinite recursion)
    m_project->addPlatformDirectly(m_platformName);
    
    // Sync with API
    syncWithAPI();
}

void AddPlatformCommand::undo()
{
    if (!m_project) {
        qWarning() << "AddPlatformCommand: Project is null";
        return;
    }

    // Remove the platform from the project
    m_project->removePlatform(m_platformName);
}

void AddPlatformCommand::syncWithAPI()
{
    if (!m_project) {
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
    QString apiProjectId = m_project->id();
    
    // Sync with API
    apiClient->syncAddPlatform(apiProjectId, m_platformName);
}