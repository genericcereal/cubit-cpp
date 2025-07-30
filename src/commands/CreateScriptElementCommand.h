#ifndef CREATESCRIPTELEMENTCOMMAND_H
#define CREATESCRIPTELEMENTCOMMAND_H

#include "../Command.h"
#include "../ScriptElement.h"
#include <QRectF>
#include <QString>
#include <QPointer>
#include <QVariant>

class ElementModel;
class SelectionManager;
class Scripts;
class Node;
class Edge;

class CreateScriptElementCommand : public Command
{
    Q_OBJECT

public:
    CreateScriptElementCommand(ElementModel* model, SelectionManager* selectionManager,
                              Scripts* scripts, const QString& elementType, 
                              const QRectF& rect, const QVariant& initialPayload = QVariant(),
                              QObject *parent = nullptr);
    ~CreateScriptElementCommand();

    void execute() override;
    void undo() override;
    
    // Get the created element (for further manipulation)
    ScriptElement* element() const { return m_element; }

private:
    void setupDefaultNodePorts(Node* node);
    void calculateEdgePoints(Edge* edge);
    void syncWithApi();
    
    QPointer<ElementModel> m_elementModel;
    QPointer<SelectionManager> m_selectionManager;
    QPointer<Scripts> m_scripts;
    QString m_elementType;
    QRectF m_rect;
    QVariant m_initialPayload;
    
    // Created element
    QPointer<ScriptElement> m_element;
    
    // ID for the created element
    QString m_elementId;
};

#endif // CREATESCRIPTELEMENTCOMMAND_H