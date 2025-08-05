#include "CreateVariableCommand.h"
#include "../Variable.h"
#include "../ElementModel.h"
#include "../SelectionManager.h"
#include "../UniqueIdGenerator.h"
#include "../Project.h"
#include "../PlatformConfig.h"
#include "../Application.h"
#include "../ProjectApiClient.h"
#include <QDebug>
#include <QJsonObject>

CreateVariableCommand::CreateVariableCommand(ElementModel* model, SelectionManager* selectionManager,
                                           QObject *parent)
    : Command(parent)
    , m_elementModel(model)
    , m_selectionManager(selectionManager)
{
    setDescription("Create Variable");
}

CreateVariableCommand::~CreateVariableCommand()
{
    // If the command owns the variable (not executed), delete it
    if (m_variable && !isExecuted()) {
        m_variable->deleteLater();
    }
}

void CreateVariableCommand::execute()
{
    if (!m_elementModel) return;

    // Get the Project from the ElementModel's parent
    Project* project = qobject_cast<Project*>(m_elementModel->parent());
    if (!project) {
        qWarning() << "CreateVariableCommand: ElementModel has no Project parent";
        return;
    }


    // Create variable on first execution
    if (!m_variable) {
        m_variableId = m_elementModel->generateId();
        m_variable = new Variable(m_variableId);
        
        if (!m_variable) {
            qWarning() << "Failed to create variable";
            return;
        }
        
        // Set a default name for the variable using only first 4 digits of ID
        QString shortId = m_variableId.left(4);
        m_variable->setName("Variable " + shortId);
    }
    
    // Add to model
    m_elementModel->addElement(m_variable);
    
    // Select the newly created variable
    if (m_selectionManager) {
        m_selectionManager->selectOnly(m_variable);
    }
    
    // Sync with API
    syncWithAPI();
}

void CreateVariableCommand::undo()
{
    if (!m_variable || !m_elementModel) return;
    
    // Get the Project from the ElementModel's parent
    Project* project = qobject_cast<Project*>(m_elementModel->parent());
    if (!project) return;
    
    
    // Clear selection if this variable is selected
    if (m_selectionManager && m_selectionManager->isSelected(m_variable)) {
        m_selectionManager->clearSelection();
    }
    
    // Remove from model
    m_elementModel->removeElement(m_variable);
}

void CreateVariableCommand::syncWithAPI()
{
    if (!m_elementModel || !m_variable) {
        return;
    }

    // Get the Project from the ElementModel's parent
    Project* project = qobject_cast<Project*>(m_elementModel->parent());
    if (!project) {
        return;
    }

    // Get the Application instance and its API client
    Application* app = Application::instance();
    if (!app) {
        return;
    }

    ProjectApiClient* apiClient = app->projectApiClient();
    if (!apiClient) {
        return;
    }

    // Get the project's API ID
    QString apiProjectId = project->id();
    
    // Serialize the variable data
    QJsonObject variableData = app->serializeElement(m_variable);
    
    // Sync with API
    apiClient->syncCreateElement(apiProjectId, variableData);
}