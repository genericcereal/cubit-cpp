#include "Serializer.h"
#include "Application.h"
#include "Project.h"
#include "Element.h"
#include "ElementModel.h"
#include "CanvasElement.h"
#include "DesignElement.h"
#include "Frame.h"
#include "Text.h"
#include "Shape.h"
#include "Variable.h"
#include "ScriptElement.h"
#include "Node.h"
#include "Edge.h"
#include "Scripts.h"
#include "platforms/web/WebTextInput.h"
#include "PlatformConfig.h"
#include "ElementTypeRegistry.h"
#include "ConsoleMessageRepository.h"
#include "CanvasController.h"
#include "HitTestService.h"
#include "VariableBinding.h"
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QJsonValue>
#include <QMetaObject>
#include <QMetaProperty>
#include <QDebug>
#include <QFont>
#include <QColor>

Serializer::Serializer(Application* app, QObject *parent)
    : QObject(parent)
    , m_application(app)
{
}

QJsonObject Serializer::serializeProject(Project* project) const {
    if (!project) return QJsonObject();
    
    QJsonObject projectObj;
    projectObj["id"] = project->id();
    projectObj["name"] = project->name();
    // Don't serialize viewMode - it's app state, not persistent data
    // Serialize platforms with their scripts
    QJsonArray platformsArray;
    QList<PlatformConfig*> platforms = project->getAllPlatforms();
    for (PlatformConfig* platform : platforms) {
        QJsonObject platformObj;
        platformObj["name"] = platform->name();
        
        // Serialize platform-specific scripts
        if (platform->scripts()) {
            QJsonObject scriptsObj;
            
            // Serialize nodes
            QJsonArray nodesArray;
            QList<Node*> nodes = platform->scripts()->getAllNodes();
            for (Node* node : nodes) {
                QJsonObject nodeObj = serializeNode(node);
                nodesArray.append(nodeObj);
            }
            scriptsObj["nodes"] = nodesArray;
            
            // Serialize edges
            QJsonArray edgesArray;
            QList<Edge*> edges = platform->scripts()->getAllEdges();
            for (Edge* edge : edges) {
                QJsonObject edgeObj = serializeEdge(edge);
                edgesArray.append(edgeObj);
            }
            scriptsObj["edges"] = edgesArray;
            
            platformObj["scripts"] = scriptsObj;
        } else {
        }
        
        // Serialize platform-specific global elements
        if (platform->globalElements()) {
            QJsonArray globalElementsArray;
            QList<Element*> globalElements = platform->globalElements()->getAllElements();
            for (Element* element : globalElements) {
                if (element) {
                    QJsonObject elementObj = serializeElement(element);
                    globalElementsArray.append(elementObj);
                }
            }
            platformObj["globalElements"] = globalElementsArray;
        }
        
        platformsArray.append(platformObj);
    }
    projectObj["platforms"] = platformsArray;
    
    // Serialize main project scripts
    if (project->scripts()) {
        QJsonObject mainScriptsObj;
        
        // Serialize nodes
        QJsonArray nodesArray;
        QList<Node*> nodes = project->scripts()->getAllNodes();
        for (Node* node : nodes) {
            QJsonObject nodeObj = serializeNode(node);
            nodesArray.append(nodeObj);
        }
        mainScriptsObj["nodes"] = nodesArray;
        
        // Serialize edges
        QJsonArray edgesArray;
        QList<Edge*> edges = project->scripts()->getAllEdges();
        for (Edge* edge : edges) {
            QJsonObject edgeObj = serializeEdge(edge);
            edgesArray.append(edgeObj);
        }
        mainScriptsObj["edges"] = edgesArray;
        
        projectObj["scripts"] = mainScriptsObj;
    }
    
    // Serialize all elements from the ElementModel (excluding globalElements)
    QJsonArray elementsArray;
    if (project->elementModel()) {
        QList<Element*> elements = project->elementModel()->getAllElements();
        for (Element* element : elements) {
            if (element) {
                // Skip globalElements (they have role == appContainer and are serialized with their platform)
                if (Frame* frame = qobject_cast<Frame*>(element)) {
                    if (frame->role() == Frame::appContainer) {
                        continue;
                    }
                }
                QJsonObject elementObj = serializeElement(element);
                elementsArray.append(elementObj);
            }
        }
    } else {
        qWarning() << "Serializer::serializeProject - Project has no ElementModel!";
    }
    
    projectObj["elements"] = elementsArray;
    
    // Serialize variable bindings
    if (project->bindingManager()) {
        QVariantList bindings = project->bindingManager()->serialize();
        projectObj["variableBindings"] = QJsonArray::fromVariantList(bindings);
    }
    
    // Add metadata
    projectObj["createdAt"] = project->property("createdAt").toString();
    projectObj["lastModified"] = QDateTime::currentDateTime().toString(Qt::ISODate);
    
    return projectObj;
}

