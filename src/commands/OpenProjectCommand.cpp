#include "OpenProjectCommand.h"
#include "../Application.h"
#include "../Project.h"
#include "../ElementModel.h"
#include "../Element.h"
#include <QDebug>
#include <QJsonArray>
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
    
    // Create a new project with the API project ID and name
    auto project = std::make_unique<Project>(m_projectId, m_projectName, m_application);
    
    // Initialize the project first (this creates controller, elementModel, etc.)
    project->initialize();
    
    // Deserialize the canvas data into the project
    if (!m_canvasData.isEmpty()) {
        // qDebug() << "OpenProjectCommand: Deserializing canvas data for project";
        
        // Set project properties if available
        if (m_canvasData.contains("viewMode")) {
            project->setViewMode(m_canvasData["viewMode"].toString());
        }
        
        if (m_canvasData.contains("platforms")) {
            QJsonArray platformsArray = m_canvasData["platforms"].toArray();
            QStringList platforms;
            for (const QJsonValue& value : platformsArray) {
                platforms.append(value.toString());
            }
            project->setPlatforms(platforms);
        }
        
        // Load elements
        if (m_canvasData.contains("elements")) {
            QJsonArray elementsArray = m_canvasData["elements"].toArray();
            ElementModel* model = project->elementModel();
            
            if (model) {
                for (const QJsonValue& elementValue : elementsArray) {
                    QJsonObject elementData = elementValue.toObject();
                    Element* element = m_application->deserializeElement(elementData, model);
                    if (element) {
                        model->addElement(element);
                    }
                }
                // qDebug() << "OpenProjectCommand: Loaded" << elementsArray.size() << "elements from project data";
            }
        }
    }
    
    // Add to application's projects list
    m_application->m_canvases.push_back(std::move(project));
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