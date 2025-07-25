#include "FileManager.h"
#include "Application.h"
#include "Project.h"
#include "Serializer.h"
#include "ConsoleMessageRepository.h"
#include <QFile>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QDebug>
#include <QQmlApplicationEngine>

FileManager::FileManager(Application* app, QObject *parent)
    : QObject(parent)
    , m_application(app)
{
}

bool FileManager::saveAs() {
    // Emit signal to request file save from QML
    emit saveFileRequested();
    return true;
}

bool FileManager::saveToFile(const QString& fileName, Project* project) {
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
        Serializer* serializer = m_application->serializer();
        QJsonObject projectObj = serializer->serializeProject(project);
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

bool FileManager::openFile() {
    // Emit signal to request file open from QML
    emit openFileRequested();
    return true;
}

bool FileManager::loadFromFile(const QString& fileName) {
    if (fileName.isEmpty()) {
        return false;
    }
    
    try {
        // Read the file
        QFile file(fileName);
        if (!file.open(QIODevice::ReadOnly)) {
            qDebug() << "Could not open file for reading:" << fileName;
            // Log error to all project consoles since we don't have a specific project yet
            const auto& canvases = m_application->canvases();
            for (const auto& canvas : canvases) {
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
            const auto& canvases = m_application->canvases();
            for (const auto& canvas : canvases) {
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
            const auto& canvases = m_application->canvases();
            for (const auto& canvas : canvases) {
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
            Serializer* serializer = m_application->serializer();
            Project* newProject = serializer->deserializeProject(canvasData);
            if (newProject) {
                qDebug() << "Project loaded successfully from:" << fileName;
                if (newProject->console()) {
                    newProject->console()->addOutput(QString("Project loaded from: %1").arg(fileName));
                }
                
                // Create a new window for this project
                QQmlApplicationEngine* engine = m_application->qmlEngine();
                if (engine) {
                    engine->load(QUrl(QStringLiteral("qrc:/qml/ProjectWindow.qml")));
                }
            } else {
                success = false;
            }
        }
        
        return success;
        
    } catch (const std::exception& e) {
        qDebug() << "Error opening project:" << e.what();
        // Log error to all project consoles since we don't have a specific project
        const auto& canvases = m_application->canvases();
        for (const auto& canvas : canvases) {
            if (canvas && canvas->console()) {
                canvas->console()->addError(QString("Error opening project: %1").arg(e.what()));
            }
        }
        return false;
    }
}