QJsonObject Serializer::serializeElement(Element* element) const {
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
        // IMPORTANT: Check specific types BEFORE generic conversions
        if (value.metaType().id() == QMetaType::QString) {
            // Handle QString directly to avoid conversion issues
            elementObj[name] = value.toString();
        } else if (value.metaType().id() == QMetaType::QVariantList || value.canConvert<QVariantList>()) {
            // Handle QVariantList FIRST before string conversion
            QVariantList list = value.toList();
            QJsonArray jsonArray;
            
            for (int idx = 0; idx < list.size(); ++idx) {
                const QVariant& item = list[idx];
                if (item.canConvert<QVariantMap>()) {
                    QVariantMap map = item.toMap();
                    
                    QJsonObject jsonObj;
                    for (auto it = map.begin(); it != map.end(); ++it) {
                        QVariant val = it.value();
                        
                        // Force conversion to double for numeric values
                        bool ok = false;
                        double dblVal = val.toDouble(&ok);
                        
                        if (ok) {
                            jsonObj[it.key()] = dblVal;
                        } else if (val.canConvert<QString>()) {
                            jsonObj[it.key()] = val.toString();
                        } else if (val.canConvert<bool>()) {
                            jsonObj[it.key()] = val.toBool();
                        } else {
                            // Last resort - force to double
                            jsonObj[it.key()] = val.toDouble();
                        }
                    }
                    
                    jsonArray.append(jsonObj);
                } else if (item.canConvert<QString>()) {
                    jsonArray.append(item.toString());
                } else if (item.canConvert<double>()) {
                    jsonArray.append(item.toDouble());
                } else if (item.canConvert<int>()) {
                    jsonArray.append(item.toInt());
                } else if (item.canConvert<bool>()) {
                    jsonArray.append(item.toBool());
                }
            }
            elementObj[name] = jsonArray;
        } else if (value.canConvert<QColor>()) {
            QColor color = value.value<QColor>();
            elementObj[name] = color.name(QColor::HexArgb);
        } else if (value.canConvert<QFont>()) {
            QFont font = value.value<QFont>();
            QJsonObject fontObj;
            fontObj["family"] = font.family();
            fontObj["pointSize"] = font.pointSize();
            fontObj["bold"] = font.bold();
            fontObj["italic"] = font.italic();
            fontObj["weight"] = font.weight();
            elementObj[name] = fontObj;
        } else if (value.canConvert<QString>()) {
            elementObj[name] = value.toString();
        } else if (value.canConvert<double>()) {
            elementObj[name] = value.toDouble();
        } else if (value.canConvert<int>()) {
            elementObj[name] = value.toInt();
        } else if (value.canConvert<bool>()) {
            elementObj[name] = value.toBool();
        } else if (value.canConvert<QStringList>()) {
            QStringList list = value.toStringList();
            elementObj[name] = QJsonArray::fromStringList(list);
        } else {
            // Handle enums and other types as strings
            int enumIndex = metaObj->indexOfEnumerator(property.typeName());
            if (enumIndex != -1) {
                QMetaEnum metaEnum = metaObj->enumerator(enumIndex);
                elementObj[name] = QString(metaEnum.valueToKey(value.toInt()));
            } else {
                // Fallback: convert to string
                elementObj[name] = value.toString();
            }
        }
    }
    
    return elementObj;
}

