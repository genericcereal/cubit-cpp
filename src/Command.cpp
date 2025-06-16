#include "Command.h"

Command::Command(QObject *parent)
    : QObject(parent)
    , m_executed(false)
{
}

Command::~Command()
{
}

void Command::redo()
{
    execute();
}

QString Command::description() const
{
    return m_description;
}

bool Command::isExecuted() const
{
    return m_executed;
}

void Command::setDescription(const QString& desc)
{
    m_description = desc;
}

void Command::setExecuted(bool executed)
{
    m_executed = executed;
}