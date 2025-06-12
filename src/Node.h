#pragma once
#include "Element.h"
#include <QColor>
#include <QStringList>

class Node : public Element {
    Q_OBJECT
    Q_PROPERTY(QString nodeTitle READ nodeTitle WRITE setNodeTitle NOTIFY nodeTitleChanged)
    Q_PROPERTY(QColor nodeColor READ nodeColor WRITE setNodeColor NOTIFY nodeColorChanged)
    Q_PROPERTY(QStringList inputPorts READ inputPorts WRITE setInputPorts NOTIFY inputPortsChanged)
    Q_PROPERTY(QStringList outputPorts READ outputPorts WRITE setOutputPorts NOTIFY outputPortsChanged)
    Q_PROPERTY(bool isExecuting READ isExecuting WRITE setIsExecuting NOTIFY isExecutingChanged)
    
public:
    explicit Node(const QString &id, QObject *parent = nullptr);
    
    // Property getters
    QString nodeTitle() const { return m_nodeTitle; }
    QColor nodeColor() const { return m_nodeColor; }
    QStringList inputPorts() const { return m_inputPorts; }
    QStringList outputPorts() const { return m_outputPorts; }
    bool isExecuting() const { return m_isExecuting; }
    
    // Property setters
    void setNodeTitle(const QString &title);
    void setNodeColor(const QColor &color);
    void setInputPorts(const QStringList &ports);
    void setOutputPorts(const QStringList &ports);
    void setIsExecuting(bool executing);
    
    // Port management
    void addInputPort(const QString &portName);
    void addOutputPort(const QString &portName);
    void removeInputPort(const QString &portName);
    void removeOutputPort(const QString &portName);
    
    // Connection points
    QPointF getInputPortPosition(int index) const;
    QPointF getOutputPortPosition(int index) const;
    
signals:
    void nodeTitleChanged();
    void nodeColorChanged();
    void inputPortsChanged();
    void outputPortsChanged();
    void isExecutingChanged();
    
private:
    QString m_nodeTitle;
    QColor m_nodeColor;
    QStringList m_inputPorts;
    QStringList m_outputPorts;
    bool m_isExecuting;
};