Project* Serializer::deserializeProject(const QJsonObject& projectData) {
    try {
        QString id = projectData["id"].toString();
        QString name = projectData["name"].toString();
        
        if (id.isEmpty()) {
            return nullptr;
        }
        
        // Create new project
        Project* project = new Project(id, name, m_application);
        // Create and initialize project
        project->initialize();
        
        // Don't restore viewMode from serialized data - it's app state
        // The project will use its default viewMode ("design")
        
        // Deserialize main project scripts
        if (projectData.contains("scripts")) {
            QJsonObject scriptsObj = projectData["scripts"].toObject();
            Scripts* mainScripts = project->scripts();
            
            if (mainScripts) {
                // Clear existing default nodes
                mainScripts->clear();
                
                // Load nodes
                if (scriptsObj.contains("nodes")) {
                    QJsonArray nodesArray = scriptsObj["nodes"].toArray();
                    for (const QJsonValue& nodeValue : nodesArray) {
                        Node* node = deserializeNode(nodeValue.toObject(), mainScripts);
                        if (node) {
                            mainScripts->addNode(node);
                            // Don't add to ElementModel here - let ScriptCanvasContext handle it
                        }
                    }
                }
                
                // Load edges
                if (scriptsObj.contains("edges")) {
                    QJsonArray edgesArray = scriptsObj["edges"].toArray();
                    for (const QJsonValue& edgeValue : edgesArray) {
                        Edge* edge = deserializeEdge(edgeValue.toObject(), mainScripts);
                        if (edge) {
                            mainScripts->addEdge(edge);
                            // Don't add to ElementModel here - let ScriptCanvasContext handle it
                        }
                    }
                }
            }
        }
        
        if (projectData.contains("platforms")) {
            QJsonArray platformsArray = projectData["platforms"].toArray();
            for (const QJsonValue& platformValue : platformsArray) {
                if (platformValue.isString()) {
                    // Legacy format: just platform names
                    project->addPlatform(platformValue.toString());
                } else if (platformValue.isObject()) {
                    // New format: platform objects with scripts
                    QJsonObject platformObj = platformValue.toObject();
                    QString platformName = platformObj["name"].toString();
                    
                    if (!platformName.isEmpty()) {
                        project->addPlatform(platformName);
                        
                        // Load platform-specific scripts if present
                        if (platformObj.contains("scripts")) {
                            QJsonObject scriptsObj = platformObj["scripts"].toObject();
                            Scripts* platformScripts = project->getPlatformScripts(platformName);
                            
                            if (platformScripts) {
                                // Clear existing default nodes
                                platformScripts->clear();
                                
                                // Load nodes
                                if (scriptsObj.contains("nodes")) {
                                    QJsonArray nodesArray = scriptsObj["nodes"].toArray();
                                    for (const QJsonValue& nodeValue : nodesArray) {
                                        Node* node = deserializeNode(nodeValue.toObject(), platformScripts);
                                        if (node) {
                                            platformScripts->addNode(node);
                                            // Don't add to ElementModel here - let ScriptCanvasContext handle it
                                        }
                                    }
                                }
                                
                                // Load edges
                                if (scriptsObj.contains("edges")) {
                                    QJsonArray edgesArray = scriptsObj["edges"].toArray();
                                    for (const QJsonValue& edgeValue : edgesArray) {
                                        Edge* edge = deserializeEdge(edgeValue.toObject(), platformScripts);
                                        if (edge) {
                                            platformScripts->addEdge(edge);
                                            // Don't add to ElementModel here - let ScriptCanvasContext handle it
                                        }
                                    }
                                }
                                
                            } else {
                                qWarning() << "  Failed to get scripts for platform:" << platformName;
                            }
                        } else {
                        }
                        
                        // Load platform-specific global elements if present
                        if (platformObj.contains("globalElements")) {
                            QJsonArray globalElementsArray = platformObj["globalElements"].toArray();
                            
                            // Get the platform's global elements model
                            PlatformConfig* platformConfig = project->getPlatform(platformName);
                            if (platformConfig && platformConfig->globalElements()) {
                                ElementModel* globalElementsModel = platformConfig->globalElements();
                                
                                // Clear existing global elements (except the default ones created on platform init)
                                globalElementsModel->clear();
                                
                                for (const QJsonValue& elementValue : globalElementsArray) {
                                    QJsonObject elementData = elementValue.toObject();
                                    Element* element = deserializeElement(elementData, globalElementsModel);
                                    if (element) {
                                        globalElementsModel->addElement(element);
                                    }
                                }
                                
                                // Resolve parent relationships for global elements after loading
                                globalElementsModel->resolveParentRelationships();
                            } else {
                                qWarning() << "  Failed to get global elements model for platform:" << platformName;
                            }
                        } else {
                        }
                    }
                }
            }
        }
        
        // Add to application's project list
        m_application->addCanvas(project);
        
        // Load elements after canvas is added
        if (projectData.contains("elements")) {
            QJsonArray elementsArray = projectData["elements"].toArray();
            for (const QJsonValue& elementValue : elementsArray) {
                QJsonObject elementData = elementValue.toObject();
                Element* element = deserializeElement(elementData, project->elementModel());
                if (element) {
                    project->elementModel()->addElement(element);
                }
            }
        }
        
        // Resolve parent relationships after all elements are loaded
        if (project->elementModel()) {
            project->elementModel()->resolveParentRelationships();
        }
        
        // Deserialize variable bindings after all elements are loaded
        if (projectData.contains("variableBindings") && project->bindingManager()) {
            QJsonArray bindingsArray = projectData["variableBindings"].toArray();
            QVariantList bindingsList;
            for (const QJsonValue& value : bindingsArray) {
                bindingsList.append(value.toObject().toVariantMap());
            }
            project->bindingManager()->deserialize(bindingsList);
        }
        
        // Connect property sync for global elements
        QList<PlatformConfig*> platforms = project->getAllPlatforms();
        for (PlatformConfig* platform : platforms) {
            if (platform && platform->globalElements() && project->elementModel()) {
                // Connect property sync for all global elements
                platform->connectAllGlobalElementsPropertySync(project->elementModel());
            }
        }
        
        // Rebuild spatial index after all elements are loaded
        CanvasController* controller = project->controller();
        if (controller && controller->hitTestService()) {
            controller->hitTestService()->rebuildSpatialIndex();
        }
        
        return project;
        
    } catch (const std::exception& e) {
        return nullptr;
    }
}

