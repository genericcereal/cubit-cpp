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
                              const QString& parentId = QString(),
                              QObject *parent = nullptr);
    ~CreateDesignElementCommand();

    void execute() override;
    void undo() override;
    
    // Call this after the element has been resized to its final dimensions
    void creationCompleted();

private:
    void syncWithAPI();
    QPointer<ElementModel> m_elementModel;
    QPointer<SelectionManager> m_selectionManager;
    QString m_elementType;
    QRectF m_rect;
    QVariant m_initialPayload;
    QString m_parentId;
    
    // Created element
    QPointer<DesignElement> m_element;
    
    // ID for the created element
    QString m_elementId;
};

#endif // CREATEDESIGNELEMENTCOMMAND_H