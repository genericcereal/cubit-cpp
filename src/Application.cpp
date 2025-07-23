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
#include "StreamingAIClient.h"
#include "ConsoleMessageRepository.h"
#include "AuthenticationManager.h"
#include "ElementTypeRegistry.h"
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
    
    // Connect to console repository for AI commands
    connect(ConsoleMessageRepository::instance(), &ConsoleMessageRepository::aiCommandReceived,
            this, &Application::onAICommandReceived, Qt::UniqueConnection);
    
    // Connect to console repository for AI continuation responses
    connect(ConsoleMessageRepository::instance(), &ConsoleMessageRepository::aiContinuationResponse,
            this, &Application::onAIContinuationResponse, Qt::UniqueConnection);
    
    // Connect to console repository for AI mode disabled
    connect(ConsoleMessageRepository::instance(), &ConsoleMessageRepository::aiModeDisabled,
            this, &Application::onAIModeDisabled, Qt::UniqueConnection);
    
    // Don't create initial canvas - let user start from ProjectList
    // createCanvas("Canvas 1");
    
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
    
    // Create Streaming AI client now that we have auth manager
    m_streamingAIClient = std::make_unique<StreamingAIClient>(m_authManager, this, this);
}

void Application::setEngine(QQmlApplicationEngine* engine) {
    m_engine = engine;
}

QString Application::createCanvas(const QString& name) {
    QString canvasId = generateCanvasId();
    QString canvasName = name.isEmpty() ? QString("Canvas %1").arg(m_canvases.size() + 1) : name;
    
    auto canvas = std::make_unique<Project>(canvasId, canvasName, this);
    canvas->initialize();
    
    m_canvases.push_back(std::move(canvas));
    
    // Don't automatically set as active - let windows manage their own canvases
    
    emit canvasListChanged();
    emit canvasCreated(canvasId);
    
    return canvasId;
}

void Application::removeCanvas(const QString& canvasId) {
    auto it = std::find_if(m_canvases.begin(), m_canvases.end(),
        [&canvasId](const std::unique_ptr<Project>& c) { return c->id() == canvasId; });
    
    if (it != m_canvases.end()) {
        bool wasActive = (m_activeCanvasId == canvasId);
        
        m_canvases.erase(it);
        
        emit canvasListChanged();
        emit canvasRemoved(canvasId);
        
        // If the removed canvas was active, switch to another
        if (wasActive) {
            if (!m_canvases.empty()) {
                setActiveCanvasId(m_canvases.front()->id());
            } else {
                m_activeCanvasId.clear();
                emit activeCanvasIdChanged();
                emit activeCanvasChanged();
            }
        }
    }
}

void Application::switchToCanvas(const QString& canvasId) {
    setActiveCanvasId(canvasId);
}

QString Application::getCanvasName(const QString& canvasId) const {
    const Project* canvas = findCanvas(canvasId);
    return canvas ? canvas->name() : QString();
}

void Application::renameCanvas(const QString& canvasId, const QString& newName) {
    Project* canvas = findCanvas(canvasId);
    if (canvas && !newName.isEmpty()) {
        canvas->setName(newName);
        emit canvasListChanged();
    }
}

Project* Application::getCanvas(const QString& canvasId) {
    return findCanvas(canvasId);
}

void Application::createNewProject() {
    // Create a new canvas with a default name
    QString newCanvasId = createCanvas("New Project");
    
    // Don't switch the active canvas - each window manages its own canvas
    
    // Create a new window for this project
    if (m_engine) {
        QQmlComponent component(m_engine, QUrl(QStringLiteral("qrc:/qml/ProjectWindow.qml")));
        if (component.isReady()) {
            QObject* window = component.create();
            if (window) {
                // Set the canvas ID property on the window
                window->setProperty("canvasId", newCanvasId);
                
                // The window will show itself due to visible: true in QML
                qDebug() << "Created new project window for canvas:" << newCanvasId;
            } else {
                qWarning() << "Failed to create project window:" << component.errorString();
            }
        } else {
            qWarning() << "ProjectWindow component not ready:" << component.errorString();
        }
    } else {
        qWarning() << "QML engine not set, cannot create new window";
    }
}

QString Application::activeCanvasId() const {
    return m_activeCanvasId;
}

