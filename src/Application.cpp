#include "Application.h"
#include "Project.h"
#include "SelectionManager.h"
#include "UniqueIdGenerator.h"
#include "Element.h"
#include "CanvasElement.h"
#include "DesignElement.h"
#include "Frame.h"
#include "Text.h"
#include "Shape.h"
#include "Variable.h"
#include "ScriptElement.h"
#include "Node.h"
#include "Edge.h"
#include "platforms/web/WebTextInput.h"
#include "ConsoleMessageRepository.h"
#include "AuthenticationManager.h"
#include "ElementTypeRegistry.h"
#include "ProjectApiClient.h"
#include "commands/CreateProjectCommand.h"
#include "commands/OpenProjectCommand.h"
#include "CommandHistory.h"
#include "Command.h"
#include "FileManager.h"
#include "Serializer.h"
#include <QDebug>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QStandardPaths>
#include <QDir>
#include <QMetaObject>
#include <QFile>
#include <QTextStream>
#include <QQmlApplicationEngine>
#include <QQmlComponent>
#include <QQmlContext>
#include <QJsonObject>
#include <QMetaProperty>
#include <QCoreApplication>

Application* Application::s_instance = nullptr;

Application::Application(QObject *parent)
    : QObject(parent)
{
    s_instance = this;
    
    // Create panels manager
    m_panels = std::make_unique<Panels>(this);
    
    // Create command history for application-level operations
    m_commandHistory = std::make_unique<CommandHistory>(this);
    
    // Create file manager and serializer
    m_fileManager = std::make_unique<FileManager>(this, this);
    m_serializer = std::make_unique<Serializer>(this, this);
    
    // Connect file manager signals
    connect(m_fileManager.get(), &FileManager::saveFileRequested, this, &Application::saveFileRequested);
    connect(m_fileManager.get(), &FileManager::openFileRequested, this, &Application::openFileRequested);
    
    // Don't create initial project - let user start from ProjectList
    // createProject("Project 1")
    
}

Application::~Application() {
    // Clear all canvases before destruction to ensure proper cleanup order
    m_canvases.clear();
    s_instance = nullptr;
}

Application* Application::instance() {
    return s_instance;
}

void Application::setAuthenticationManager(AuthenticationManager* authManager) {
    m_authManager = authManager;
    
    // Create ProjectApiClient when we have authentication
    if (authManager && !m_projectApiClient) {
        m_projectApiClient = std::make_unique<ProjectApiClient>(authManager, this);
        m_projectApiClient->setApplication(this);
        
        // Connect API client signals
        connect(m_projectApiClient.get(), &ProjectApiClient::projectsFetched, 
                this, &Application::projectsFetchedFromAPI, Qt::UniqueConnection);
        connect(m_projectApiClient.get(), &ProjectApiClient::fetchProjectsFailed, 
                this, &Application::apiErrorOccurred, Qt::UniqueConnection);
        connect(m_projectApiClient.get(), &ProjectApiClient::projectsListed,
                this, &Application::projectsListedFromAPI, Qt::UniqueConnection);
        connect(m_projectApiClient.get(), &ProjectApiClient::listProjectsFailed,
                this, &Application::apiErrorOccurred, Qt::UniqueConnection);
        connect(m_projectApiClient.get(), &ProjectApiClient::projectFetched,
                this, &Application::projectFetchedFromAPI, Qt::UniqueConnection);
        connect(m_projectApiClient.get(), &ProjectApiClient::getProjectFailed,
                this, [this](const QString& projectId, const QString& error) {
                    emit apiErrorOccurred(QString("Failed to fetch project %1: %2").arg(projectId, error));
                });
        connect(m_projectApiClient.get(), &ProjectApiClient::projectCreated,
                this, &Application::projectCreatedInAPI, Qt::UniqueConnection);
        connect(m_projectApiClient.get(), &ProjectApiClient::createProjectFailed,
                this, &Application::apiErrorOccurred, Qt::UniqueConnection);
        connect(m_projectApiClient.get(), &ProjectApiClient::projectDeleted,
                this, &Application::projectDeletedFromAPI, Qt::UniqueConnection);
        connect(m_projectApiClient.get(), &ProjectApiClient::deleteProjectFailed,
                this, [this](const QString& projectId, const QString& error) {
                    emit apiErrorOccurred(QString("Failed to delete project %1: %2").arg(projectId, error));
                });
    }
}

