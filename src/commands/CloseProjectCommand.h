#ifndef CLOSEPROJECTCOMMAND_H
#define CLOSEPROJECTCOMMAND_H

#include "../Command.h"
#include <QString>
#include <QPointer>
#include <QJsonObject>

class Application;
class ProjectApiClient;

class CloseProjectCommand : public Command
{
    Q_OBJECT

public:
    CloseProjectCommand(Application* application, const QString& projectId,
                       QObject *parent = nullptr);
    ~CloseProjectCommand();

    void execute() override;
    void undo() override;

signals:
    void closeComplete();

private slots:
    void onApiSyncComplete();
    void onApiSyncFailed(const QString& error);
    void performClose();

private:
    QPointer<Application> m_application;
    QString m_projectId;
    QJsonObject m_savedProjectData;  // Store project state for undo
    bool m_syncInProgress;
};

#endif // CLOSEPROJECTCOMMAND_H