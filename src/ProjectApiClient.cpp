#include "ProjectApiClient.h"
#include "AuthenticationManager.h"
#include "Config.h"
#include "Application.h"
#include "Project.h"
#include <QDebug>
#include <QJsonDocument>
#include <QNetworkRequest>
#include <QUuid>

ProjectApiClient::ProjectApiClient(AuthenticationManager* authManager, QObject *parent)
    : QObject(parent)
    , m_authManager(authManager)
    , m_networkManager(new QNetworkAccessManager(this))
{
}

ProjectApiClient::~ProjectApiClient()
{
    // Cancel any pending requests
    for (auto it = m_pendingRequests.begin(); it != m_pendingRequests.end(); ++it) {
        QNetworkReply* reply = it.key();
        if (reply) {
            reply->abort();
            reply->deleteLater();
        }
    }
    m_pendingRequests.clear();
}

void ProjectApiClient::createProject(const QString& name, const QJsonObject& canvasData)
{
    if (!m_authManager || !m_authManager->isAuthenticated()) {
        emit createProjectFailed("Not authenticated");
        return;
    }

    QString mutation = R"(
        mutation CreateProject($input: CreateProjectInput!) {
            createProject(input: $input) {
                id
                name
                teamId
                canvasData
                createdAt
                updatedAt
            }
        }
    )";

    QJsonObject variables;
    QJsonObject input;
    input["name"] = name;
    input["teamId"] = "default-team"; // TODO: Replace with actual team ID when teams are implemented
    
    // Convert canvasData object to JSON string - GraphQL schema expects String type
    QJsonDocument canvasDoc(canvasData);
    input["canvasData"] = QString(canvasDoc.toJson(QJsonDocument::Compact));
    
    variables["input"] = input;

    // Debug: Log what we're sending
    qDebug() << "ProjectApiClient: Sending canvasData as string:" << input["canvasData"].toString();
    qDebug() << "ProjectApiClient: Full input:" << QJsonDocument(input).toJson(QJsonDocument::Compact);

    sendGraphQLRequest(mutation, variables, "createProject", QString(), name);
}

void ProjectApiClient::fetchProjects()
{
    if (!m_authManager || !m_authManager->isAuthenticated()) {
        emit fetchProjectsFailed("Not authenticated");
        return;
    }

    QString query = R"(
        query ListProjects {
            listProjects {
                items {
                    id
                    name
                    teamId
                    canvasData
                    createdAt
                    updatedAt
                }
            }
        }
    )";

    QJsonObject variables; // Empty for list query
    sendGraphQLRequest(query, variables, "fetchProjects");
}

void ProjectApiClient::listProjects(const QString& filter, int limit, const QString& nextToken)
{
    if (!m_authManager || !m_authManager->isAuthenticated()) {
        emit listProjectsFailed("Not authenticated");
        return;
    }

    QString query = R"(
        query ListProjects($filter: ModelProjectFilterInput, $limit: Int, $nextToken: String) {
            listProjects(filter: $filter, limit: $limit, nextToken: $nextToken) {
                items {
                    id
                    name
                    teamId
                    createdAt
                    updatedAt
                }
                nextToken
            }
        }
    )";

    QJsonObject variables;
    if (!filter.isEmpty()) {
        QJsonDocument filterDoc = QJsonDocument::fromJson(filter.toUtf8());
        if (!filterDoc.isNull()) {
            variables["filter"] = filterDoc.object();
        }
    }
    variables["limit"] = limit;
    if (!nextToken.isEmpty()) {
        variables["nextToken"] = nextToken;
    }
    sendGraphQLRequest(query, variables, "listProjects");
}

void ProjectApiClient::getProject(const QString& projectId)
{
    if (!m_authManager || !m_authManager->isAuthenticated()) {
        emit getProjectFailed(projectId, "Not authenticated");
        return;
    }

    QString query = R"(
        query GetProject($id: ID!) {
            getProject(id: $id) {
                id
                name
                teamId
                canvasData
                createdAt
                updatedAt
            }
        }
    )";

    QJsonObject variables;
    variables["id"] = projectId;
    sendGraphQLRequest(query, variables, "getProject", projectId);
}

