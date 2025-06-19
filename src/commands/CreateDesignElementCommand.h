#ifndef CREATEDESIGNELEMENTCOMMAND_H
#define CREATEDESIGNELEMENTCOMMAND_H

#include "../Command.h"
#include <QRectF>
#include <QString>
#include <QPointer>
#include <QVariant>

class ElementModel;
class SelectionManager;
class DesignElement;
class Frame;
class Text;
class Html;

class CreateDesignElementCommand : public Command
{
    Q_OBJECT

public:
    enum ElementType {
        FrameElement,
        TextElement,
        HtmlElement
    };
    Q_ENUM(ElementType)

    CreateDesignElementCommand(ElementModel* model, SelectionManager* selectionManager,
                              ElementType type, const QRectF& rect, 
                              const QVariant& initialPayload = QVariant(),
                              QObject *parent = nullptr);
    ~CreateDesignElementCommand();

    void execute() override;
    void undo() override;

private:
    QPointer<ElementModel> m_elementModel;
    QPointer<SelectionManager> m_selectionManager;
    ElementType m_elementType;
    QRectF m_rect;
    QVariant m_initialPayload;
    
    // Created elements
    QPointer<Frame> m_frame;
    QPointer<Text> m_textElement;
    QPointer<Html> m_htmlElement;
    
    // IDs for the created elements
    QString m_frameId;
    QString m_childElementId;
};

#endif // CREATEDESIGNELEMENTCOMMAND_H