Project* Application::activeCanvas() const {
    return const_cast<Project*>(findCanvas(m_activeCanvasId));
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

void Application::setActiveCanvasId(const QString& canvasId) {
    if (m_activeCanvasId != canvasId && findCanvas(canvasId)) {
        m_activeCanvasId = canvasId;
        
        // Clear selection when switching canvases
        Project* canvas = findCanvas(canvasId);
        if (canvas && canvas->selectionManager()) {
            canvas->selectionManager()->clearSelection();
        }
        
        emit activeCanvasIdChanged();
        emit activeCanvasChanged();
    }
}


Project* Application::findCanvas(const QString& canvasId) {
    auto it = std::find_if(m_canvases.begin(), m_canvases.end(),
        [&canvasId](const std::unique_ptr<Project>& c) { return c->id() == canvasId; });
    return (it != m_canvases.end()) ? it->get() : nullptr;
}

const Project* Application::findCanvas(const QString& canvasId) const {
    auto it = std::find_if(m_canvases.begin(), m_canvases.end(),
        [&canvasId](const std::unique_ptr<Project>& c) { return c->id() == canvasId; });
    return (it != m_canvases.end()) ? it->get() : nullptr;
}

QString Application::generateCanvasId() const {
    return UniqueIdGenerator::generate16DigitId();
}


Panels* Application::panels() const {
    if (!m_panels) {
        qWarning() << "Application::panels() called but m_panels is null!";
        return nullptr;
    }
    return m_panels.get();
}

bool Application::saveAs() {
    // Emit signal to request file save from QML
    emit saveFileRequested();
    return true;
}

bool Application::saveToFile(const QString& fileName) {
    if (fileName.isEmpty()) {
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
        
        // Serialize all canvases
        QJsonArray canvasesArray;
        for (const auto& canvas : m_canvases) {
            QJsonObject canvasObj = serializeCanvas(canvas.get());
            canvasesArray.append(canvasObj);
        }
        projectData["canvases"] = canvasesArray;
        
        // Store active canvas ID
        projectData["activeCanvasId"] = m_activeCanvasId;
        
        // Write to file
        QJsonDocument doc(projectData);
        QFile file(actualFileName);
        if (!file.open(QIODevice::WriteOnly)) {
            qDebug() << "Could not open file for writing:" << actualFileName;
            ConsoleMessageRepository::instance()->addError(QString("Could not open file for writing: %1").arg(actualFileName));
            return false;
        }
        
        file.write(doc.toJson());
        file.close();
        
        qDebug() << "Project saved successfully to:" << actualFileName;
        ConsoleMessageRepository::instance()->addOutput(QString("Project saved to: %1").arg(actualFileName));
        return true;
        
    } catch (const std::exception& e) {
        qDebug() << "Error saving project:" << e.what();
        ConsoleMessageRepository::instance()->addError(QString("Error saving project: %1").arg(e.what()));
        return false;
    }
}

QJsonObject Application::serializeCanvas(Project* canvas) const {
    if (!canvas) return QJsonObject();
    
    QJsonObject canvasObj;
    canvasObj["id"] = canvas->id();
    canvasObj["name"] = canvas->name();
    canvasObj["viewMode"] = canvas->viewMode();
    canvasObj["platforms"] = QJsonArray::fromStringList(canvas->platforms());
    
    // Serialize all elements from the ElementModel
    QJsonArray elementsArray;
    if (canvas->elementModel()) {
        QList<Element*> elements = canvas->elementModel()->getAllElements();
        for (Element* element : elements) {
            if (element) {
                QJsonObject elementObj = serializeElement(element);
                elementsArray.append(elementObj);
            }
        }
    }
    canvasObj["elements"] = elementsArray;
    
    // Serialize scripts/nodes if present
    if (canvas->scripts()) {
        QJsonObject scriptsObj;
        // Note: We don't save the compiled output, just the node structure
        // The Scripts class would need serialization methods for this
        canvasObj["scripts"] = scriptsObj;
    }
    
    return canvasObj;
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
            ConsoleMessageRepository::instance()->addError(QString("Could not open file for reading: %1").arg(fileName));
            return false;
        }
        
        QByteArray data = file.readAll();
        file.close();
        
        // Parse JSON
        QJsonParseError parseError;
        QJsonDocument doc = QJsonDocument::fromJson(data, &parseError);
        if (parseError.error != QJsonParseError::NoError) {
            qDebug() << "Invalid JSON format:" << parseError.errorString();
            ConsoleMessageRepository::instance()->addError(QString("Invalid JSON format: %1").arg(parseError.errorString()));
            return false;
        }
        
        QJsonObject projectData = doc.object();
        
        // Validate file format
        if (!projectData.contains("version") || !projectData.contains("canvases")) {
            qDebug() << "Invalid Cubit project file format";
            ConsoleMessageRepository::instance()->addError("Invalid Cubit project file format");
            return false;
        }
        
        // Clear existing project and load new one
        bool success = deserializeProject(projectData);
        if (success) {
            qDebug() << "Project loaded successfully from:" << fileName;
            ConsoleMessageRepository::instance()->addOutput(QString("Project loaded from: %1").arg(fileName));
        }
        
        return success;
        
    } catch (const std::exception& e) {
        qDebug() << "Error opening project:" << e.what();
        ConsoleMessageRepository::instance()->addError(QString("Error opening project: %1").arg(e.what()));
        return false;
    }
}

