#ifndef PROJECTAPICLIENT_H
#define PROJECTAPICLIENT_H

#include <QObject>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QJsonObject>
#include <QJsonArray>

class AuthenticationManager;
class Project;
class Application;

class ProjectApiClient : public QObject
{
    Q_OBJECT

public:
    explicit ProjectApiClient(AuthenticationManager* authManager, QObject *parent = nullptr);
    
    // Set the Application instance for accessing project data
    void setApplication(Application* app) { m_application = app; }
    ~ProjectApiClient();

    // Project API operations
    void createProject(const QString& name, const QJsonObject& canvasData);
    void fetchProjects();
    void listProjects(const QString& filter = QString(), int limit = 100, const QString& nextToken = QString());
    void getProject(const QString& projectId);
    void updateProject(const QString& apiProjectId, const QString& name, const QJsonObject& canvasData);
    void deleteProject(const QString& apiProjectId);
    
    // Canvas operation synchronization
    void syncCreateElement(const QString& apiProjectId, const QJsonObject& elementData);
    void syncUpdateElement(const QString& apiProjectId, const QString& elementId);
    void syncMoveElements(const QString& apiProjectId, const QJsonArray& elementIds);
    void syncDeleteElements(const QString& apiProjectId, const QJsonArray& elementIds);
    
    // Platform synchronization
    void syncAddPlatform(const QString& apiProjectId, const QString& platformName);
    void syncRemovePlatform(const QString& apiProjectId, const QString& platformName);

signals:
    // Success signals
    void projectCreated(const QString& apiProjectId, const QString& name);
    void projectsFetched(const QJsonArray& projects);
    void projectsListed(const QJsonArray& projects, const QString& nextToken);
    void projectFetched(const QString& projectId, const QJsonObject& project);
    void projectUpdated(const QString& apiProjectId);
    void projectDeleted(const QString& apiProjectId);
    
    // Canvas operation sync signals
    void elementCreated(const QString& apiProjectId, const QString& elementId);
    void elementUpdated(const QString& apiProjectId, const QString& elementId);
    void elementsDeleted(const QString& apiProjectId, const QJsonArray& elementIds);
    void elementsMoved(const QString& apiProjectId, const QJsonArray& elementIds);
    
    // Platform sync signals
    void platformAdded(const QString& apiProjectId, const QString& platformName);
    void platformRemoved(const QString& apiProjectId, const QString& platformName);
    
    // Error signals
    void createProjectFailed(const QString& error);
    void fetchProjectsFailed(const QString& error);
    void listProjectsFailed(const QString& error);
    void getProjectFailed(const QString& projectId, const QString& error);
    void updateProjectFailed(const QString& apiProjectId, const QString& error);
    void deleteProjectFailed(const QString& apiProjectId, const QString& error);
    
    // Canvas operation sync error signals
    void syncCreateElementFailed(const QString& apiProjectId, const QString& error);
    void syncUpdateElementFailed(const QString& apiProjectId, const QString& elementId, const QString& error);
    void syncMoveElementsFailed(const QString& apiProjectId, const QString& error);
    void syncDeleteElementsFailed(const QString& apiProjectId, const QString& error);
    
    // Platform sync error signals
    void syncAddPlatformFailed(const QString& apiProjectId, const QString& platformName, const QString& error);
    void syncRemovePlatformFailed(const QString& apiProjectId, const QString& platformName, const QString& error);

private slots:
    void onCreateProjectFinished();
    void onFetchProjectsFinished();
    void onListProjectsFinished();
    void onGetProjectFinished();
    void onUpdateProjectFinished();
    void onDeleteProjectFinished();
    
    // Canvas operation sync response handlers
    void onSyncCreateElementFinished();
    void onSyncUpdateElementFinished();
    void onSyncMoveElementsFinished();
    void onSyncDeleteElementsFinished();

private:
    struct PendingRequest {
        QString operation;
        QString projectId;
        QString projectName;
        QJsonObject data;
        QString elementId;        // For element-specific operations
        QJsonArray elementIds;    // For bulk operations
    };

    // Helper methods
    QNetworkRequest createAuthenticatedRequest() const;
    void sendGraphQLRequest(const QString& query, const QJsonObject& variables, 
                           const QString& operation, const QString& projectId = QString(),
                           const QString& projectName = QString());
    void handleGraphQLResponse(QNetworkReply* reply, const PendingRequest& request);
    void emitErrorForOperation(const QString& operation, const QString& projectId, const QString& error);
    void cleanupRequest(QNetworkReply* reply);

    AuthenticationManager* m_authManager;
    QNetworkAccessManager* m_networkManager;
    Application* m_application = nullptr;
    QHash<QNetworkReply*, PendingRequest> m_pendingRequests;
};

#endif // PROJECTAPICLIENT_H