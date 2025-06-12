#include "Node.h"
#include "Config.h"
#include <QDebug>

Node::Node(const QString &id, QObject *parent)
    : Element(Element::NodeType, id, parent)
    , m_nodeTitle("Node")
    , m_nodeColor(Config::NODE_DEFAULT_COLOR)
    , m_isExecuting(false)
{
    // Set default size for nodes
    setWidth(150);
    setHeight(80);
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