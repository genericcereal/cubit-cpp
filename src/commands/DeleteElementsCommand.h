#ifndef DELETEELEMENTSCOMMAND_H
#define DELETEELEMENTSCOMMAND_H

#include "../Command.h"
#include <QList>
#include <QString>
#include <QMap>

class Element;
class ElementModel;
class SelectionManager;

class DeleteElementsCommand : public Command
{
    Q_OBJECT

public:
    DeleteElementsCommand(ElementModel* model, SelectionManager* selectionManager,
                          const QList<Element*>& elements, QObject *parent = nullptr);
    ~DeleteElementsCommand();

    void execute() override;
    void undo() override;

private:
    void syncWithAPI();
    void findChildElements(const QString& parentId, const QList<Element*>& allElements);
    
    struct ElementInfo {
        Element* element;
        Element* parent;
        int index;
    };

    ElementModel* m_elementModel;
    SelectionManager* m_selectionManager;
    QList<ElementInfo> m_deletedElements;
    QList<ElementInfo> m_deletedChildren;
    QStringList m_deletedElementIds;  // Store IDs before deletion for API sync
};

#endif // DELETEELEMENTSCOMMAND_H