#ifndef COMMANDHISTORY_H
#define COMMANDHISTORY_H

#include <QObject>
#include <memory>
#include <stack>

class Command;

class CommandHistory : public QObject
{
    Q_OBJECT

public:
    explicit CommandHistory(QObject *parent = nullptr);
    ~CommandHistory();

    void execute(std::unique_ptr<Command> command);
    void undo();
    void redo();

    bool canUndo() const;
    bool canRedo() const;

    QString undoDescription() const;
    QString redoDescription() const;

    void clear();
    int undoStackSize() const;
    int redoStackSize() const;

    void setMaxUndoCount(int count);
    int maxUndoCount() const;

signals:
    void canUndoChanged(bool canUndo);
    void canRedoChanged(bool canRedo);
    void commandExecuted(const QString& description);

private:
    void updateCanUndoRedo();
    void limitUndoStack();

    std::stack<std::unique_ptr<Command>> m_undoStack;
    std::stack<std::unique_ptr<Command>> m_redoStack;
    int m_maxUndoCount;
};

#endif // COMMANDHISTORY_H