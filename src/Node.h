#pragma once
#include "ScriptElement.h"
#include <QColor>
#include <QStringList>
#include <QMap>
#include <QVariantList>
#include <QVariantMap>

class Node : public ScriptElement {
    Q_OBJECT
    Q_PROPERTY(QString nodeTitle READ nodeTitle WRITE setNodeTitle NOTIFY nodeTitleChanged)
    Q_PROPERTY(QString nodeType READ nodeType CONSTANT)
    Q_PROPERTY(QColor nodeColor READ nodeColor WRITE setNodeColor NOTIFY nodeColorChanged)
    Q_PROPERTY(QStringList inputPorts READ inputPorts WRITE setInputPorts NOTIFY inputPortsChanged)
    Q_PROPERTY(QStringList outputPorts READ outputPorts WRITE setOutputPorts NOTIFY outputPortsChanged)
    Q_PROPERTY(bool isExecuting READ isExecuting WRITE setIsExecuting NOTIFY isExecutingChanged)
    Q_PROPERTY(QVariantList rowConfigurations READ rowConfigurations NOTIFY rowConfigurationsChanged)
    Q_PROPERTY(QString value READ value WRITE setValue NOTIFY valueChanged)
    Q_PROPERTY(QString sourceElementId READ sourceElementId WRITE setSourceElementId NOTIFY sourceElementIdChanged)
    Q_PROPERTY(bool isAsync READ isAsync WRITE setIsAsync NOTIFY isAsyncChanged)
    
public:
    explicit Node(const QString &id, QObject *parent = nullptr);
    
    // Property getters
    QString nodeTitle() const { return m_nodeTitle; }
    QString nodeType() const { return m_nodeType; }
    QColor nodeColor() const { return m_nodeColor; }
    QStringList inputPorts() const { return m_inputPorts; }
    QStringList outputPorts() const { return m_outputPorts; }
    bool isExecuting() const { return m_isExecuting; }
    QString value() const;
    QString sourceElementId() const { return m_sourceElementId; }
    bool isAsync() const { return m_isAsync; }
    
    // Property setters
    void setNodeTitle(const QString &title);
    void setNodeColor(const QColor &color);
    void setInputPorts(const QStringList &ports);
    void setOutputPorts(const QStringList &ports);
    void setIsExecuting(bool executing);
    void setValue(const QString &value);
    void setSourceElementId(const QString &elementId);
    void setIsAsync(bool isAsync);
    
    // Port management
    void addInputPort(const QString &portName);
    void addOutputPort(const QString &portName);
    void removeInputPort(const QString &portName);
    void removeOutputPort(const QString &portName);
    
    // Connection points
    QPointF getInputPortPosition(int index) const;
    QPointF getOutputPortPosition(int index) const;
    
    // Port type management
    void setInputPortType(int index, const QString &type);
    void setOutputPortType(int index, const QString &type);
    QString getInputPortType(int index) const;
    QString getOutputPortType(int index) const;
    
    // Row configuration
    struct RowConfig {
        bool hasTarget = false;
        QString targetLabel;
        QString targetType = "Flow";
        int targetPortIndex = -1;
        
        bool hasSource = false;
        QString sourceLabel;
        QString sourceType = "Flow";
        int sourcePortIndex = -1;
    };
    
    void addRow(const RowConfig &config);
    void clearRows();
    QVariantList rowConfigurations() const;
    Q_INVOKABLE int getRowForInputPort(int portIndex) const;
    Q_INVOKABLE int getRowForOutputPort(int portIndex) const;
    
    // Port ID lookup
    int getInputPortIndex(const QString &portId) const;
    int getOutputPortIndex(const QString &portId) const;
    
    // Set node type (only in constructor or initialization)
    void setNodeType(const QString &type) { m_nodeType = type; }
    
signals:
    void nodeTitleChanged();
    void nodeColorChanged();
    void inputPortsChanged();
    void outputPortsChanged();
    void isExecutingChanged();
    void rowConfigurationsChanged();
    void valueChanged();
    void sourceElementIdChanged();
    void isAsyncChanged();
    
private:
    QString m_nodeTitle;
    QString m_nodeType = "Operation";  // Default to Operation
    QColor m_nodeColor;
    QStringList m_inputPorts;
    QStringList m_outputPorts;
    QMap<int, QString> m_inputPortTypes;
    QMap<int, QString> m_outputPortTypes;
    bool m_isExecuting;
    QList<RowConfig> m_rowConfigs;
    QString m_value;  // Stores input value for non-Flow type inputs
    QString m_sourceElementId;  // ID of the design element this Variable node represents
    bool m_isAsync = false;  // Whether this node returns a Promise
};