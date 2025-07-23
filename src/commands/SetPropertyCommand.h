#ifndef SETPROPERTYCOMMAND_H
#define SETPROPERTYCOMMAND_H

#include "../Command.h"
#include <QVariant>
#include <QString>

class QObject;

class SetPropertyCommand : public Command
{
    Q_OBJECT

public:
    SetPropertyCommand(QObject* target, const QString& propertyName, 
                       const QVariant& oldValue, const QVariant& newValue, 
                       QObject *parent = nullptr);
    ~SetPropertyCommand();

    void execute() override;
    void undo() override;

    // Merge with another property change command if possible
    bool mergeWith(SetPropertyCommand* other);

private:
    void syncWithAPI();
    QObject* m_target;
    QString m_propertyName;
    QVariant m_oldValue;
    QVariant m_newValue;
    bool m_firstExecute;
};

#endif // SETPROPERTYCOMMAND_H