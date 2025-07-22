#include "SetPropertyCommand.h"
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