void Application::setEngine(QQmlApplicationEngine* engine) {
    m_engine = engine;
}

void Application::fetchProjectsFromAPI()
{
    if (!m_projectApiClient) {
        qWarning() << "Application::fetchProjectsFromAPI: No API client available";
        return;
    }

    m_projectApiClient->fetchProjects();
}

void Application::listProjects(const QString& filter, int limit, const QString& nextToken)
{
    if (!m_projectApiClient) {
        qWarning() << "Application::listProjects: No API client available";
        emit apiErrorOccurred("API client not available");
        return;
    }

    m_projectApiClient->listProjects(filter, limit, nextToken);
}

void Application::fetchProjectFromAPI(const QString& projectId)
{
    if (!m_projectApiClient) {
        qWarning() << "Application::fetchProjectFromAPI: No API client available";
        emit apiErrorOccurred("API client not available");
        return;
    }

    m_projectApiClient->getProject(projectId);
}

QString Application::createProject(const QString& name) {
    QString projectId = generateProjectId();
    QString projectName = name.isEmpty() ? QString("Project %1").arg(m_canvases.size() + 1) : name;
    
    // Creating new project
    
    auto project = std::make_unique<Project>(projectId, projectName, this);
    project->initialize();
    
    // Project initialized successfully
    
    m_canvases.push_back(std::move(project));
    
    // Don't automatically set as active - let windows manage their own canvases
    
    emit canvasListChanged();
    emit canvasCreated(projectId);
    
    return projectId;
}

void Application::removeProject(const QString& projectId) {
    // Check if this is an API project (API projects have UUIDs as IDs)
    bool isApiProject = projectId.contains('-') && projectId.length() == 36;
    
    // Look for the project in the open canvases
    auto it = std::find_if(m_canvases.begin(), m_canvases.end(),
        [&projectId](const std::unique_ptr<Project>& c) { return c->id() == projectId; });
    
    if (it != m_canvases.end()) {
        // Project is currently open, remove it from the canvas list
        m_canvases.erase(it);
        
        emit canvasListChanged();
        emit canvasRemoved(projectId);
    }
    
    // If it's an API project, delete it from the API regardless of whether it's open
    if (isApiProject && m_projectApiClient) {
        qDebug() << "Deleting API project:" << projectId;
        m_projectApiClient->deleteProject(projectId);
        // Don't emit projectDeletedFromAPI here - wait for the API response
    }
}


QString Application::getProjectName(const QString& projectId) const {
    const Project* project = findProject(projectId);
    return project ? project->name() : QString();
}

void Application::renameProject(const QString& projectId, const QString& newName) {
    Project* project = findProject(projectId);
    if (project && !newName.isEmpty()) {
        project->setName(newName);
        emit canvasListChanged();
    }
}

Project* Application::getProject(const QString& projectId) {
    return findProject(projectId);
}

void Application::addProject(Project* project) {
    if (!project) {
        qWarning() << "Application::addProject: Project is null";
        return;
    }

    // Check if project with this ID already exists
    if (findProject(project->id())) {
        qWarning() << "Application::addProject: Project with ID" << project->id() << "already exists";
        return;
    }

    // Take ownership and add to collection
    m_canvases.push_back(std::unique_ptr<Project>(project));
    
    emit canvasListChanged();
    emit canvasCreated(project->id());
    
    qDebug() << "Application::addProject: Added project" << project->name() << "with ID" << project->id();
}

