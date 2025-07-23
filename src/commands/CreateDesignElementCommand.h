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

class CreateDesignElementCommand : public Command
{
    Q_OBJECT

public:
    CreateDesignElementCommand(ElementModel* model, SelectionManager* selectionManager,
                              const QString& elementType, const QRectF& rect, 
                              const QVariant& initialPayload = QVariant(),
                              QObject *parent = nullptr);
    ~CreateDesignElementCommand();

    void execute() override;
    void undo() override;

private:
    void syncWithAPI();
    QPointer<ElementModel> m_elementModel;
    QPointer<SelectionManager> m_selectionManager;
    QString m_elementType;
    QRectF m_rect;
    QVariant m_initialPayload;
    
    // Created element
    QPointer<DesignElement> m_element;
    
    // ID for the created element
    QString m_elementId;
};

#endif // CREATEDESIGNELEMENTCOMMAND_H