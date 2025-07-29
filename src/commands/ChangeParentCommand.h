#ifndef CHANGEPARENTCOMMAND_H
#define CHANGEPARENTCOMMAND_H

#include "Command.h"
#include <QString>
#include <QPointF>

class Element;
class CanvasElement;
class DesignElement;

class ChangeParentCommand : public Command
{
    Q_OBJECT
public:
    explicit ChangeParentCommand(Element* element, const QString& newParentId, QObject *parent = nullptr);
    explicit ChangeParentCommand(DesignElement* element, CanvasElement* newParent, const QPointF& relativePosition, QObject *parent = nullptr);
    virtual ~ChangeParentCommand();

    void execute() override;
    void undo() override;

private:
    void syncWithAPI();

    Element* m_element;
    DesignElement* m_designElement; // For elements that support advanced parenting
    QString m_oldParentId;
    QString m_newParentId;
    
    // For design elements that support position-based parenting
    bool m_usePositionBasedParenting;
    CanvasElement* m_newParent;
    QPointF m_relativePosition;
    QPointF m_oldPosition; // Store old absolute position for undo
    
    // Track which model the element was in for undo
    bool m_wasInGlobalElements = false;
    QString m_originalPlatformName;
};

#endif // CHANGEPARENTCOMMAND_H