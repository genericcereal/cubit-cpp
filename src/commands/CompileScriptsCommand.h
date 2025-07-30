#ifndef COMPILESCRIPTSCOMMAND_H
#define COMPILESCRIPTSCOMMAND_H

#include "../Command.h"
#include <QString>
#include <QPointer>

class Scripts;
class ElementModel;
class Project;
class ConsoleMessageRepository;

class CompileScriptsCommand : public Command
{
    Q_OBJECT

public:
    CompileScriptsCommand(Scripts* scripts, ElementModel* elementModel, 
                         ConsoleMessageRepository* console, Project* project,
                         QObject *parent = nullptr);
    ~CompileScriptsCommand();

    void execute() override;
    void undo() override;
    
    // Get the compilation result
    QString compiledScript() const { return m_compiledScript; }
    bool wasSuccessful() const { return m_wasSuccessful; }

private:
    void syncWithApi();
    
    QPointer<Scripts> m_scripts;
    QPointer<ElementModel> m_elementModel;
    QPointer<ConsoleMessageRepository> m_console;
    QPointer<Project> m_project;
    
    QString m_compiledScript;
    QString m_previousCompiledScript;
    bool m_wasSuccessful;
    bool m_previousIsCompiled;
};

#endif // COMPILESCRIPTSCOMMAND_H