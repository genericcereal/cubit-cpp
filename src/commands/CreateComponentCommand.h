#ifndef CREATECOMPONENTCOMMAND_H
#define CREATECOMPONENTCOMMAND_H

#include "../Command.h"
#include <QString>

class ElementModel;
class DesignElement;
class ComponentElement;
class SelectionManager;

class CreateComponentCommand : public Command
{
    Q_OBJECT

public:
    CreateComponentCommand(ElementModel* elementModel, SelectionManager* selectionManager, 
                          DesignElement* sourceElement, QObject *parent = nullptr);
    ~CreateComponentCommand();

    void execute() override;
    void undo() override;
    void redo() override;

    DesignElement* getCreatedInstance() const { return m_createdInstance; }

private:
    ElementModel* m_elementModel;
    SelectionManager* m_selectionManager;
    DesignElement* m_sourceElement;
    DesignElement* m_createdInstance;
    ComponentElement* m_createdComponent;
    QString m_sourceElementId;
    QString m_sourceParentId;
    QString m_instanceId;
    QString m_componentId;
    QList<DesignElement*> m_createdChildInstances;  // Track created child instances for undo
    
    void createChildInstances(DesignElement* sourceElement, DesignElement* parentInstance);
};

#endif // CREATECOMPONENTCOMMAND_H