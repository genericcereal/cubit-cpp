#ifndef CREATECOMPONENTCOMMAND_H
#define CREATECOMPONENTCOMMAND_H

#include "../Command.h"
#include <QString>

class ElementModel;
class DesignElement;

class CreateComponentCommand : public Command
{
    Q_OBJECT

public:
    CreateComponentCommand(ElementModel* elementModel, DesignElement* sourceElement,
                          QObject *parent = nullptr);
    ~CreateComponentCommand();

    void execute() override;
    void undo() override;
    void redo() override;

    DesignElement* getCreatedInstance() const { return m_createdInstance; }

private:
    ElementModel* m_elementModel;
    DesignElement* m_sourceElement;
    DesignElement* m_createdInstance;
    QString m_sourceElementId;
    QString m_sourceParentId;
    QString m_instanceId;
};

#endif // CREATECOMPONENTCOMMAND_H