#include "CreateProjectCommand.h"
#include "../Application.h"
#include "../ProjectApiClient.h"
#include "../Project.h"
#include <QDebug>

CreateProjectCommand::CreateProjectCommand(Application* application, const QString& projectName, QObject *parent)
    : Command(parent)
    , m_application(application)
    , m_projectName(projectName.isEmpty() ? "New Project" : projectName)
{
    setDescription(QString("Create project '%1'").arg(m_projectName));
}

CreateProjectCommand::~CreateProjectCommand()
{
    // QPointer will automatically be null if the application was deleted
    // The Application manages project lifecycle, so we don't need to delete anything here
}

void CreateProjectCommand::execute()
{
    if (!m_application) {
        qWarning() << "CreateProjectCommand: Application is null";
        return;
    }

    // Create the project using Application's createProject method
    m_projectId = m_application->createProject(m_projectName);
    
    if (m_projectId.isEmpty()) {
        qWarning() << "CreateProjectCommand: Failed to create project" << m_projectName;
        return;
    }
    
    qDebug() << "CreateProjectCommand: Created project" << m_projectName << "with ID" << m_projectId;
    
    // Sync with API
    syncWithAPI();
}

void CreateProjectCommand::undo()
{
    if (!m_application || m_projectId.isEmpty()) {
        qWarning() << "CreateProjectCommand: Cannot undo - application is null or project ID is empty";
        return;
    }

    // Remove the project using Application's removeProject method
    m_application->removeProject(m_projectId);
    
    qDebug() << "CreateProjectCommand: Removed project" << m_projectName << "with ID" << m_projectId;
    
    // TODO: Also remove from API when delete API is implemented
}

void CreateProjectCommand::syncWithAPI()
{
    if (!m_application) {
        qWarning() << "CreateProjectCommand: Cannot sync with API - application is null";
        return;
    }

    // Get the project to serialize its data
    Project* project = m_application->getProject(m_projectId);
    if (!project) {
        qWarning() << "CreateProjectCommand: Cannot sync with API - project not found";
        return;
    }

    // Get the ProjectApiClient from Application
    ProjectApiClient* apiClient = m_application->projectApiClient();
    if (!apiClient) {
        qWarning() << "CreateProjectCommand: Cannot sync with API - no API client available";
        return;
    }

    // Connect to API client signals
    connect(apiClient, &ProjectApiClient::projectCreated, this, &CreateProjectCommand::onApiProjectCreated, Qt::UniqueConnection);
    connect(apiClient, &ProjectApiClient::createProjectFailed, this, &CreateProjectCommand::onApiCreateFailed, Qt::UniqueConnection);

    // Serialize the project data
    QJsonObject canvasData = m_application->serializeProjectData(project);
    
    // Create project via API client
    apiClient->createProject(m_projectName, canvasData);
    
    qDebug() << "CreateProjectCommand: Syncing project creation to API...";
}

void CreateProjectCommand::onApiProjectCreated(const QString& apiProjectId, const QString& name)
{
    // Only handle if this is for our project
    if (name == m_projectName) {
        m_apiProjectId = apiProjectId;
        qDebug() << "CreateProjectCommand: Successfully synced project to API with ID:" << m_apiProjectId;
        
        // Disconnect signals to avoid handling other projects
        if (m_application && m_application->projectApiClient()) {
            disconnect(m_application->projectApiClient(), &ProjectApiClient::projectCreated, this, &CreateProjectCommand::onApiProjectCreated);
            disconnect(m_application->projectApiClient(), &ProjectApiClient::createProjectFailed, this, &CreateProjectCommand::onApiCreateFailed);
        }
    }
}

void CreateProjectCommand::onApiCreateFailed(const QString& error)
{
    qWarning() << "CreateProjectCommand: API sync failed -" << error;
    
    // Disconnect signals
    if (m_application && m_application->projectApiClient()) {
        disconnect(m_application->projectApiClient(), &ProjectApiClient::projectCreated, this, &CreateProjectCommand::onApiProjectCreated);
        disconnect(m_application->projectApiClient(), &ProjectApiClient::createProjectFailed, this, &CreateProjectCommand::onApiCreateFailed);
    }
}