Element* Serializer::deserializeElement(const QJsonObject& elementData, ElementModel* model) {
    try {
        // Handle case where strings are stored as character arrays
        QString elementId;
        QString elementType;
        QString name;
        QString parentId;
        
        auto getStringFromJsonValue = [](const QJsonValue& value) -> QString {
            if (value.isString()) {
                return value.toString();
            } else if (value.isArray()) {
                // Handle case where string is stored as array of characters
                QJsonArray arr = value.toArray();
                QString result;
                for (const QJsonValue& val : arr) {
                    result += val.toString();
                }
                return result;
            }
            return QString();
        };
        
        elementId = getStringFromJsonValue(elementData["elementId"]);
        elementType = getStringFromJsonValue(elementData["elementType"]);
        name = getStringFromJsonValue(elementData["name"]);
        parentId = getStringFromJsonValue(elementData["parentId"]);
        
        
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
            if (!element) {
            }
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
        
        // For Shape elements, we need to set shapeType and joints before geometry to avoid losing custom joints
        if (elementType == "Shape") {
            if (elementData.contains("shapeType")) {
                QString shapeTypeStr = getStringFromJsonValue(elementData["shapeType"]);
                if (Shape* shape = qobject_cast<Shape*>(element)) {
                    if (shapeTypeStr == "Square") {
                        shape->setShapeType(Shape::Square);
                    } else if (shapeTypeStr == "Triangle") {
                        shape->setShapeType(Shape::Triangle);
                    } else if (shapeTypeStr == "Line") {
                        shape->setShapeType(Shape::Line);
                    }
                }
            }
            
            // Load joints immediately after shape type and before geometry
            if (elementData.contains("joints")) {
                QJsonArray jointsArray = elementData["joints"].toArray();
                QVariantList variantJoints;
                for (const QJsonValue& jointValue : jointsArray) {
                    if (jointValue.isObject()) {
                        QJsonObject jointObj = jointValue.toObject();
                        QVariantMap jointMap;
                        jointMap["x"] = jointObj["x"].toDouble();
                        jointMap["y"] = jointObj["y"].toDouble();
                        
                        // Load new properties if they exist (for backward compatibility)
                        if (jointObj.contains("mirroring")) {
                            jointMap["mirroring"] = jointObj["mirroring"].toInt();
                        } else {
                            jointMap["mirroring"] = 0; // NoMirroring
                        }
                        
                        if (jointObj.contains("cornerRadius")) {
                            jointMap["cornerRadius"] = jointObj["cornerRadius"].toDouble();
                        } else {
                            jointMap["cornerRadius"] = 0.0;
                        }
                        
                        variantJoints.append(jointMap);
                    }
                }
                if (Shape* shape = qobject_cast<Shape*>(element)) {
                    shape->setJoints(variantJoints);
                }
            }
        }
        
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
                propNameStr == "parentElement" ||
                (elementType == "Shape" && propNameStr == "shapeType") ||  // Already handled above
                (elementType == "Shape" && propNameStr == "joints")) {     // Already handled above
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
                    QJsonArray array = jsonValue.toArray();
                    
                    // Check if property expects QVariantList (like joints)
                    if (property.metaType().id() == QMetaType::QVariantList) {
                        QVariantList list;
                        for (const QJsonValue& val : array) {
                            if (val.isObject()) {
                                // Convert JSON object to QVariantMap
                                QJsonObject obj = val.toObject();
                                QVariantMap map;
                                for (auto it = obj.begin(); it != obj.end(); ++it) {
                                    if (it.value().isDouble()) {
                                        map[it.key()] = it.value().toDouble();
                                    } else if (it.value().isString()) {
                                        map[it.key()] = it.value().toString();
                                    } else if (it.value().isBool()) {
                                        map[it.key()] = it.value().toBool();
                                    } // integers will be handled as doubles
                                }
                                list.append(map);
                            } else if (val.isString()) {
                                list.append(val.toString());
                            } else if (val.isDouble()) {
                                list.append(val.toDouble());
                            } else if (val.isBool()) {
                                list.append(val.toBool());
                            }
                        }
                        value = list;
                    } else {
                        // Handle as string lists for backwards compatibility
                        QStringList list;
                        for (const QJsonValue& val : array) {
                            list.append(val.toString());
                        }
                        value = list;
                    }
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
        return nullptr;
    }
}

