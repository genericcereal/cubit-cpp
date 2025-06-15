#pragma once
#include "CanvasElement.h"
#include <QColor>
#include <QStringList>
#include <QMap>
#include <QVariantList>
#include <QVariantMap>

class Node : public CanvasElement {
    Q_OBJECT
    Q_PROPERTY(QString nodeTitle READ nodeTitle WRITE setNodeTitle NOTIFY nodeTitleChanged)
    Q_PROPERTY(QColor nodeColor READ nodeColor WRITE setNodeColor NOTIFY nodeColorChanged)
    Q_PROPERTY(QStringList inputPorts READ inputPorts WRITE setInputPorts NOTIFY inputPortsChanged)
    Q_PROPERTY(QStringList outputPorts READ outputPorts WRITE setOutputPorts NOTIFY outputPortsChanged)
    Q_PROPERTY(bool isExecuting READ isExecuting WRITE setIsExecuting NOTIFY isExecutingChanged)
    Q_PROPERTY(QVariantList rowConfigurations READ rowConfigurations NOTIFY rowConfigurationsChanged)
    
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
    
signals:
    void nodeTitleChanged();
    void nodeColorChanged();
    void inputPortsChanged();
    void outputPortsChanged();
    void isExecutingChanged();
    void rowConfigurationsChanged();
    
private:
    QString m_nodeTitle;
    QColor m_nodeColor;
    QStringList m_inputPorts;
    QStringList m_outputPorts;
    QMap<int, QString> m_inputPortTypes;
    QMap<int, QString> m_outputPortTypes;
    bool m_isExecuting;
    QList<RowConfig> m_rowConfigs;
};