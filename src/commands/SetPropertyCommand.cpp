#include "SetPropertyCommand.h"
#include "../Element.h"
#include "../Application.h"
#include "../Project.h"
#include "../ProjectApiClient.h"
#include "../ElementModel.h"
#include <QObject>
#include <QMetaProperty>
#include <QDebug>

SetPropertyCommand::SetPropertyCommand(QObject* target, const QString& propertyName,
                                       const QVariant& oldValue, const QVariant& newValue,
                                       QObject *parent)
    : Command(parent)
    , m_target(target)
    , m_propertyName(propertyName)
    , m_oldValue(oldValue)
    , m_newValue(newValue)
    , m_firstExecute(true)
{
    QString targetName = target ? target->objectName() : "Unknown";
    if (targetName.isEmpty()) {
        targetName = target->metaObject()->className();
    }
    
    setDescription(QString("Change %1.%2")
                   .arg(targetName)
                   .arg(propertyName));
}

SetPropertyCommand::~SetPropertyCommand()
{
}

void SetPropertyCommand::execute()
{
    if (!m_target) return;

    // Always set the property to the new value
    m_target->setProperty(m_propertyName.toUtf8().constData(), m_newValue);
    m_firstExecute = false;
    
    // Log to console
    QString targetName = m_target ? m_target->objectName() : "Unknown";
    if (targetName.isEmpty() && m_target) {
        targetName = m_target->metaObject()->className();
    }
    
    qDebug() << QString("Set property %1.%2 from '%3' to '%4'")
                .arg(targetName)
                .arg(m_propertyName)
                .arg(m_oldValue.toString())
                .arg(m_newValue.toString());
    
    // Sync with API after the property change
    syncWithAPI();
}

void SetPropertyCommand::undo()
{
    if (!m_target) return;

    // Set property back to old value
    m_target->setProperty(m_propertyName.toUtf8().constData(), m_oldValue);

}

bool SetPropertyCommand::mergeWith(SetPropertyCommand* other)
{
    if (!other) return false;

    // Can only merge if we're changing the same property on the same object
    if (m_target != other->m_target || m_propertyName != other->m_propertyName) {
        return false;
    }

    // Update the new value to the latest change
    m_newValue = other->m_newValue;

    return true;
}

void SetPropertyCommand::syncWithAPI()
{
    if (!m_target) {
        return;
    }

    // Only sync if the target is an Element
    Element* element = qobject_cast<Element*>(m_target);
    if (!element) {
        return;
    }

    // Navigate up to find the project (through ElementModel)
    ElementModel* model = nullptr;
    QObject* parent = element->parent();
    while (parent) {
        model = qobject_cast<ElementModel*>(parent);
        if (model) break;
        parent = parent->parent();
    }

    if (!model) {
        return;
    }

    Project* project = qobject_cast<Project*>(model->parent());
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
    
    // Sync with API
    apiClient->syncUpdateElement(apiProjectId, element->getId());
    
    qDebug() << "SetPropertyCommand: Syncing property change with API for project" << apiProjectId 
             << "element:" << element->getId() << "property:" << m_propertyName << "new value:" << m_newValue;
}