QJsonObject Serializer::serializeNode(Node* node) const {
    QJsonObject nodeObj;
    
    // Serialize basic properties from Element base class
    nodeObj["id"] = node->getId();
    nodeObj["name"] = node->getName();
    nodeObj["x"] = node->x();
    nodeObj["y"] = node->y();
    nodeObj["width"] = node->width();
    nodeObj["height"] = node->height();
    
    // Serialize Node-specific properties
    nodeObj["nodeType"] = node->nodeType();
    nodeObj["nodeTitle"] = node->nodeTitle();
    nodeObj["nodeColor"] = node->nodeColor().name();
    nodeObj["value"] = node->value();
    nodeObj["sourceElementId"] = node->sourceElementId();
    
    // Serialize ports
    QJsonArray inputPortsArray = QJsonArray::fromStringList(node->inputPorts());
    nodeObj["inputPorts"] = inputPortsArray;
    
    QJsonArray outputPortsArray = QJsonArray::fromStringList(node->outputPorts());
    nodeObj["outputPorts"] = outputPortsArray;
    
    // Serialize isAsync property
    nodeObj["isAsync"] = node->isAsync();
    
    // Serialize row configurations
    QJsonArray rowsArray;
    QVariantList rows = node->rowConfigurations();
    for (const QVariant& rowVariant : rows) {
        QVariantMap rowMap = rowVariant.toMap();
        QJsonObject rowObj;
        
        if (rowMap.contains("hasTarget") && rowMap["hasTarget"].toBool()) {
            rowObj["hasTarget"] = true;
            rowObj["targetLabel"] = rowMap["targetLabel"].toString();
            rowObj["targetType"] = rowMap["targetType"].toString();
            rowObj["targetPortIndex"] = rowMap["targetPortIndex"].toInt();
        }
        
        if (rowMap.contains("hasSource") && rowMap["hasSource"].toBool()) {
            rowObj["hasSource"] = true;
            rowObj["sourceLabel"] = rowMap["sourceLabel"].toString();
            rowObj["sourceType"] = rowMap["sourceType"].toString();
            rowObj["sourcePortIndex"] = rowMap["sourcePortIndex"].toInt();
        }
        
        rowsArray.append(rowObj);
    }
    nodeObj["rowConfigurations"] = rowsArray;
    
    return nodeObj;
}