void Application::createNewProject(const QString& projectName) {
    // Use CreateProjectCommand to create and sync with API
    // Execute directly without adding to command history (project management operations don't support undo)
    QString name = projectName.isEmpty() ? "New Project" : projectName;
    auto command = std::make_unique<CreateProjectCommand>(this, name);
    CreateProjectCommand* commandPtr = command.get();
    
    // Connect to apiSyncComplete signal to clean up when done
    connect(commandPtr, &CreateProjectCommand::apiSyncComplete, this, [this, commandPtr]() {
        // Remove the command from pending commands when sync is complete
        auto it = std::find_if(m_pendingCommands.begin(), m_pendingCommands.end(),
            [commandPtr](const std::unique_ptr<Command>& cmd) {
                return cmd.get() == commandPtr;
            });
        if (it != m_pendingCommands.end()) {
            m_pendingCommands.erase(it);
        }
    });
    
    // Execute the command directly
    command->execute();
    
    // Get the project ID and create window
    QString projectId = commandPtr->getCreatedProjectId();
    if (!projectId.isEmpty()) {
        // Create a new window for this project
        if (m_engine) {
            QQmlComponent component(m_engine, QUrl(QStringLiteral("qrc:/qml/ProjectWindow.qml")));
            if (component.isReady()) {
                QObject* window = component.create();
                if (window) {
                    // Set the canvas ID property on the window
                    window->setProperty("canvasId", projectId);
                    qDebug() << "Created window for new project:" << projectId;
                } else {
                    qWarning() << "Failed to create project window:" << component.errorString();
                }
            } else {
                qWarning() << "ProjectWindow component not ready:" << component.errorString();
            }
        } else {
            qWarning() << "QML engine not set, cannot create new window";
        }
    } else {
        qWarning() << "Failed to get project ID from CreateProjectCommand";
    }
    
    // Keep the command alive until API sync completes
    m_pendingCommands.push_back(std::move(command));
}

void Application::createNewProjectWithTemplate(const QString& projectName, const QString& templatePath) {
    QString name = projectName.isEmpty() ? "New Project" : projectName;
    QJsonObject canvasData;
    
    // Load the template QML file
    QString fullPath = "qrc:/" + templatePath;
    
    QQmlComponent component(m_engine, QUrl(fullPath));
    
    if (component.isReady()) {
        QObject* templateObject = component.create();
        if (templateObject) {
            // Get the canvasDataJson property from the QML object
            QVariant canvasDataVar = templateObject->property("canvasDataJson");
            if (canvasDataVar.isValid() && canvasDataVar.typeId() == QMetaType::QString) {
                QString jsonString = canvasDataVar.toString();
                
                // Parse the JSON string
                QJsonDocument doc = QJsonDocument::fromJson(jsonString.toUtf8());
                canvasData = doc.object();
            } else {
                qWarning() << "Template file does not have a valid canvasDataJson property";
            }
            delete templateObject;
        } else {
            qWarning() << "Failed to create template object from:" << fullPath;
        }
    } else {
        qWarning() << "Failed to load template component:" << component.errorString();
    }
    
    // Create the project with initial canvas data
    auto command = std::make_unique<CreateProjectCommand>(this, name, canvasData);
    CreateProjectCommand* commandPtr = command.get();
    
    // Connect to apiSyncComplete signal to clean up when done
    connect(commandPtr, &CreateProjectCommand::apiSyncComplete, this, [this, commandPtr]() {
        // Remove the command from pending commands when sync is complete
        auto it = std::find_if(m_pendingCommands.begin(), m_pendingCommands.end(),
            [commandPtr](const std::unique_ptr<Command>& cmd) {
                return cmd.get() == commandPtr;
            });
        if (it != m_pendingCommands.end()) {
            m_pendingCommands.erase(it);
        }
    });
    
    // Execute the command directly
    command->execute();
    
    // Get the project ID and create window
    QString projectId = commandPtr->getCreatedProjectId();
    if (!projectId.isEmpty()) {
        // Create a new window for this project
        if (m_engine) {
            QQmlComponent component(m_engine, QUrl(QStringLiteral("qrc:/qml/ProjectWindow.qml")));
            if (component.isReady()) {
                QObject* window = component.create();
                if (window) {
                    window->setProperty("canvasId", projectId);
                } else {
                    qWarning() << "Failed to create ProjectWindow:" << component.errorString();
                }
            } else {
                qWarning() << "ProjectWindow.qml component not ready:" << component.errorString();
            }
        }
    }
    
    // Keep the command alive until API sync completes
    m_pendingCommands.push_back(std::move(command));
}

void Application::openAPIProject(const QString& projectId, const QString& projectName, const QJsonObject& canvasData) {
    // qDebug() << "Application::openAPIProject called with ID:" << projectId << "Name:" << projectName;
    
    // Use OpenProjectCommand to open and deserialize the project
    // Execute directly without adding to command history (project management operations don't support undo)
    auto command = std::make_unique<OpenProjectCommand>(this, projectId, projectName, canvasData);
    command->execute();
}


