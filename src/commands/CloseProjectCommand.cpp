#include "CloseProjectCommand.h"
#include "../Application.h"
#include "../Project.h"
#include "../ProjectApiClient.h"
#include "../Serializer.h"
#include <QDebug>
#include <QTimer>

CloseProjectCommand::CloseProjectCommand(Application* application, const QString& projectId,
                                       QObject *parent)
    : Command(parent)
    , m_application(application)
    , m_projectId(projectId)
    , m_syncInProgress(false)
{
    setDescription(QString("Close project"));
}

CloseProjectCommand::~CloseProjectCommand()
{
    // QPointer will automatically be null if the application was deleted
}

void CloseProjectCommand::execute()
{
    if (!m_application) {
        qWarning() << "CloseProjectCommand: Application is null";
        return;
    }

    if (m_projectId.isEmpty()) {
        qWarning() << "CloseProjectCommand: Project ID is empty";
        return;
    }

    // Get the project to save its state for undo
    Project* project = m_application->getProject(m_projectId);
    if (!project) {
        qWarning() << "CloseProjectCommand: Project not found:" << m_projectId;
        return;
    }

    // Serialize the project state before closing
    m_savedProjectData = m_application->serializeProjectData(project);
    
    // Store the project name for the description
    QString projectName = project->name();
    setDescription(QString("Close project '%1'").arg(projectName));

    // Sync with API before closing
    ProjectApiClient* apiClient = m_application->projectApiClient();
    if (apiClient && !m_syncInProgress) {
        m_syncInProgress = true;
        
        // Connect to API client signals
        connect(apiClient, &ProjectApiClient::projectUpdated, this, &CloseProjectCommand::onApiSyncComplete, Qt::UniqueConnection);
        connect(apiClient, &ProjectApiClient::updateProjectFailed, this, &CloseProjectCommand::onApiSyncFailed, Qt::UniqueConnection);
        
        // Update project via API - need to extract the name from saved data
        QString projectName = m_savedProjectData.value("name").toString();
        apiClient->updateProject(m_projectId, projectName, m_savedProjectData);
    } else {
        // No API client or sync already in progress, proceed directly
        performClose();
    }
}

void CloseProjectCommand::undo()
{
    if (!m_application) {
        qWarning() << "CloseProjectCommand: Cannot undo - application is null";
        return;
    }

    if (m_projectId.isEmpty() || m_savedProjectData.isEmpty()) {
        qWarning() << "CloseProjectCommand: Cannot undo - no saved project data";
        return;
    }

    // Recreate the project with its original ID
    QString projectName = m_savedProjectData.value("name").toString();
    if (projectName.isEmpty()) {
        projectName = "Restored Project";
    }

    // Deserialize the project from saved data
    Project* project = m_application->serializer()->deserializeProject(m_savedProjectData);
    if (project) {
        // Add the project back to the application
        m_application->addProject(project);
    }
}

void CloseProjectCommand::onApiSyncComplete()
{
    // Disconnect signals
    if (m_application && m_application->projectApiClient()) {
        disconnect(m_application->projectApiClient(), &ProjectApiClient::projectUpdated, this, &CloseProjectCommand::onApiSyncComplete);
        disconnect(m_application->projectApiClient(), &ProjectApiClient::updateProjectFailed, this, &CloseProjectCommand::onApiSyncFailed);
    }
    
    m_syncInProgress = false;
    
    // Refresh the project list from API
    if (m_application) {
        m_application->fetchProjectsFromAPI();
    }
    
    performClose();
}

void CloseProjectCommand::onApiSyncFailed(const QString& error)
{
    qWarning() << "CloseProjectCommand: API sync failed -" << error;
    
    // Disconnect signals
    if (m_application && m_application->projectApiClient()) {
        disconnect(m_application->projectApiClient(), &ProjectApiClient::projectUpdated, this, &CloseProjectCommand::onApiSyncComplete);
        disconnect(m_application->projectApiClient(), &ProjectApiClient::updateProjectFailed, this, &CloseProjectCommand::onApiSyncFailed);
    }
    
    m_syncInProgress = false;
    
    // Proceed with close even if sync failed
    performClose();
}

void CloseProjectCommand::performClose()
{
    if (!m_application) {
        qWarning() << "CloseProjectCommand: Application is null during close";
        return;
    }

    // Close the project
    m_application->removeProject(m_projectId);
    
    // Emit signal to indicate close is complete
    emit closeComplete();
}