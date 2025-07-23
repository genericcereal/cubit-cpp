#ifndef DELETEPROJECTCOMMAND_H
#define DELETEPROJECTCOMMAND_H

#include "../Command.h"
#include <QString>
#include <QPointer>
#include <QJsonObject>

class Application;
class Project;

class DeleteProjectCommand : public Command
{
    Q_OBJECT

public:
    DeleteProjectCommand(Application* application, const QString& projectId,
                        QObject *parent = nullptr);
    ~DeleteProjectCommand();

    void execute() override;
    void undo() override;

    QString getDeletedProjectId() const { return m_projectId; }

private:
    QPointer<Application> m_application;
    QString m_projectId;
    QString m_projectName;
    QJsonObject m_projectData;  // Serialized project data for restoration
    bool m_projectDataCaptured;
};

#endif // DELETEPROJECTCOMMAND_H