QJsonObject Serializer::serializeEdge(Edge* edge) const {
    QJsonObject edgeObj;
    
    // Serialize basic properties from Element base class
    edgeObj["id"] = edge->getId();
    edgeObj["name"] = edge->getName();
    
    // Serialize Edge-specific properties
    edgeObj["sourceNodeId"] = edge->sourceNodeId();
    edgeObj["targetNodeId"] = edge->targetNodeId();
    edgeObj["sourcePortIndex"] = edge->sourcePortIndex();
    edgeObj["targetPortIndex"] = edge->targetPortIndex();
    edgeObj["sourcePortType"] = edge->sourcePortType();
    edgeObj["targetPortType"] = edge->targetPortType();
    edgeObj["edgeColor"] = edge->edgeColor().name();
    edgeObj["edgeWidth"] = edge->edgeWidth();
    
    // Serialize points
    QJsonObject sourcePoint;
    sourcePoint["x"] = edge->sourcePoint().x();
    sourcePoint["y"] = edge->sourcePoint().y();
    edgeObj["sourcePoint"] = sourcePoint;
    
    QJsonObject targetPoint;
    targetPoint["x"] = edge->targetPoint().x();
    targetPoint["y"] = edge->targetPoint().y();
    edgeObj["targetPoint"] = targetPoint;
    
    return edgeObj;
}

