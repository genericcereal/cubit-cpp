#ifndef OPENPROJECTCOMMAND_H
#define OPENPROJECTCOMMAND_H

#include "../Command.h"
#include <QString>
#include <QPointer>
#include <QJsonObject>

class Application;
class Project;

class OpenProjectCommand : public Command
{
    Q_OBJECT

public:
    OpenProjectCommand(Application* application, 
                      const QString& projectId, 
                      const QString& projectName, 
                      const QJsonObject& canvasData = QJsonObject(),
                      QObject *parent = nullptr);
    ~OpenProjectCommand();

    void execute() override;
    void undo() override;

    QString getOpenedProjectId() const { return m_projectId; }

private:
    QPointer<Application> m_application;
    QString m_projectId;
    QString m_projectName;
    QJsonObject m_canvasData;
    bool m_projectWasCreated;
};

#endif // OPENPROJECTCOMMAND_H