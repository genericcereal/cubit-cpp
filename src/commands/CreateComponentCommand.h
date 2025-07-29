#ifndef CREATECOMPONENTCOMMAND_H
#define CREATECOMPONENTCOMMAND_H

#include "../Command.h"
#include "../Component.h"
#include <QString>
#include <QPointer>
#include <memory>

class ElementModel;
class DesignElement;
class CreateInstanceCommand;

class CreateComponentCommand : public Command
{
    Q_OBJECT

public:
    CreateComponentCommand(ElementModel* elementModel, DesignElement* sourceElement,
                          QObject *parent = nullptr);
    ~CreateComponentCommand();

    void execute() override;
    void undo() override;

    Component* getCreatedComponent() const { return m_createdComponent; }

private:
    QPointer<ElementModel> m_elementModel;
    QPointer<DesignElement> m_sourceElement;
    QPointer<Component> m_createdComponent;
    std::unique_ptr<CreateInstanceCommand> m_createInstanceCommand;
    QString m_componentId;
    QString m_variantId;
};

#endif // CREATECOMPONENTCOMMAND_H