void ProjectApiClient::updateProject(const QString& apiProjectId, const QString& name, const QJsonObject& canvasData)
{
    if (!m_authManager || !m_authManager->isAuthenticated()) {
        emit updateProjectFailed(apiProjectId, "Not authenticated");
        return;
    }

    QString mutation = R"(
        mutation UpdateProject($input: UpdateProjectInput!) {
            updateProject(input: $input) {
                id
                name
                teamId
                canvasData
                updatedAt
            }
        }
    )";

    QJsonObject variables;
    QJsonObject input;
    input["id"] = apiProjectId;
    input["name"] = name;
    
    // Convert canvasData object to JSON string - GraphQL schema expects String type
    QJsonDocument canvasDoc(canvasData);
    input["canvasData"] = QString(canvasDoc.toJson(QJsonDocument::Compact));
    
    variables["input"] = input;

    sendGraphQLRequest(mutation, variables, "updateProject", apiProjectId, name);
}

void ProjectApiClient::deleteProject(const QString& apiProjectId)
{
    if (!m_authManager || !m_authManager->isAuthenticated()) {
        emit deleteProjectFailed(apiProjectId, "Not authenticated");
        return;
    }

    QString mutation = R"(
        mutation DeleteProject($input: DeleteProjectInput!) {
            deleteProject(input: $input) {
                id
            }
        }
    )";

    QJsonObject variables;
    QJsonObject input;
    input["id"] = apiProjectId;
    variables["input"] = input;

    sendGraphQLRequest(mutation, variables, "deleteProject", apiProjectId);
}

void ProjectApiClient::syncCreateElement(const QString& apiProjectId, const QJsonObject& elementData)
{
    if (!m_authManager || !m_authManager->isAuthenticated()) {
        emit syncCreateElementFailed(apiProjectId, "Not authenticated");
        return;
    }
    
    if (!m_application) {
        emit syncCreateElementFailed(apiProjectId, "Application not available");
        return;
    }

    // Find the project by its ID (which should be the API project ID for API projects)
    Project* project = m_application->getProject(apiProjectId);
    if (!project) {
        qDebug() << "ProjectApiClient::syncCreateElement - Project not found for ID:" << apiProjectId;
        emit syncCreateElementFailed(apiProjectId, "Project not found");
        return;
    }
    
    qDebug() << "ProjectApiClient::syncCreateElement - Found project:" << project->name() << "with ID:" << project->id();
    
    // Get current project data
    QJsonObject currentProjectData = m_application->serializeProjectData(project);
    
    // Debug: Check if the element is in the serialized data
    if (currentProjectData.contains("elements")) {
        QJsonArray elements = currentProjectData["elements"].toArray();
        qDebug() << "ProjectApiClient::syncCreateElement - Project has" << elements.size() << "elements in serialized data";
    } else {
        qDebug() << "ProjectApiClient::syncCreateElement - No elements array in serialized data!";
    }
    
    // Update the project via API using the project's ID (which is the API project ID)
    updateProject(apiProjectId, project->name(), currentProjectData);
    
    // Emit success immediately since updateProject handles API communication
    QString elementId = elementData["elementId"].toString();
    emit elementCreated(apiProjectId, elementId);
}

void ProjectApiClient::syncUpdateElement(const QString& apiProjectId, const QString& elementId)
{
    if (!m_authManager || !m_authManager->isAuthenticated()) {
        emit syncUpdateElementFailed(apiProjectId, elementId, "Not authenticated");
        return;
    }
    
    if (!m_application) {
        emit syncUpdateElementFailed(apiProjectId, elementId, "Application not available");
        return;
    }

    // Find the project by its ID (which should be the API project ID for API projects)
    Project* project = m_application->getProject(apiProjectId);
    if (!project) {
        emit syncUpdateElementFailed(apiProjectId, elementId, "Project not found");
        return;
    }
    
    // Get current project data
    QJsonObject currentProjectData = m_application->serializeProjectData(project);
    
    // Update the project via API using the project's ID (which is the API project ID)
    updateProject(apiProjectId, project->name(), currentProjectData);
    
    // Emit success immediately since updateProject handles API communication
    emit elementUpdated(apiProjectId, elementId);
}

void ProjectApiClient::syncMoveElements(const QString& apiProjectId, const QJsonArray& elementIds)
{
    if (!m_authManager || !m_authManager->isAuthenticated()) {
        emit syncMoveElementsFailed(apiProjectId, "Not authenticated");
        return;
    }
    
    if (!m_application) {
        emit syncMoveElementsFailed(apiProjectId, "Application not available");
        return;
    }

    // Find the project by its ID (which should be the API project ID for API projects)
    Project* project = m_application->getProject(apiProjectId);
    if (!project) {
        emit syncMoveElementsFailed(apiProjectId, "Project not found");
        return;
    }
    
    // Get current project data
    QJsonObject currentProjectData = m_application->serializeProjectData(project);
    
    // Update the project via API using the project's ID (which is the API project ID)
    updateProject(apiProjectId, project->name(), currentProjectData);
    
    // Emit success immediately since updateProject handles API communication
    emit elementsMoved(apiProjectId, elementIds);
}

