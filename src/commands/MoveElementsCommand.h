#ifndef MOVEELEMENTSCOMMAND_H
#define MOVEELEMENTSCOMMAND_H

#include "../Command.h"
#include <QList>
#include <QPointF>
#include <QMap>
#include <QHash>

class Element;
class CanvasElement;

class MoveElementsCommand : public Command
{
    Q_OBJECT

public:
    MoveElementsCommand(const QList<Element*>& elements, const QPointF& delta, QObject *parent = nullptr);
    MoveElementsCommand(const QList<Element*>& elements, const QPointF& delta, const QHash<QString, QPointF>& originalPositions, QObject *parent = nullptr);
    ~MoveElementsCommand();

    void execute() override;
    void undo() override;

    // Merge with another move command if possible
    bool mergeWith(MoveElementsCommand* other);

private:
    void syncWithAPI();
    void syncGlobalElements();
    
    struct ElementMove {
        CanvasElement* element;
        QPointF originalPosition;
        QPointF newPosition;
    };

    QList<ElementMove> m_moves;
    QPointF m_totalDelta;
    bool m_firstExecute;
};

#endif // MOVEELEMENTSCOMMAND_H