#ifndef APPLICATION_H
#define APPLICATION_H

#include <QObject>
#include <QQmlEngine>
#include <QJsonObject>
#include <memory>
#include <vector>
#include "Project.h"
#include "Panels.h"

class Element;
class AuthenticationManager;
class ProjectApiClient;
class CommandHistory;
class Command;
class FileManager;
class Serializer;
Q_DECLARE_OPAQUE_POINTER(AuthenticationManager*)
class QQmlApplicationEngine;

class Application : public QObject {
    Q_OBJECT
    Q_PROPERTY(QStringList canvasIds READ canvasIds NOTIFY canvasListChanged)
    Q_PROPERTY(QStringList canvasNames READ canvasNames NOTIFY canvasListChanged)
    Q_PROPERTY(Panels* panels READ panels CONSTANT)
    Q_PROPERTY(AuthenticationManager* authManager READ authManager CONSTANT)
    
    friend class OpenProjectCommand;
    friend class FileManager;
    friend class Serializer;

public:
    explicit Application(QObject *parent = nullptr);
    ~Application();
    
    void setAuthenticationManager(AuthenticationManager* authManager);

    static Application* instance();

    // Project management
    Q_INVOKABLE QString createProject(const QString& name = QString());
    Q_INVOKABLE void removeProject(const QString& projectId);
    Q_INVOKABLE QString getProjectName(const QString& projectId) const;
    Q_INVOKABLE void renameProject(const QString& projectId, const QString& newName);
    Q_INVOKABLE Project* getProject(const QString& projectId);
    void addProject(Project* project);  // For command undo operations
    Q_INVOKABLE void createNewProject(const QString& projectName = QString());
    Q_INVOKABLE void createNewProjectWithTemplate(const QString& projectName, const QString& templatePath);
    Q_INVOKABLE void fetchProjectsFromAPI();  // Fetch projects from API
    Q_INVOKABLE void listProjects(const QString& filter = QString(), int limit = 100, const QString& nextToken = QString());
    Q_INVOKABLE void fetchProjectFromAPI(const QString& projectId);
    Q_INVOKABLE void openAPIProject(const QString& projectId, const QString& projectName, const QJsonObject& canvasData);
    
    // Engine management for multi-window support
    void setEngine(QQmlApplicationEngine* engine);

    // Property getters
    QStringList canvasIds() const;
    QStringList canvasNames() const;
    Panels* panels() const;
    AuthenticationManager* authManager() const;
    ProjectApiClient* projectApiClient() const;
    FileManager* fileManager() const;
    Serializer* serializer() const;
    QQmlApplicationEngine* qmlEngine() const { return m_engine; }
    const std::vector<std::unique_ptr<Project>>& canvases() const { return m_canvases; }
    void addCanvas(Project* project); // For use by Serializer

    // File operations (delegated to FileManager)
    Q_INVOKABLE bool saveAs();
    Q_INVOKABLE bool openFile();
    Q_INVOKABLE bool saveToFile(const QString& fileName, Project* project = nullptr);
    Q_INVOKABLE bool loadFromFile(const QString& fileName);
    Q_INVOKABLE void deleteProject(const QString& projectId);
    Q_INVOKABLE void closeProjectWindow(const QString& projectId);
    
    // Public access for commands (delegated to Serializer)
    Project* deserializeProjectFromData(const QJsonObject& projectData);
    QJsonObject serializeProjectData(Project* project) const;
    QJsonObject serializeElement(Element* element) const;
    void updateProjectId(const QString& oldId, const QString& newId);
    

signals:
    void canvasListChanged();
    void canvasCreated(const QString& canvasId);
    void canvasRemoved(const QString& canvasId);
    void canvasIdUpdated(const QString& oldId, const QString& newId);
    void saveFileRequested();
    void openFileRequested();
    void projectsFetchedFromAPI(const QJsonArray& projects);
    void projectsListedFromAPI(const QJsonArray& projects, const QString& nextToken);
    void projectFetchedFromAPI(const QString& projectId, const QJsonObject& project);
    void projectCreatedInAPI(const QString& projectId, const QString& projectName);
    void projectDeletedFromAPI(const QString& projectId);
    void apiErrorOccurred(const QString& error);

private slots:

private:
    static Application* s_instance;
    std::vector<std::unique_ptr<Project>> m_canvases;
    std::unique_ptr<Panels> m_panels;
    AuthenticationManager* m_authManager = nullptr;
    QQmlApplicationEngine* m_engine = nullptr;
    std::unique_ptr<ProjectApiClient> m_projectApiClient;
    std::unique_ptr<CommandHistory> m_commandHistory;
    std::unique_ptr<FileManager> m_fileManager;
    std::unique_ptr<Serializer> m_serializer;
    std::vector<std::unique_ptr<Command>> m_pendingCommands;  // Keep async commands alive
    
    Project* findProject(const QString& projectId);
    const Project* findProject(const QString& projectId) const;
    QString generateProjectId() const;
};

#endif // APPLICATION_H