void ProjectApiClient::syncDeleteElements(const QString& apiProjectId, const QJsonArray& elementIds)
{
    if (!m_authManager || !m_authManager->isAuthenticated()) {
        emit syncDeleteElementsFailed(apiProjectId, "Not authenticated");
        return;
    }
    
    if (!m_application) {
        emit syncDeleteElementsFailed(apiProjectId, "Application not available");
        return;
    }

    // Find the project by its ID (which should be the API project ID for API projects)
    Project* project = m_application->getProject(apiProjectId);
    if (!project) {
        emit syncDeleteElementsFailed(apiProjectId, "Project not found");
        return;
    }
    
    // Get current project data
    QJsonObject currentProjectData = m_application->serializeProjectData(project);
    
    // Update the project via API using the project's ID (which is the API project ID)
    updateProject(apiProjectId, project->name(), currentProjectData);
    
    // Emit success immediately since updateProject handles API communication
    emit elementsDeleted(apiProjectId, elementIds);
}

QNetworkRequest ProjectApiClient::createAuthenticatedRequest() const
{
    QNetworkRequest request{QUrl(Config::GRAPHQL_ENDPOINT)};
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
    
    if (m_authManager) {
        QString authToken = m_authManager->getIdToken();
        if (!authToken.isEmpty()) {
            request.setRawHeader("Authorization", ("Bearer " + authToken).toUtf8());
        }
    }
    
    return request;
}

void ProjectApiClient::sendGraphQLRequest(const QString& query, const QJsonObject& variables, 
                                        const QString& operation, const QString& projectId,
                                        const QString& projectName)
{
    QJsonObject body;
    body["query"] = query;
    body["variables"] = variables;

    QNetworkRequest request = createAuthenticatedRequest();
    QNetworkReply* reply = m_networkManager->post(request, QJsonDocument(body).toJson(QJsonDocument::Compact));

    // Store request details for handling response
    PendingRequest pendingRequest;
    pendingRequest.operation = operation;
    pendingRequest.projectId = projectId;
    pendingRequest.projectName = projectName;
    m_pendingRequests[reply] = pendingRequest;

    // Connect to appropriate slot based on operation
    if (operation == "createProject") {
        connect(reply, &QNetworkReply::finished, this, &ProjectApiClient::onCreateProjectFinished);
    } else if (operation == "fetchProjects") {
        connect(reply, &QNetworkReply::finished, this, &ProjectApiClient::onFetchProjectsFinished);
    } else if (operation == "listProjects") {
        connect(reply, &QNetworkReply::finished, this, &ProjectApiClient::onListProjectsFinished);
    } else if (operation == "getProject") {
        connect(reply, &QNetworkReply::finished, this, &ProjectApiClient::onGetProjectFinished);
    } else if (operation == "updateProject") {
        connect(reply, &QNetworkReply::finished, this, &ProjectApiClient::onUpdateProjectFinished);
    } else if (operation == "deleteProject") {
        connect(reply, &QNetworkReply::finished, this, &ProjectApiClient::onDeleteProjectFinished);
    }

}

void ProjectApiClient::onCreateProjectFinished()
{
    QNetworkReply* reply = qobject_cast<QNetworkReply*>(sender());
    if (!reply || !m_pendingRequests.contains(reply)) {
        return;
    }

    PendingRequest request = m_pendingRequests[reply];
    handleGraphQLResponse(reply, request);
}

void ProjectApiClient::onFetchProjectsFinished()
{
    QNetworkReply* reply = qobject_cast<QNetworkReply*>(sender());
    if (!reply || !m_pendingRequests.contains(reply)) {
        return;
    }

    PendingRequest request = m_pendingRequests[reply];
    handleGraphQLResponse(reply, request);
}

void ProjectApiClient::onListProjectsFinished()
{
    QNetworkReply* reply = qobject_cast<QNetworkReply*>(sender());
    if (!reply || !m_pendingRequests.contains(reply)) {
        return;
    }

    PendingRequest request = m_pendingRequests[reply];
    handleGraphQLResponse(reply, request);
}

