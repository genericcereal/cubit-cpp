#ifndef ADDPLATFORMCOMMAND_H
#define ADDPLATFORMCOMMAND_H

#include "../Command.h"
#include <QString>
#include <QPointer>

class Project;

class AddPlatformCommand : public Command
{
    Q_OBJECT

public:
    AddPlatformCommand(Project* project, const QString& platformName, QObject *parent = nullptr);
    ~AddPlatformCommand();

    void execute() override;
    void undo() override;

private:
    void syncWithAPI();
    
    QPointer<Project> m_project;
    QString m_platformName;
};

#endif // ADDPLATFORMCOMMAND_H