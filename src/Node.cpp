#include "Node.h"
#include "Config.h"
#include <QDebug>

Node::Node(const QString &id, QObject *parent)
    : Element(Element::NodeType, id, parent)
    , m_nodeTitle("Node")
    , m_nodeColor(Config::NODE_DEFAULT_COLOR)
    , m_isExecuting(false)
{
    // Set object name for type identification
    setObjectName("Node");
    
    // Set default size for nodes
    setWidth(200);
    setHeight(180);
}

void Node::setNodeTitle(const QString &title)
{
    if (m_nodeTitle != title) {
        m_nodeTitle = title;
        emit nodeTitleChanged();
        emit elementChanged();
    }
}

void Node::setNodeColor(const QColor &color)
{
    if (m_nodeColor != color) {
        m_nodeColor = color;
        emit nodeColorChanged();
        emit elementChanged();
    }
}

void Node::setInputPorts(const QStringList &ports)
{
    if (m_inputPorts != ports) {
        m_inputPorts = ports;
        emit inputPortsChanged();
        emit elementChanged();
    }
}

void Node::setOutputPorts(const QStringList &ports)
{
    if (m_outputPorts != ports) {
        m_outputPorts = ports;
        emit outputPortsChanged();
        emit elementChanged();
    }
}

void Node::setIsExecuting(bool executing)
{
    if (m_isExecuting != executing) {
        m_isExecuting = executing;
        emit isExecutingChanged();
    }
}

void Node::addInputPort(const QString &portName)
{
    if (!m_inputPorts.contains(portName)) {
        m_inputPorts.append(portName);
        emit inputPortsChanged();
        emit elementChanged();
    }
}

void Node::addOutputPort(const QString &portName)
{
    if (!m_outputPorts.contains(portName)) {
        m_outputPorts.append(portName);
        emit outputPortsChanged();
        emit elementChanged();
    }
}

void Node::removeInputPort(const QString &portName)
{
    if (m_inputPorts.removeAll(portName) > 0) {
        emit inputPortsChanged();
        emit elementChanged();
    }
}

void Node::removeOutputPort(const QString &portName)
{
    if (m_outputPorts.removeAll(portName) > 0) {
        emit outputPortsChanged();
        emit elementChanged();
    }
}

QPointF Node::getInputPortPosition(int index) const
{
    if (index < 0 || index >= m_inputPorts.size()) {
        return QPointF();
    }
    
    // Calculate port position on the left side of the node
    qreal portSpacing = height() / (m_inputPorts.size() + 1);
    qreal portY = y() + portSpacing * (index + 1);
    
    return QPointF(x(), portY);
}

QPointF Node::getOutputPortPosition(int index) const
{
    if (index < 0 || index >= m_outputPorts.size()) {
        return QPointF();
    }
    
    // Calculate port position on the right side of the node
    qreal portSpacing = height() / (m_outputPorts.size() + 1);
    qreal portY = y() + portSpacing * (index + 1);
    
    return QPointF(x() + width(), portY);
}

void Node::setInputPortType(int index, const QString &type)
{
    m_inputPortTypes[index] = type;
}

void Node::setOutputPortType(int index, const QString &type)
{
    m_outputPortTypes[index] = type;
}

QString Node::getInputPortType(int index) const
{
    return m_inputPortTypes.value(index, "Flow"); // Default to Flow
}

QString Node::getOutputPortType(int index) const
{
    return m_outputPortTypes.value(index, "Flow"); // Default to Flow
}

void Node::addRow(const RowConfig &config)
{
    m_rowConfigs.append(config);
    emit rowConfigurationsChanged();
}

void Node::clearRows()
{
    m_rowConfigs.clear();
    emit rowConfigurationsChanged();
}

QVariantList Node::rowConfigurations() const
{
    QVariantList rows;
    for (const auto &config : m_rowConfigs) {
        QVariantMap row;
        row["hasTarget"] = config.hasTarget;
        row["targetLabel"] = config.targetLabel;
        row["targetType"] = config.targetType;
        row["targetPortIndex"] = config.targetPortIndex;
        row["hasSource"] = config.hasSource;
        row["sourceLabel"] = config.sourceLabel;
        row["sourceType"] = config.sourceType;
        row["sourcePortIndex"] = config.sourcePortIndex;
        rows.append(row);
    }
    return rows;
}

int Node::getRowForInputPort(int portIndex) const
{
    for (int i = 0; i < m_rowConfigs.size(); ++i) {
        if (m_rowConfigs[i].hasTarget && m_rowConfigs[i].targetPortIndex == portIndex) {
            return i;
        }
    }
    return -1;
}

int Node::getRowForOutputPort(int portIndex) const
{
    for (int i = 0; i < m_rowConfigs.size(); ++i) {
        if (m_rowConfigs[i].hasSource && m_rowConfigs[i].sourcePortIndex == portIndex) {
            return i;
        }
    }
    return -1;
}