QStringList Application::canvasIds() const {
    QStringList ids;
    for (const auto& canvas : m_canvases) {
        ids.append(canvas->id());
    }
    return ids;
}

QStringList Application::canvasNames() const {
    QStringList names;
    for (const auto& canvas : m_canvases) {
        names.append(canvas->name());
    }
    return names;
}



Project* Application::findProject(const QString& projectId) {
    auto it = std::find_if(m_canvases.begin(), m_canvases.end(),
        [&projectId](const std::unique_ptr<Project>& c) { return c->id() == projectId; });
    return (it != m_canvases.end()) ? it->get() : nullptr;
}

const Project* Application::findProject(const QString& projectId) const {
    auto it = std::find_if(m_canvases.begin(), m_canvases.end(),
        [&projectId](const std::unique_ptr<Project>& c) { return c->id() == projectId; });
    return (it != m_canvases.end()) ? it->get() : nullptr;
}

QString Application::generateProjectId() const {
    return UniqueIdGenerator::generate16DigitId();
}


Panels* Application::panels() const {
    if (!m_panels) {
        qWarning() << "Application::panels() called but m_panels is null!";
        return nullptr;
    }
    return m_panels.get();
}

AuthenticationManager* Application::authManager() const {
    return m_authManager;
}

ProjectApiClient* Application::projectApiClient() const {
    return m_projectApiClient.get();
}

FileManager* Application::fileManager() const {
    return m_fileManager.get();
}

Serializer* Application::serializer() const {
    return m_serializer.get();
}

void Application::addCanvas(Project* project) {
    if (!project) return;
    m_canvases.push_back(std::unique_ptr<Project>(project));
    emit canvasListChanged();
    emit canvasCreated(project->id());
}

Project* Application::deserializeProjectFromData(const QJsonObject& projectData)
{
    return m_serializer->deserializeProject(projectData);
}

QJsonObject Application::serializeProjectData(Project* project) const
{
    return m_serializer->serializeProject(project);
}

QJsonObject Application::serializeElement(Element* element) const
{
    return m_serializer->serializeElement(element);
}

bool Application::saveAs() {
    return m_fileManager->saveAs();
}

bool Application::saveToFile(const QString& fileName, Project* project) {
    return m_fileManager->saveToFile(fileName, project);
}

bool Application::openFile() {
    return m_fileManager->openFile();
}

bool Application::loadFromFile(const QString& fileName) {
    return m_fileManager->loadFromFile(fileName);
}

void Application::deleteProject(const QString& projectId) {
    // Check if the project is currently open
    auto it = std::find_if(m_canvases.begin(), m_canvases.end(),
        [&projectId](const std::unique_ptr<Project>& c) { return c->id() == projectId; });
    
    if (it != m_canvases.end()) {
        // Project is currently open, don't allow deletion
        qWarning() << "Cannot delete project" << projectId << "- project is currently open";
        emit apiErrorOccurred("Cannot delete an open project. Please close it first.");
        return;
    }
    
    // Project deletion is a permanent operation that should not be undoable
    // Directly remove the project without using the command pattern
    
    // Delete from API if it's an API project
    if (m_projectApiClient) {
        m_projectApiClient->deleteProject(projectId);
    }
}

void Application::closeProjectWindow(const QString& projectId) {
    // Find and remove the project from the open canvases
    auto it = std::find_if(m_canvases.begin(), m_canvases.end(),
        [&projectId](const std::unique_ptr<Project>& c) { return c->id() == projectId; });
    
    if (it != m_canvases.end()) {
        m_canvases.erase(it);
        emit canvasListChanged();
        emit canvasRemoved(projectId);
    }
}

void Application::updateProjectId(const QString& oldId, const QString& newId) {
    // Find the project with the old ID
    Project* project = findProject(oldId);
    if (!project) {
        qWarning() << "Application::updateProjectId - Project not found with ID:" << oldId;
        return;
    }
    
    // Update the project's ID
    project->setId(newId);
    
    // The project is now accessible by the new ID
    // No need to update m_canvases as it stores by pointer, not by ID
    
    emit canvasListChanged();  // Notify UI of the change
}

