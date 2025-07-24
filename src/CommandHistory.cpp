#include "CommandHistory.h"
#include "Command.h"
#include <QDebug>

CommandHistory::CommandHistory(QObject *parent)
    : QObject(parent)
    , m_maxUndoCount(100)
{
}

CommandHistory::~CommandHistory()
{
}

void CommandHistory::execute(std::unique_ptr<Command> command)
{
    if (!command) {
        return;
    }

    QString description = command->description();
    qDebug() << "CommandHistory::execute() - executing command:" << description;

    command->execute();
    command->setExecuted(true);

    m_undoStack.push(std::move(command));
    
    qDebug() << "CommandHistory::execute() - command added to undo stack. Stack size:" << m_undoStack.size();
    
    // Clear redo stack
    while (!m_redoStack.empty()) {
        m_redoStack.pop();
    }

    limitUndoStack();
    updateCanUndoRedo();

    emit commandExecuted(description);
}

void CommandHistory::undo()
{
    qDebug() << "CommandHistory::undo() called. Can undo:" << canUndo() << "Stack size:" << m_undoStack.size();
    
    if (!canUndo()) {
        qDebug() << "CommandHistory::undo() - cannot undo, returning";
        return;
    }

    std::unique_ptr<Command> command = std::move(const_cast<std::unique_ptr<Command>&>(m_undoStack.top()));
    m_undoStack.pop();
    
    qDebug() << "CommandHistory::undo() - undoing command:" << command->description();

    command->undo();
    command->setExecuted(false);

    m_redoStack.push(std::move(command));
    updateCanUndoRedo();
    
    qDebug() << "CommandHistory::undo() completed. New stack size:" << m_undoStack.size();
}

void CommandHistory::redo()
{
    if (!canRedo()) {
        return;
    }

    std::unique_ptr<Command> command = std::move(const_cast<std::unique_ptr<Command>&>(m_redoStack.top()));
    m_redoStack.pop();

    command->redo();
    command->setExecuted(true);

    m_undoStack.push(std::move(command));
    updateCanUndoRedo();
}

bool CommandHistory::canUndo() const
{
    return !m_undoStack.empty();
}

bool CommandHistory::canRedo() const
{
    return !m_redoStack.empty();
}

QString CommandHistory::undoDescription() const
{
    if (canUndo()) {
        return m_undoStack.top()->description();
    }
    return QString();
}

QString CommandHistory::redoDescription() const
{
    if (canRedo()) {
        return m_redoStack.top()->description();
    }
    return QString();
}

void CommandHistory::clear()
{
    while (!m_undoStack.empty()) {
        m_undoStack.pop();
    }
    while (!m_redoStack.empty()) {
        m_redoStack.pop();
    }
    updateCanUndoRedo();
}

int CommandHistory::undoStackSize() const
{
    return m_undoStack.size();
}

int CommandHistory::redoStackSize() const
{
    return m_redoStack.size();
}

void CommandHistory::setMaxUndoCount(int count)
{
    m_maxUndoCount = qMax(1, count);
    limitUndoStack();
}

int CommandHistory::maxUndoCount() const
{
    return m_maxUndoCount;
}

void CommandHistory::updateCanUndoRedo()
{
    emit canUndoChanged(canUndo());
    emit canRedoChanged(canRedo());
}

void CommandHistory::limitUndoStack()
{
    // For std::stack, we need to reverse, limit, then restore
    if (m_undoStack.size() > static_cast<size_t>(m_maxUndoCount)) {
        std::stack<std::unique_ptr<Command>> temp;
        
        // Keep only the most recent m_maxUndoCount commands
        int toKeep = m_maxUndoCount;
        while (!m_undoStack.empty() && toKeep > 0) {
            temp.push(std::move(const_cast<std::unique_ptr<Command>&>(m_undoStack.top())));
            m_undoStack.pop();
            toKeep--;
        }
        
        // Clear remaining old commands
        while (!m_undoStack.empty()) {
            m_undoStack.pop();
        }
        
        // Restore the kept commands
        while (!temp.empty()) {
            m_undoStack.push(std::move(const_cast<std::unique_ptr<Command>&>(temp.top())));
            temp.pop();
        }
    }
}