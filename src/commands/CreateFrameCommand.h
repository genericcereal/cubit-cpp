#ifndef CREATEFRAMECOMMAND_H
#define CREATEFRAMECOMMAND_H

#include "../Command.h"
#include <QRectF>
#include <QString>

class ElementModel;
class SelectionManager;
class Frame;

class CreateFrameCommand : public Command
{
    Q_OBJECT

public:
    CreateFrameCommand(ElementModel* model, SelectionManager* selectionManager,
                       const QRectF& rect, QObject *parent = nullptr);
    ~CreateFrameCommand();

    void execute() override;
    void undo() override;

private:
    ElementModel* m_elementModel;
    SelectionManager* m_selectionManager;
    QRectF m_rect;
    QString m_frameId;
    Frame* m_frame;
};

#endif // CREATEFRAMECOMMAND_H