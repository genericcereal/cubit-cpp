#include "OpenProjectCommand.h"
#include "../Application.h"
#include "../Project.h"
#include "../ElementModel.h"
#include "../Element.h"
#include "../Serializer.h"
#include <QDebug>
#include <QJsonArray>
#include <QJsonDocument>
#include <QQmlApplicationEngine>
#include <QQmlComponent>

OpenProjectCommand::OpenProjectCommand(Application* application, 
                                     const QString& projectId, 
                                     const QString& projectName, 
                                     const QJsonObject& canvasData,
                                     QObject *parent)
    : Command(parent)
    , m_application(application)
    , m_projectId(projectId)
    , m_projectName(projectName)
    , m_canvasData(canvasData)
    , m_projectWasCreated(false)
{
    setDescription(QString("Open project '%1'").arg(m_projectName));
}

OpenProjectCommand::~OpenProjectCommand()
{
    // QPointer will automatically be null if the application was deleted
    // The Application manages project lifecycle, so we don't need to delete anything here
}

void OpenProjectCommand::execute()
{
    if (!m_application) {
        qWarning() << "OpenProjectCommand: Application is null";
        return;
    }

    // qDebug() << "OpenProjectCommand: Opening project" << m_projectName << "with ID" << m_projectId;
    
    // Check if project already exists
    Project* existingProject = m_application->getProject(m_projectId);
    if (existingProject) {
        qDebug() << "OpenProjectCommand: Project already exists, not creating duplicate";
        m_projectWasCreated = false;
        
        // Just create a new window for existing project
        if (m_application->m_engine) {
            QQmlComponent component(m_application->m_engine, QUrl(QStringLiteral("qrc:/qml/ProjectWindow.qml")));
            if (component.isReady()) {
                QObject* window = component.create();
                if (window) {
                    window->setProperty("canvasId", m_projectId);
                    qDebug() << "OpenProjectCommand: Created new window for existing project";
                }
            }
        }
        return;
    }
    
    // Use the Serializer to deserialize the project data properly
    // This will handle the new platform format with scripts
    qDebug() << "OpenProjectCommand: Using serializer to deserialize project data";
    
    // Ensure the canvas data has the required ID and name
    QJsonObject projectData = m_canvasData;
    projectData["id"] = m_projectId;
    projectData["name"] = m_projectName;
    
    Project* project = m_application->serializer()->deserializeProject(projectData);
    
    if (!project) {
        qWarning() << "OpenProjectCommand: Failed to deserialize project";
        return;
    }
    
    m_projectWasCreated = true;
    
    emit m_application->canvasListChanged();
    
    // Create a new window for this project
    if (m_application->m_engine) {
        QQmlComponent component(m_application->m_engine, QUrl(QStringLiteral("qrc:/qml/ProjectWindow.qml")));
        if (component.isReady()) {
            QObject* window = component.create();
            if (window) {
                // Set the canvas ID property on the window
                window->setProperty("canvasId", m_projectId);
                
                // qDebug() << "OpenProjectCommand: Successfully opened project in new window";
            } else {
                qWarning() << "OpenProjectCommand: Failed to create project window:" << component.errorString();
            }
        } else {
            qWarning() << "OpenProjectCommand: ProjectWindow component not ready:" << component.errorString();
        }
    } else {
        qWarning() << "OpenProjectCommand: QML engine not set, cannot create new window";
    }
}

void OpenProjectCommand::undo()
{
    if (!m_application || m_projectId.isEmpty() || !m_projectWasCreated) {
        qWarning() << "OpenProjectCommand: Cannot undo - application is null, project ID is empty, or project was not created by this command";
        return;
    }

    // Remove the project using Application's removeProject method
    m_application->removeProject(m_projectId);
    
    qDebug() << "OpenProjectCommand: Removed opened project" << m_projectName << "with ID" << m_projectId;
}