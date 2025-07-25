#ifndef CREATEPROJECTCOMMAND_H
#define CREATEPROJECTCOMMAND_H

#include "../Command.h"
#include <QString>
#include <QPointer>
#include <QJsonObject>

class Application;
class ProjectApiClient;

class CreateProjectCommand : public Command
{
    Q_OBJECT

public:
    CreateProjectCommand(Application* application, const QString& projectName = QString(),
                        const QJsonObject& initialCanvasData = QJsonObject(),
                        QObject *parent = nullptr);
    ~CreateProjectCommand();

    void execute() override;
    void undo() override;

    QString getCreatedProjectId() const { return m_projectId; }
    void setCreatedProjectId(const QString& projectId) { m_projectId = projectId; }
    void syncWithAPI();

signals:
    void apiSyncComplete();

private slots:
    void onApiProjectCreated(const QString& apiProjectId, const QString& name);
    void onApiCreateFailed(const QString& error);

private:
    
    QPointer<Application> m_application;
    QString m_projectName;
    QString m_projectId;
    QString m_apiProjectId;  // Store the API project ID
    QJsonObject m_initialCanvasData;  // Optional initial canvas data
};

#endif // CREATEPROJECTCOMMAND_H