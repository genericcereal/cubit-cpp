#include "CompileScriptsCommand.h"
#include "../Scripts.h"
#include "../ElementModel.h"
#include "../Project.h"
#include "../ConsoleMessageRepository.h"
#include "../Application.h"
#include "../ProjectApiClient.h"
#include <QJsonObject>
#include <QDateTime>
#include <QDebug>

CompileScriptsCommand::CompileScriptsCommand(Scripts* scripts, ElementModel* elementModel,
                                           ConsoleMessageRepository* console, Project* project,
                                           QObject *parent)
    : Command(parent)
    , m_scripts(scripts)
    , m_elementModel(elementModel)
    , m_console(console)
    , m_project(project)
    , m_wasSuccessful(false)
    , m_previousIsCompiled(false)
{
    setDescription("Compile Scripts");
}

CompileScriptsCommand::~CompileScriptsCommand()
{
}

void CompileScriptsCommand::execute()
{
    if (!m_scripts || !m_elementModel) {
        if (m_console) {
            m_console->addError("Cannot compile: Scripts or ElementModel is null");
        }
        return;
    }

    // Store previous state for undo
    m_previousCompiledScript = m_scripts->compiledScript();
    m_previousIsCompiled = m_scripts->isCompiled();

    // Compile the scripts using the Scripts::compile method which includes debug output
    m_compiledScript = m_scripts->compile(m_elementModel, m_console);
    
    if (m_compiledScript.isEmpty()) {
        // Error already handled by Scripts::compile
        m_wasSuccessful = false;
        return;
    }
    
    // Scripts::compile already sets isCompiled and compiledScript
    m_wasSuccessful = true;
    
    if (m_console) {
        m_console->addOutput("Scripts compiled successfully");
    }
    
    // Sync with API after successful compilation
    syncWithApi();
}

void CompileScriptsCommand::undo()
{
    if (!m_scripts) return;

    // Restore previous compiled state
    m_scripts->setCompiledScript(m_previousCompiledScript);
    m_scripts->setIsCompiled(m_previousIsCompiled);
    
    if (m_console) {
        m_console->addInfo("Script compilation undone");
    }
    
    // Note: We don't undo the API sync as that would require storing
    // the previous server state, which could be complex
}

void CompileScriptsCommand::syncWithApi()
{
    if (!m_project || !m_wasSuccessful) return;
    
    // Get the Application instance
    Application* app = qobject_cast<Application*>(m_project->parent());
    if (!app) return;
    
    // Get the ProjectApiClient
    ProjectApiClient* apiClient = app->projectApiClient();
    if (!apiClient) return;
    
    // Use the project's ID as the API project ID
    QString apiProjectId = m_project->id();
    
    // The compilation has changed the state of the scripts
    // We need to sync the entire project to ensure consistency
    QJsonObject compilationData;
    compilationData["action"] = "scripts_compiled";
    compilationData["timestamp"] = QDateTime::currentDateTime().toString(Qt::ISODate);
    
    // Call syncCreateElement which internally updates the entire project
    // This ensures all nodes, edges, and the compiled script are synced
    apiClient->syncCreateElement(apiProjectId, compilationData);
    
    // qDebug() << "CompileScriptsCommand: Syncing compiled scripts with API for project:" << apiProjectId;
}