Node* Serializer::deserializeNode(const QJsonObject& nodeData, Scripts* scripts) {
    if (!scripts) return nullptr;
    
    QString id = nodeData["id"].toString();
    if (id.isEmpty()) return nullptr;
    
    Node* node = new Node(id, scripts);
    
    // Set basic properties from Element base class
    if (nodeData.contains("name")) node->setName(nodeData["name"].toString());
    if (nodeData.contains("x")) node->setX(nodeData["x"].toDouble());
    if (nodeData.contains("y")) node->setY(nodeData["y"].toDouble());
    if (nodeData.contains("width")) node->setWidth(nodeData["width"].toDouble());
    if (nodeData.contains("height")) node->setHeight(nodeData["height"].toDouble());
    
    // Set Node-specific properties
    if (nodeData.contains("nodeType")) node->setNodeType(nodeData["nodeType"].toString());
    if (nodeData.contains("nodeTitle")) node->setNodeTitle(nodeData["nodeTitle"].toString());
    if (nodeData.contains("nodeColor")) node->setNodeColor(QColor(nodeData["nodeColor"].toString()));
    if (nodeData.contains("value")) node->setValue(nodeData["value"].toString());
    if (nodeData.contains("sourceElementId")) node->setSourceElementId(nodeData["sourceElementId"].toString());
    
    // Set ports
    if (nodeData.contains("inputPorts")) {
        QJsonArray inputPortsArray = nodeData["inputPorts"].toArray();
        QStringList inputPorts;
        for (const QJsonValue& port : inputPortsArray) {
            inputPorts << port.toString();
        }
        node->setInputPorts(inputPorts);
    }
    
    if (nodeData.contains("outputPorts")) {
        QJsonArray outputPortsArray = nodeData["outputPorts"].toArray();
        QStringList outputPorts;
        for (const QJsonValue& port : outputPortsArray) {
            outputPorts << port.toString();
        }
        node->setOutputPorts(outputPorts);
    }
    
    // Set isAsync property
    if (nodeData.contains("isAsync")) {
        node->setIsAsync(nodeData["isAsync"].toBool());
    }
    
    // Set row configurations
    if (nodeData.contains("rowConfigurations")) {
        QJsonArray rowsArray = nodeData["rowConfigurations"].toArray();
        for (const QJsonValue& rowValue : rowsArray) {
            QJsonObject rowObj = rowValue.toObject();
            Node::RowConfig config;
            
            if (rowObj.contains("hasTarget") && rowObj["hasTarget"].toBool()) {
                config.hasTarget = true;
                config.targetLabel = rowObj["targetLabel"].toString();
                config.targetType = rowObj["targetType"].toString();
                config.targetPortIndex = rowObj["targetPortIndex"].toInt();
            }
            
            if (rowObj.contains("hasSource") && rowObj["hasSource"].toBool()) {
                config.hasSource = true;
                config.sourceLabel = rowObj["sourceLabel"].toString();
                config.sourceType = rowObj["sourceType"].toString();
                config.sourcePortIndex = rowObj["sourcePortIndex"].toInt();
            }
            
            node->addRow(config);
        }
    }
    
    return node;
}

Edge* Serializer::deserializeEdge(const QJsonObject& edgeData, Scripts* scripts) {
    if (!scripts) return nullptr;
    
    QString id = edgeData["id"].toString();
    if (id.isEmpty()) return nullptr;
    
    Edge* edge = new Edge(id, scripts);
    
    // Set basic properties from Element base class
    if (edgeData.contains("name")) edge->setName(edgeData["name"].toString());
    
    // Set Edge-specific properties
    if (edgeData.contains("sourceNodeId")) edge->setSourceNodeId(edgeData["sourceNodeId"].toString());
    if (edgeData.contains("targetNodeId")) edge->setTargetNodeId(edgeData["targetNodeId"].toString());
    if (edgeData.contains("sourcePortIndex")) edge->setSourcePortIndex(edgeData["sourcePortIndex"].toInt());
    if (edgeData.contains("targetPortIndex")) edge->setTargetPortIndex(edgeData["targetPortIndex"].toInt());
    if (edgeData.contains("sourcePortType")) edge->setSourcePortType(edgeData["sourcePortType"].toString());
    if (edgeData.contains("targetPortType")) edge->setTargetPortType(edgeData["targetPortType"].toString());
    if (edgeData.contains("edgeColor")) edge->setEdgeColor(QColor(edgeData["edgeColor"].toString()));
    if (edgeData.contains("edgeWidth")) edge->setEdgeWidth(edgeData["edgeWidth"].toDouble());
    
    // Set points
    if (edgeData.contains("sourcePoint")) {
        QJsonObject sourcePoint = edgeData["sourcePoint"].toObject();
        edge->setSourcePoint(QPointF(sourcePoint["x"].toDouble(), sourcePoint["y"].toDouble()));
    }
    
    if (edgeData.contains("targetPoint")) {
        QJsonObject targetPoint = edgeData["targetPoint"].toObject();
        edge->setTargetPoint(QPointF(targetPoint["x"].toDouble(), targetPoint["y"].toDouble()));
    }
    
    return edge;
}