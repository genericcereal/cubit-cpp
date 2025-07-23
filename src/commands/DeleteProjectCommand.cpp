#include "DeleteProjectCommand.h"
#include "../Application.h"
#include "../Project.h"
#include <QDebug>

DeleteProjectCommand::DeleteProjectCommand(Application* application, const QString& projectId, QObject *parent)
    : Command(parent)
    , m_application(application)
    , m_projectId(projectId)
    , m_projectDataCaptured(false)
{
    if (!m_application) {
        qWarning() << "DeleteProjectCommand: Application is null";
        setDescription("Delete project (invalid)");
        return;
    }

    // Get the project name for the description
    m_projectName = m_application->getProjectName(projectId);
    if (m_projectName.isEmpty()) {
        m_projectName = "Unknown Project";
    }

    setDescription(QString("Delete project '%1'").arg(m_projectName));
}

DeleteProjectCommand::~DeleteProjectCommand()
{
    // QPointer will automatically be null if the application was deleted
    // The Application manages project lifecycle, so we don't need to delete anything here
}

void DeleteProjectCommand::execute()
{
    if (!m_application || m_projectId.isEmpty()) {
        qWarning() << "DeleteProjectCommand: Cannot execute - application is null or project ID is empty";
        return;
    }

    // Get the project before deleting it
    Project* project = m_application->getProject(m_projectId);
    if (!project) {
        qWarning() << "DeleteProjectCommand: Project not found:" << m_projectId;
        return;
    }

    // Capture project data for undo if not already captured
    if (!m_projectDataCaptured) {
        m_projectData = m_application->serializeProjectData(project);
        m_projectDataCaptured = true;
        
        if (m_projectData.isEmpty()) {
            qWarning() << "DeleteProjectCommand: Failed to serialize project data for" << m_projectId;
        }
    }

    // Remove the project using Application's removeProject method
    m_application->removeProject(m_projectId);
    
    qDebug() << "DeleteProjectCommand: Deleted project" << m_projectName << "with ID" << m_projectId;
}

void DeleteProjectCommand::undo()
{
    if (!m_application || m_projectId.isEmpty() || !m_projectDataCaptured) {
        qWarning() << "DeleteProjectCommand: Cannot undo - application is null, project ID is empty, or project data not captured";
        return;
    }

    if (m_projectData.isEmpty()) {
        qWarning() << "DeleteProjectCommand: Cannot undo - no project data available";
        return;
    }

    // Recreate the project from the serialized data
    Project* restoredProject = m_application->deserializeProjectFromData(m_projectData);
    if (!restoredProject) {
        qWarning() << "DeleteProjectCommand: Failed to restore project from serialized data";
        return;
    }

    // Add the restored project back to the application
    m_application->addProject(restoredProject);
    
    qDebug() << "DeleteProjectCommand: Restored project" << m_projectName << "with ID" << m_projectId;
}