bool Application::deserializeProject(const QJsonObject& projectData) {
    try {
        // Clear active canvas first to disconnect QML bindings safely
        m_activeCanvasId.clear();
        emit activeCanvasIdChanged();
        emit activeCanvasChanged();
        
        // Process pending events to allow QML to handle the null canvas state
        QCoreApplication::processEvents();
        
        // Clear existing canvases
        m_canvases.clear();
        
        // Load canvases
        QJsonArray canvasesArray = projectData["canvases"].toArray();
        QString activeCanvasId = projectData["activeCanvasId"].toString();
        
        for (const QJsonValue& canvasValue : canvasesArray) {
            QJsonObject canvasData = canvasValue.toObject();
            Project* canvas = deserializeCanvas(canvasData);
            if (canvas) {
                m_canvases.push_back(std::unique_ptr<Project>(canvas));
            }
        }
        
        // Set active canvas
        if (!activeCanvasId.isEmpty() && findCanvas(activeCanvasId)) {
            setActiveCanvasId(activeCanvasId);
        } else if (!m_canvases.empty()) {
            setActiveCanvasId(m_canvases.front()->id());
        }
        
        // Emit signals to update UI
        emit canvasListChanged();
        emit activeCanvasChanged();
        
        return true;
        
    } catch (const std::exception& e) {
        qDebug() << "Error deserializing project:" << e.what();
        return false;
    }
}

Project* Application::deserializeCanvas(const QJsonObject& canvasData) {
    try {
        QString id = canvasData["id"].toString();
        QString name = canvasData["name"].toString();
        
        if (id.isEmpty()) {
            return nullptr;
        }
        
        // Create new canvas
        Project* canvas = new Project(id, name, this);
        canvas->initialize();
        
        // Set canvas properties
        if (canvasData.contains("viewMode")) {
            canvas->setViewMode(canvasData["viewMode"].toString());
        }
        
        if (canvasData.contains("platforms")) {
            QJsonArray platformsArray = canvasData["platforms"].toArray();
            QStringList platforms;
            for (const QJsonValue& value : platformsArray) {
                platforms.append(value.toString());
            }
            canvas->setPlatforms(platforms);
        }
        
        // Load elements
        if (canvasData.contains("elements")) {
            QJsonArray elementsArray = canvasData["elements"].toArray();
            ElementModel* model = canvas->elementModel();
            
            if (!model) {
                delete canvas;
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
        
        return canvas;
        
    } catch (const std::exception& e) {
        qDebug() << "Error deserializing canvas:" << e.what();
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

void Application::onAICommandReceived(const QString& prompt) {
    // Check if we have authentication
    if (!m_authManager || !m_authManager->isAuthenticated()) {
        ConsoleMessageRepository::instance()->addError("You must be authenticated to use the AI assistant. Please log in first.");
        return;
    }
    
    // Check if Streaming AI client is available
    if (!m_streamingAIClient) {
        ConsoleMessageRepository::instance()->addError("AI client is not initialized.");
        return;
    }
    
    // Send the prompt to the AI
    m_streamingAIClient->sendMessage(prompt);
}

void Application::onAIContinuationResponse(bool accepted, const QString& feedback) {
    // Check if Streaming AI client is available
    if (!m_streamingAIClient) {
        ConsoleMessageRepository::instance()->addError("AI client is not initialized.");
        return;
    }
    
    // Forward the response to the AI client
    m_streamingAIClient->handleUserContinuationResponse(accepted, feedback);
}

void Application::onAIModeDisabled() {
    // Clear the AI conversation when AI mode is disabled
    if (m_streamingAIClient) {
        m_streamingAIClient->clearConversation();
    }
}