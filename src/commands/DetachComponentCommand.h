#ifndef DETACHCOMPONENTCOMMAND_H
#define DETACHCOMPONENTCOMMAND_H

#include "Command.h"
#include <QString>

class DesignElement;
class SelectionManager;

class DetachComponentCommand : public Command
{
public:
    DetachComponentCommand(DesignElement* element, SelectionManager* selectionManager);
    ~DetachComponentCommand() override = default;
    
    void execute() override;
    void undo() override;
    
private:
    DesignElement* m_element;
    SelectionManager* m_selectionManager;
    QString m_originalInstanceOf;
    QString m_originalName;
    QString m_newName;
};

#endif // DETACHCOMPONENTCOMMAND_H