void ProjectApiClient::onGetProjectFinished()
{
    QNetworkReply* reply = qobject_cast<QNetworkReply*>(sender());
    if (!reply || !m_pendingRequests.contains(reply)) {
        return;
    }

    PendingRequest request = m_pendingRequests[reply];
    handleGraphQLResponse(reply, request);
}

void ProjectApiClient::onUpdateProjectFinished()
{
    QNetworkReply* reply = qobject_cast<QNetworkReply*>(sender());
    if (!reply || !m_pendingRequests.contains(reply)) {
        return;
    }

    PendingRequest request = m_pendingRequests[reply];
    handleGraphQLResponse(reply, request);
}

void ProjectApiClient::onDeleteProjectFinished()
{
    QNetworkReply* reply = qobject_cast<QNetworkReply*>(sender());
    if (!reply || !m_pendingRequests.contains(reply)) {
        return;
    }

    PendingRequest request = m_pendingRequests[reply];
    handleGraphQLResponse(reply, request);
}

void ProjectApiClient::handleGraphQLResponse(QNetworkReply* reply, const PendingRequest& request)
{
    QByteArray responseData = reply->readAll();
    
    
    if (reply->error() == QNetworkReply::NoError) {
        QJsonDocument responseDoc = QJsonDocument::fromJson(responseData);
        QJsonObject response = responseDoc.object();
        
        // Debug: Log the parsed response structure
        // qDebug() << "ProjectApiClient: Parsed response keys:" << response.keys();
        
        if (response.contains("data")) {
            QJsonObject data = response["data"].toObject();
            
            // Debug: Log the data structure
            // qDebug() << "ProjectApiClient: Data keys:" << data.keys();
            
            if (request.operation == "createProject" && data.contains("createProject")) {
                QJsonObject createdProject = data["createProject"].toObject();
                QString apiProjectId = createdProject["id"].toString();
                emit projectCreated(apiProjectId, request.projectName);
                qDebug() << "ProjectApiClient: Successfully created project with API ID:" << apiProjectId;
                
            } else if (request.operation == "fetchProjects" && data.contains("listProjects")) {
                QJsonObject listResult = data["listProjects"].toObject();
                QJsonArray projects = listResult["items"].toArray();
                
                // Convert canvasData strings back to JSON objects
                QJsonArray processedProjects;
                for (const QJsonValue& projectValue : projects) {
                    QJsonObject project = projectValue.toObject();
                    
                    // Parse canvasData string back to JSON object
                    if (project.contains("canvasData") && project["canvasData"].isString()) {
                        QString canvasDataStr = project["canvasData"].toString();
                        QJsonDocument canvasDoc = QJsonDocument::fromJson(canvasDataStr.toUtf8());
                        if (!canvasDoc.isNull()) {
                            project["canvasData"] = canvasDoc.object();
                        }
                    }
                    
                    processedProjects.append(project);
                }
                
                emit projectsFetched(processedProjects);
                qDebug() << "ProjectApiClient: Successfully fetched" << processedProjects.size() << "projects";
                
            } else if (request.operation == "listProjects" && data.contains("listProjects")) {
                QJsonObject listResult = data["listProjects"].toObject();
                QJsonArray projects = listResult["items"].toArray();
                QString nextToken = listResult["nextToken"].toString();
                
                emit projectsListed(projects, nextToken);
                // qDebug() << "ProjectApiClient: Successfully listed" << projects.size() << "projects";
                
            } else if (request.operation == "getProject" && data.contains("getProject")) {
                QJsonObject project = data["getProject"].toObject();
                
                // Parse canvasData string back to JSON object if it's a string
                if (project.contains("canvasData") && project["canvasData"].isString()) {
                    QString canvasDataStr = project["canvasData"].toString();
                    QJsonDocument canvasDoc = QJsonDocument::fromJson(canvasDataStr.toUtf8());
                    if (!canvasDoc.isNull()) {
                        project["canvasData"] = canvasDoc.object();
                    } else {
                        emit getProjectFailed(request.projectId, "Failed to parse canvas data");
                        qDebug() << "ProjectApiClient: Failed to parse canvas data for project:" << request.projectId;
                        return;
                    }
                }
                
                emit projectFetched(request.projectId, project);
                // qDebug() << "ProjectApiClient: Successfully fetched project:" << request.projectId;
                
            } else if (request.operation == "updateProject" && data.contains("updateProject")) {
                QJsonObject updatedProject = data["updateProject"].toObject();
                QString apiProjectId = updatedProject["id"].toString();
                emit projectUpdated(apiProjectId);
                qDebug() << "ProjectApiClient: Successfully updated project:" << apiProjectId;
                
            } else if (request.operation == "deleteProject" && data.contains("deleteProject")) {
                emit projectDeleted(request.projectId);
                qDebug() << "ProjectApiClient: Successfully deleted project:" << request.projectId;
                
            } else {
                QString error = QString("API response missing expected data for operation: %1").arg(request.operation);
                qWarning() << "ProjectApiClient:" << error;
                emitErrorForOperation(request.operation, request.projectId, error);
            }
            
        } else if (response.contains("errors")) {
            QJsonArray errors = response["errors"].toArray();
            QString errorMsg = "GraphQL errors: ";
            for (const QJsonValue& error : errors) {
                errorMsg += error.toObject()["message"].toString() + "; ";
            }
            qWarning() << "ProjectApiClient: GraphQL error for" << request.operation << "-" << errorMsg;
            emitErrorForOperation(request.operation, request.projectId, errorMsg);
        }
    } else {
        QString error = QString("Network error: %1. Response: %2")
                       .arg(reply->errorString())
                       .arg(QString(responseData));
        qWarning() << "ProjectApiClient: Network error for" << request.operation << "-" << error;
        
        // Check if this is an authentication error
        if (error.contains("Token has expired") || error.contains("UnauthorizedException")) {
            // Trigger token refresh
            qDebug() << "ProjectApiClient: Token expired, triggering refresh";
            if (m_authManager) {
                m_authManager->refreshAccessToken();
            }
        }
        
        emitErrorForOperation(request.operation, request.projectId, error);
    }

    cleanupRequest(reply);
}

