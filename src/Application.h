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
class StreamingAIClient;
class AuthenticationManager;
class QQmlApplicationEngine;

class Application : public QObject {
    Q_OBJECT
    Q_PROPERTY(QString activeCanvasId READ activeCanvasId WRITE setActiveCanvasId NOTIFY activeCanvasIdChanged)
    Q_PROPERTY(Project* activeCanvas READ activeCanvas NOTIFY activeCanvasChanged)
    Q_PROPERTY(QStringList canvasIds READ canvasIds NOTIFY canvasListChanged)
    Q_PROPERTY(QStringList canvasNames READ canvasNames NOTIFY canvasListChanged)
    Q_PROPERTY(Panels* panels READ panels CONSTANT)

public:
    explicit Application(QObject *parent = nullptr);
    ~Application();
    
    void setAuthenticationManager(AuthenticationManager* authManager);

    static Application* instance();

    // Canvas management
    Q_INVOKABLE QString createCanvas(const QString& name = QString());
    Q_INVOKABLE void removeCanvas(const QString& canvasId);
    Q_INVOKABLE void switchToCanvas(const QString& canvasId);
    Q_INVOKABLE QString getCanvasName(const QString& canvasId) const;
    Q_INVOKABLE void renameCanvas(const QString& canvasId, const QString& newName);
    Q_INVOKABLE Project* getCanvas(const QString& canvasId);
    Q_INVOKABLE void createNewProject();
    
    // Engine management for multi-window support
    void setEngine(QQmlApplicationEngine* engine);

    // Property getters
    QString activeCanvasId() const;
    Project* activeCanvas() const;
    QStringList canvasIds() const;
    QStringList canvasNames() const;
    Panels* panels() const;

    // Property setters
    void setActiveCanvasId(const QString& canvasId);

    // File operations
    Q_INVOKABLE bool saveAs();
    Q_INVOKABLE bool openFile();
    Q_INVOKABLE bool saveToFile(const QString& fileName);
    Q_INVOKABLE bool loadFromFile(const QString& fileName);
    

signals:
    void activeCanvasIdChanged();
    void activeCanvasChanged();
    void canvasListChanged();
    void canvasCreated(const QString& canvasId);
    void canvasRemoved(const QString& canvasId);
    void saveFileRequested();
    void openFileRequested();

private slots:
    void onAICommandReceived(const QString& prompt);
    void onAIContinuationResponse(bool accepted, const QString& feedback);
    void onAIModeDisabled();

private:
    static Application* s_instance;
    std::vector<std::unique_ptr<Project>> m_canvases;
    QString m_activeCanvasId;
    std::unique_ptr<Panels> m_panels;
    std::unique_ptr<StreamingAIClient> m_streamingAIClient;
    AuthenticationManager* m_authManager = nullptr;
    QQmlApplicationEngine* m_engine = nullptr;
    
    Project* findCanvas(const QString& canvasId);
    const Project* findCanvas(const QString& canvasId) const;
    QString generateCanvasId() const;
    
    // Serialization methods
    QJsonObject serializeCanvas(Project* canvas) const;
    QJsonObject serializeElement(Element* element) const;
    
    // Deserialization methods
    bool deserializeProject(const QJsonObject& projectData);
    Project* deserializeCanvas(const QJsonObject& canvasData);
    Element* deserializeElement(const QJsonObject& elementData, ElementModel* model);
};

#endif // APPLICATION_H