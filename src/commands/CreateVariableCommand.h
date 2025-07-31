#ifndef CREATEVARIABLECOMMAND_H
#define CREATEVARIABLECOMMAND_H

#include "../Command.h"
#include <QString>
#include <QPointer>

class ElementModel;
class SelectionManager;
class Variable;

class CreateVariableCommand : public Command
{
    Q_OBJECT

public:
    CreateVariableCommand(ElementModel* model, SelectionManager* selectionManager,
                         QObject *parent = nullptr);
    ~CreateVariableCommand();

    void execute() override;
    void undo() override;

private:
    void syncWithAPI();
    QPointer<ElementModel> m_elementModel;
    QPointer<SelectionManager> m_selectionManager;
    
    // Created variable
    QPointer<Variable> m_variable;
    
    // ID for the created variable
    QString m_variableId;
};

#endif // CREATEVARIABLECOMMAND_H