void ProjectApiClient::onSyncCreateElementFinished()
{
    QNetworkReply* reply = qobject_cast<QNetworkReply*>(sender());
    if (!reply || !m_pendingRequests.contains(reply)) {
        return;
    }

    PendingRequest request = m_pendingRequests[reply];
    handleGraphQLResponse(reply, request);
}

void ProjectApiClient::onSyncUpdateElementFinished()
{
    QNetworkReply* reply = qobject_cast<QNetworkReply*>(sender());
    if (!reply || !m_pendingRequests.contains(reply)) {
        return;
    }

    PendingRequest request = m_pendingRequests[reply];
    handleGraphQLResponse(reply, request);
}

void ProjectApiClient::onSyncMoveElementsFinished()
{
    QNetworkReply* reply = qobject_cast<QNetworkReply*>(sender());
    if (!reply || !m_pendingRequests.contains(reply)) {
        return;
    }

    PendingRequest request = m_pendingRequests[reply];
    handleGraphQLResponse(reply, request);
}

void ProjectApiClient::onSyncDeleteElementsFinished()
{
    QNetworkReply* reply = qobject_cast<QNetworkReply*>(sender());
    if (!reply || !m_pendingRequests.contains(reply)) {
        return;
    }

    PendingRequest request = m_pendingRequests[reply];
    handleGraphQLResponse(reply, request);
}

void ProjectApiClient::emitErrorForOperation(const QString& operation, const QString& projectId, const QString& error)
{
    if (operation == "createProject") {
        emit createProjectFailed(error);
    } else if (operation == "fetchProjects") {
        emit fetchProjectsFailed(error);
    } else if (operation == "listProjects") {
        emit listProjectsFailed(error);
    } else if (operation == "getProject") {
        emit getProjectFailed(projectId, error);
    } else if (operation == "updateProject") {
        emit updateProjectFailed(projectId, error);
    } else if (operation == "deleteProject") {
        emit deleteProjectFailed(projectId, error);
    } else if (operation == "syncCreateElement") {
        emit syncCreateElementFailed(projectId, error);
    } else if (operation == "syncUpdateElement") {
        emit syncUpdateElementFailed(projectId, "", error); // elementId would need to be stored in request
    } else if (operation == "syncMoveElements") {
        emit syncMoveElementsFailed(projectId, error);
    } else if (operation == "syncDeleteElements") {
        emit syncDeleteElementsFailed(projectId, error);
    }
}

void ProjectApiClient::cleanupRequest(QNetworkReply* reply)
{
    if (reply) {
        m_pendingRequests.remove(reply);
        reply->deleteLater();
    }
}