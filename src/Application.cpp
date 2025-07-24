#include "Application.h"
#include "Project.h"
#include "SelectionManager.h"
#include "UniqueIdGenerator.h"
#include "Element.h"
#include "CanvasElement.h"
#include "DesignElement.h"
#include "Frame.h"
#include "Text.h"
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
#include <QDebug>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QStandardPaths>
#include <QDir>
#include <QMetaObject>
#include <QQmlApplicationEngine>
#include <QQmlComponent>
#include <QQmlContext>
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
                }, Qt::UniqueConnection);
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
    auto it = std::find_if(m_canvases.begin(), m_canvases.end(),
        [&projectId](const std::unique_ptr<Project>& c) { return c->id() == projectId; });
    
    if (it != m_canvases.end()) {
        m_canvases.erase(it);
        
        emit canvasListChanged();
        emit canvasRemoved(projectId);
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

void Application::createNewProject() {
    // Use CreateProjectCommand to create and sync with API
    // Execute directly without adding to command history (project management operations don't support undo)
    auto command = std::make_unique<CreateProjectCommand>(this, "New Project");
    CreateProjectCommand* commandPtr = command.get();
    
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
}

void Application::openAPIProject(const QString& projectId, const QString& projectName, const QJsonObject& canvasData) {
    qDebug() << "Application::openAPIProject called with ID:" << projectId << "Name:" << projectName;
    
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

Project* Application::deserializeProjectFromData(const QJsonObject& projectData)
{
    return deserializeProject(projectData);
}

QJsonObject Application::serializeProjectData(Project* project) const
{
    return serializeProject(project);
}

bool Application::saveAs() {
    // Emit signal to request file save from QML
    emit saveFileRequested();
    return true;
}

bool Application::saveToFile(const QString& fileName, Project* project) {
    if (fileName.isEmpty() || !project) {
        return false;
    }
    
    QString actualFileName = fileName;
    
    // Ensure .cbt extension
    if (!actualFileName.endsWith(".cbt", Qt::CaseInsensitive)) {
        actualFileName += ".cbt";
    }
    
    try {
        // Create the main project JSON object
        QJsonObject projectData;
        projectData["version"] = "1.0";
        projectData["createdWith"] = "Cubit";
        
        // Serialize only the specified project
        QJsonArray canvasesArray;
        QJsonObject projectObj = serializeProject(project);
        canvasesArray.append(projectObj);
        projectData["canvases"] = canvasesArray;
        
        
        // Write to file
        QJsonDocument doc(projectData);
        QFile file(actualFileName);
        if (!file.open(QIODevice::WriteOnly)) {
            qDebug() << "Could not open file for writing:" << actualFileName;
            if (project && project->console()) {
                project->console()->addError(QString("Could not open file for writing: %1").arg(actualFileName));
            }
            return false;
        }
        
        file.write(doc.toJson());
        file.close();
        
        qDebug() << "Project saved successfully to:" << actualFileName;
        if (project && project->console()) {
            project->console()->addOutput(QString("Project saved to: %1").arg(actualFileName));
        }
        return true;
        
    } catch (const std::exception& e) {
        qDebug() << "Error saving project:" << e.what();
        if (project && project->console()) {
            project->console()->addError(QString("Error saving project: %1").arg(e.what()));
        }
        return false;
    }
}

QJsonObject Application::serializeProject(Project* project) const {
    if (!project) return QJsonObject();
    
    QJsonObject projectObj;
    projectObj["id"] = project->id();
    projectObj["name"] = project->name();
    projectObj["viewMode"] = project->viewMode();
    projectObj["platforms"] = QJsonArray::fromStringList(project->platforms());
    
    // Serialize all elements from the ElementModel
    QJsonArray elementsArray;
    if (project->elementModel()) {
        QList<Element*> elements = project->elementModel()->getAllElements();
        for (Element* element : elements) {
            if (element) {
                QJsonObject elementObj = serializeElement(element);
                elementsArray.append(elementObj);
            }
        }
    }
    projectObj["elements"] = elementsArray;
    
    // Serialize scripts/nodes if present
    if (project->scripts()) {
        QJsonObject scriptsObj;
        // Note: We don't save the compiled output, just the node structure
        // The Scripts class would need serialization methods for this
        projectObj["scripts"] = scriptsObj;
    }
    
    return projectObj;
}

QJsonObject Application::serializeElement(Element* element) const {
    if (!element) return QJsonObject();
    
    QJsonObject elementObj;
    
    // Basic element properties
    elementObj["elementId"] = element->getId();
    elementObj["elementType"] = element->getTypeName();
    elementObj["name"] = element->getName();
    elementObj["parentId"] = element->getParentElementId();
    
    // Use Qt's meta-object system to serialize all properties
    const QMetaObject* metaObj = element->metaObject();
    for (int i = 0; i < metaObj->propertyCount(); ++i) {
        QMetaProperty property = metaObj->property(i);
        const char* name = property.name();
        
        // Skip properties we don't want to save
        if (QString(name) == "objectName" || 
            QString(name) == "selected" ||
            QString(name) == "parentElement") {
            continue;
        }
        
        QVariant value = property.read(element);
        
        // Convert QVariant to JSON-compatible value
        if (value.canConvert<QString>()) {
            elementObj[name] = value.toString();
        } else if (value.canConvert<int>()) {
            elementObj[name] = value.toInt();
        } else if (value.canConvert<double>()) {
            elementObj[name] = value.toDouble();
        } else if (value.canConvert<bool>()) {
            elementObj[name] = value.toBool();
        } else if (value.metaType().id() == QMetaType::QColor) {
            QColor color = value.value<QColor>();
            elementObj[name] = color.name(QColor::HexArgb);
        } else if (value.metaType().id() == QMetaType::QFont) {
            QFont font = value.value<QFont>();
            QJsonObject fontObj;
            fontObj["family"] = font.family();
            fontObj["pointSize"] = font.pointSize();
            fontObj["bold"] = font.bold();
            fontObj["italic"] = font.italic();
            fontObj["weight"] = font.weight();
            elementObj[name] = fontObj;
        } else if (value.canConvert<QStringList>()) {
            QStringList list = value.toStringList();
            elementObj[name] = QJsonArray::fromStringList(list);
        }
        // Add more type conversions as needed
    }
    
    return elementObj;
}

bool Application::openFile() {
    // Emit signal to request file open from QML
    emit openFileRequested();
    return true;
}

bool Application::loadFromFile(const QString& fileName) {
    if (fileName.isEmpty()) {
        return false;
    }
    
    try {
        // Read the file
        QFile file(fileName);
        if (!file.open(QIODevice::ReadOnly)) {
            qDebug() << "Could not open file for reading:" << fileName;
            // Log error to all project consoles since we don't have a specific project yet
            for (const auto& canvas : m_canvases) {
                if (canvas && canvas->console()) {
                    canvas->console()->addError(QString("Could not open file for reading: %1").arg(fileName));
                }
            }
            return false;
        }
        
        QByteArray data = file.readAll();
        file.close();
        
        // Parse JSON
        QJsonParseError parseError;
        QJsonDocument doc = QJsonDocument::fromJson(data, &parseError);
        if (parseError.error != QJsonParseError::NoError) {
            qDebug() << "Invalid JSON format:" << parseError.errorString();
            // Log error to all project consoles since we don't have a specific project yet
            for (const auto& canvas : m_canvases) {
                if (canvas && canvas->console()) {
                    canvas->console()->addError(QString("Invalid JSON format: %1").arg(parseError.errorString()));
                }
            }
            return false;
        }
        
        QJsonObject projectData = doc.object();
        
        // Validate file format
        if (!projectData.contains("version") || !projectData.contains("canvases")) {
            qDebug() << "Invalid Cubit project file format";
            // Log error to all project consoles since we don't have a specific project yet
            for (const auto& canvas : m_canvases) {
                if (canvas && canvas->console()) {
                    canvas->console()->addError("Invalid Cubit project file format");
                }
            }
            return false;
        }
        
        // Load project data and create new projects (don't clear existing ones)
        QJsonArray canvasesArray = projectData["canvases"].toArray();
        bool success = true;
        
        for (const QJsonValue& canvasValue : canvasesArray) {
            QJsonObject canvasData = canvasValue.toObject();
            Project* newProject = deserializeProject(canvasData);
            if (newProject) {
                qDebug() << "Project loaded successfully from:" << fileName;
                if (newProject->console()) {
                    newProject->console()->addOutput(QString("Project loaded from: %1").arg(fileName));
                }
                
                // Create a new window for this project
                if (m_engine) {
                    m_engine->load(QUrl(QStringLiteral("qrc:/qml/ProjectWindow.qml")));
                }
            } else {
                success = false;
            }
        }
        
        return success;
        
    } catch (const std::exception& e) {
        qDebug() << "Error opening project:" << e.what();
        // Log error to all project consoles since we don't have a specific project
        for (const auto& canvas : m_canvases) {
            if (canvas && canvas->console()) {
                canvas->console()->addError(QString("Error opening project: %1").arg(e.what()));
            }
        }
        return false;
    }
}

bool Application::deserializeApplication(const QJsonObject& projectData) {
    try {
        // Process pending events to allow QML to handle state changes
        QCoreApplication::processEvents();
        
        // Clear existing canvases
        m_canvases.clear();
        
        // Load canvases
        QJsonArray canvasesArray = projectData["canvases"].toArray();
        
        for (const QJsonValue& canvasValue : canvasesArray) {
            QJsonObject canvasData = canvasValue.toObject();
            Project* project = deserializeProject(canvasData);
            if (project) {
                m_canvases.push_back(std::unique_ptr<Project>(project));
            }
        }
        
        // Emit signals to update UI
        emit canvasListChanged();
        
        return true;
        
    } catch (const std::exception& e) {
        qDebug() << "Error deserializing project:" << e.what();
        return false;
    }
}

Project* Application::deserializeProject(const QJsonObject& projectData) {
    try {
        QString id = projectData["id"].toString();
        QString name = projectData["name"].toString();
        
        if (id.isEmpty()) {
            return nullptr;
        }
        
        // Create new project
        Project* project = new Project(id, name, this);
        // Create and initialize project
        project->initialize();
        
        // Set project properties
        if (projectData.contains("viewMode")) {
            project->setViewMode(projectData["viewMode"].toString());
        }
        
        if (projectData.contains("platforms")) {
            QJsonArray platformsArray = projectData["platforms"].toArray();
            QStringList platforms;
            for (const QJsonValue& value : platformsArray) {
                platforms.append(value.toString());
            }
            project->setPlatforms(platforms);
        }
        
        // Load elements
        if (projectData.contains("elements")) {
            QJsonArray elementsArray = projectData["elements"].toArray();
            ElementModel* model = project->elementModel();
            
            if (!model) {
                delete project;
                return nullptr;
            }
            
            for (const QJsonValue& elementValue : elementsArray) {
                QJsonObject elementData = elementValue.toObject();
                Element* element = deserializeElement(elementData, model);
                if (element) {
                    model->addElement(element);
                }
            }
        }
        
        return project;
        
    } catch (const std::exception& e) {
        qDebug() << "Error deserializing project:" << e.what();
        return nullptr;
    }
}

Element* Application::deserializeElement(const QJsonObject& elementData, ElementModel* model) {
    try {
        QString elementId = elementData["elementId"].toString();
        QString elementType = elementData["elementType"].toString();
        QString name = elementData["name"].toString();
        QString parentId = elementData["parentId"].toString();
        
        if (elementId.isEmpty() || elementType.isEmpty()) {
            return nullptr;
        }
        
        Element* element = nullptr;
        
        // Try to create element using registry first
        ElementTypeRegistry& registry = ElementTypeRegistry::instance();
        QString lowerTypeName = elementType.toLower();
        
        if (registry.hasType(lowerTypeName)) {
            // Registry uses lowercase type names
            element = registry.createElement(lowerTypeName, elementId);
        } else {
            // Fall back to direct creation for types not in registry yet
            // (Variable, Node, Edge)
            if (elementType == "Variable") {
                element = new Variable(elementId, model);
            } else if (elementType == "Node") {
                element = new Node(elementId, model);
            } else if (elementType == "Edge") {
                element = new Edge(elementId, model);
            } else {
                qWarning() << "Unknown element type in deserialization:" << elementType;
                return nullptr;
            }
        }
        
        if (!element) {
            return nullptr;
        }
        
        // Set basic properties
        element->setName(name);
        element->setParentElementId(parentId);
        
        // Restore all other properties using Qt's meta-object system
        const QMetaObject* metaObj = element->metaObject();
        for (int i = 0; i < metaObj->propertyCount(); ++i) {
            QMetaProperty property = metaObj->property(i);
            const char* propName = property.name();
            QString propNameStr(propName);
            
            // Skip properties we don't want to restore or already handled
            if (propNameStr == "objectName" || 
                propNameStr == "selected" ||
                propNameStr == "elementId" ||
                propNameStr == "elementType" ||
                propNameStr == "name" ||
                propNameStr == "parentId" ||
                propNameStr == "parentElement") {
                continue;
            }
            
            if (elementData.contains(propName)) {
                QJsonValue jsonValue = elementData[propName];
                QVariant value;
                
                // Convert JSON value to appropriate QVariant type
                if (jsonValue.isString()) {
                    QString stringValue = jsonValue.toString();
                    
                    // Try to convert numeric strings to proper types
                    bool isNumber;
                    double numberValue = stringValue.toDouble(&isNumber);
                    if (isNumber && (property.metaType().id() == QMetaType::Double || 
                                    property.metaType().id() == QMetaType::Float ||
                                    property.metaType().id() == QMetaType::Int)) {
                        if (property.metaType().id() == QMetaType::Int) {
                            value = QVariant(static_cast<int>(numberValue));
                        } else {
                            value = QVariant(numberValue);
                        }
                    } else if (stringValue == "true" || stringValue == "false") {
                        // Convert boolean strings
                        value = QVariant(stringValue == "true");
                    } else {
                        value = stringValue;
                    }
                } else if (jsonValue.isDouble()) {
                    value = jsonValue.toDouble();
                } else if (jsonValue.isBool()) {
                    value = jsonValue.toBool();
                } else if (jsonValue.isObject()) {
                    // Handle complex types like fonts
                    QJsonObject obj = jsonValue.toObject();
                    if (obj.contains("family")) {
                        // It's a font
                        QFont font;
                        font.setFamily(obj["family"].toString());
                        font.setPointSize(obj["pointSize"].toInt());
                        font.setBold(obj["bold"].toBool());
                        font.setItalic(obj["italic"].toBool());
                        font.setWeight(QFont::Weight(obj["weight"].toInt()));
                        value = font;
                    }
                } else if (jsonValue.isArray()) {
                    // Handle string lists
                    QJsonArray array = jsonValue.toArray();
                    QStringList list;
                    for (const QJsonValue& val : array) {
                        list.append(val.toString());
                    }
                    value = list;
                }
                
                // Handle color strings (hex format)
                if (value.metaType().id() == QMetaType::QString && propNameStr.contains("color", Qt::CaseInsensitive)) {
                    QString colorStr = value.toString();
                    if (colorStr.startsWith("#")) {
                        QColor color(colorStr);
                        if (color.isValid()) {
                            value = color;
                        }
                    }
                }
                
                // Set the property if the value is valid and writable
                if (value.isValid() && property.isWritable()) {
                    property.write(element, value);
                }
            }
        }
        
        return element;
        
    } catch (const std::exception& e) {
        qDebug() << "Error deserializing element:" << e.what();
        return nullptr;
    }
}

