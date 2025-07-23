#ifndef RESIZEELEMENTCOMMAND_H
#define RESIZEELEMENTCOMMAND_H

#include "../Command.h"
#include <QRectF>

class CanvasElement;

class ResizeElementCommand : public Command
{
    Q_OBJECT

public:
    ResizeElementCommand(CanvasElement* element, const QRectF& oldRect, const QRectF& newRect, QObject *parent = nullptr);
    ~ResizeElementCommand();

    void execute() override;
    void undo() override;

    // Merge with another resize command if possible
    bool mergeWith(ResizeElementCommand* other);

private:
    void syncWithAPI();
    CanvasElement* m_element;
    QRectF m_oldRect;
    QRectF m_newRect;
    bool m_firstExecute;
};

#endif // RESIZEELEMENTCOMMAND_H