#ifndef COMMAND_H
#define COMMAND_H

#include <QObject>
#include <QString>

class Command : public QObject
{
    Q_OBJECT

public:
    explicit Command(QObject *parent = nullptr);
    virtual ~Command();

    virtual void execute() = 0;
    virtual void undo() = 0;
    virtual void redo();

    QString description() const;
    bool isExecuted() const;

protected:
    void setDescription(const QString& desc);

private:
    friend class CommandHistory;
    void setExecuted(bool executed);

private:
    QString m_description;
    bool m_executed;
};

#endif // COMMAND_H