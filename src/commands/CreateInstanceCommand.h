#ifndef CREATEINSTANCECOMMAND_H
#define CREATEINSTANCECOMMAND_H

#include "../Command.h"
#include "../DesignElement.h"
#include <QString>
#include <QPointer>

class ElementModel;
class Element;
class ComponentInstance;

class CreateInstanceCommand : public Command
{
    Q_OBJECT

public:
    CreateInstanceCommand(ElementModel* elementModel, Element* sourceElement,
                         const QString& parentId = QString(),
                         QObject *parent = nullptr);
    ~CreateInstanceCommand();

    void execute() override;
    void undo() override;

    DesignElement* getCreatedInstance() const { return m_createdInstance; }

private:
    QPointer<ElementModel> m_elementModel;
    QPointer<Element> m_sourceElement;
    QPointer<DesignElement> m_createdInstance;
    QString m_parentId;
    QString m_instanceId;
};

#endif // CREATEINSTANCECOMMAND_H