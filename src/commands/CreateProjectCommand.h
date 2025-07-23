#ifndef CREATEPROJECTCOMMAND_H
#define CREATEPROJECTCOMMAND_H

#include "../Command.h"
#include <QString>
#include <QPointer>

class Application;
class ProjectApiClient;

class CreateProjectCommand : public Command
{
    Q_OBJECT

public:
    CreateProjectCommand(Application* application, const QString& projectName = QString(),
                        QObject *parent = nullptr);
    ~CreateProjectCommand();

    void execute() override;
    void undo() override;

    QString getCreatedProjectId() const { return m_projectId; }

private slots:
    void onApiProjectCreated(const QString& apiProjectId, const QString& name);
    void onApiCreateFailed(const QString& error);

private:
    void syncWithAPI();
    
    QPointer<Application> m_application;
    QString m_projectName;
    QString m_projectId;
    QString m_apiProjectId;  // Store the API project ID
};

#endif // CREATEPROJECTCOMMAND_H