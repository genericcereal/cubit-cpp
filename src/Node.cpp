#include "Node.h"
#include "Config.h"
#include "ElementModel.h"
#include "Frame.h"
#include "Text.h"
#include "Variable.h"
#include "Scripts.h"
#include "Project.h"
#include <QDebug>

Node::Node(const QString &id, QObject *parent)
    : ScriptElement(id, parent)
    , m_nodeTitle("Node")
    , m_nodeColor(Config::NODE_DEFAULT_COLOR)
    , m_isExecuting(false)
{
    // Set element type
    elementType = Element::NodeType;
    
    // Set object name for type identification
    setObjectName("Node");
    
    // Set default name using last 4 digits of ID
    setName(QString("Node %1").arg(id.right(4)));
    
    // Set default size for nodes
    setWidth(200);
    setHeight(180);
    
    // qDebug() << "Node constructor - ID:" << id << "Initial nodeType:" << m_nodeType;
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

void Node::setValue(const QString &value)
{
    if (m_value != value) {
        m_value = value;
        emit valueChanged();
        emit elementChanged();
    }
}

void Node::setSourceElementId(const QString &elementId)
{
    if (m_sourceElementId != elementId) {
        m_sourceElementId = elementId;
        emit sourceElementIdChanged();
        emit elementChanged();
    }
}

void Node::setIsAsync(bool isAsync)
{
    if (m_isAsync != isAsync) {
        m_isAsync = isAsync;
        emit isAsyncChanged();
        emit elementChanged();
    }
}

void Node::setScript(const QString &script)
{
    if (m_script != script) {
        m_script = script;
        emit scriptChanged();
        emit elementChanged();
    }
}

QString Node::value() const
{
    // If this is a Variable node representing a design element
    if (m_nodeType == "Variable" && !m_sourceElementId.isEmpty()) {
        qDebug() << "Node::value() - Variable node" << getId() << "has sourceElementId:" << m_sourceElementId;
        
        // Try to find the element model from parent hierarchy
        ElementModel* elementModel = nullptr;
        QObject* p = parent();
        while (p && !elementModel) {
            elementModel = qobject_cast<ElementModel*>(p);
            p = p->parent();
        }
        
        if (elementModel) {
            Element* sourceElement = elementModel->getElementById(m_sourceElementId);
            if (sourceElement) {
                qDebug() << "Node::value() - Found source element:" << sourceElement->getName() 
                         << "type:" << sourceElement->objectName();
                
                // Return the element ID for design elements (Frame, Text)
                if (qobject_cast<Frame*>(sourceElement) || 
                    qobject_cast<Text*>(sourceElement)) {
                    QString id = sourceElement->getId();
                    qDebug() << "Node::value() - Returning element ID:" << id;
                    return id;
                }
                // Return the value for Variable elements
                else if (Variable* var = qobject_cast<Variable*>(sourceElement)) {
                    QString val = var->value().toString();
                    qDebug() << "Node::value() - Returning Variable value:" << val;
                    return val;
                }
            } else {
                qDebug() << "Node::value() - Source element not found for ID:" << m_sourceElementId;
            }
        } else {
            qDebug() << "Node::value() - ElementModel not found in parent hierarchy";
            // Try to get ElementModel from Scripts parent
            if (Scripts* scripts = qobject_cast<Scripts*>(parent())) {
                QObject* scriptsParent = scripts->parent();
                while (scriptsParent) {
                    if (ElementModel* model = qobject_cast<ElementModel*>(scriptsParent)) {
                        Element* sourceElement = model->getElementById(m_sourceElementId);
                        if (sourceElement) {
                            if (Variable* var = qobject_cast<Variable*>(sourceElement)) {
                                QString val = var->value().toString();
                                qDebug() << "Node::value() - Found Variable through Scripts parent:" << val;
                                return val;
                            }
                        }
                        break;
                    }
                    // Also check if parent is a Project which might have the ElementModel
                    if (Project* project = qobject_cast<Project*>(scriptsParent)) {
                        if (ElementModel* model = project->elementModel()) {
                            Element* sourceElement = model->getElementById(m_sourceElementId);
                            if (sourceElement) {
                                if (Variable* var = qobject_cast<Variable*>(sourceElement)) {
                                    QString val = var->value().toString();
                                    qDebug() << "Node::value() - Found Variable through Project:" << val;
                                    return val;
                                }
                            }
                        }
                        break;
                    }
                    scriptsParent = scriptsParent->parent();
                }
            }
        }
    }
    
    // Default: return the stored value
    return m_value;
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
    
    // Also set the port types so they're available for edge creation
    if (config.hasSource && config.sourcePortIndex >= 0) {
        setOutputPortType(config.sourcePortIndex, config.sourceType);
    }
    if (config.hasTarget && config.targetPortIndex >= 0) {
        setInputPortType(config.targetPortIndex, config.targetType);
    }
    
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

int Node::getInputPortIndex(const QString &portId) const
{
    return m_inputPorts.indexOf(portId);
}

int Node::getOutputPortIndex(const QString &portId) const
{
    return m_outputPorts.indexOf(portId);
}

void Node::addDynamicRow()
{
    if (!m_isDynamic) return;
    
    // Find the current highest row index for number inputs
    int maxIndex = -1;
    for (int i = 0; i < m_rowConfigs.size(); ++i) {
        if (m_rowConfigs[i].hasTarget && m_rowConfigs[i].targetType == "Number") {
            maxIndex = i;
        }
    }
    
    // Add a new row configuration
    int newIndex = maxIndex + 1;
    RowConfig newRow;
    newRow.hasTarget = true;
    newRow.targetLabel = QString("[%1]").arg(newIndex);
    newRow.targetType = "Number";
    newRow.targetPortIndex = newIndex;
    
    // Add to row configs
    m_rowConfigs.append(newRow);
    
    // Add to input ports
    QString portId = QString("value%1").arg(newIndex);
    m_inputPorts.append(portId);
    m_inputPortTypes[newIndex] = "Number";
    
    emit rowConfigurationsChanged();
    emit inputPortsChanged();
    emit elementChanged();
}

void Node::removeDynamicRow(int rowIndex)
{
    if (!m_isDynamic || rowIndex < 1) return;  // Don't remove the first row
    
    // Find the row with this index
    int configIndex = -1;
    for (int i = 0; i < m_rowConfigs.size(); ++i) {
        if (m_rowConfigs[i].hasTarget && m_rowConfigs[i].targetPortIndex == rowIndex) {
            configIndex = i;
            break;
        }
    }
    
    if (configIndex >= 0) {
        // Remove from row configs
        m_rowConfigs.removeAt(configIndex);
        
        // Remove from input ports
        QString portId = QString("value%1").arg(rowIndex);
        int portIndex = m_inputPorts.indexOf(portId);
        if (portIndex >= 0) {
            m_inputPorts.removeAt(portIndex);
        }
        m_inputPortTypes.remove(rowIndex);
        
        // Re-index remaining rows
        for (int i = 0; i < m_rowConfigs.size(); ++i) {
            if (m_rowConfigs[i].hasTarget && m_rowConfigs[i].targetPortIndex > rowIndex) {
                m_rowConfigs[i].targetPortIndex--;
                m_rowConfigs[i].targetLabel = QString("[%1]").arg(m_rowConfigs[i].targetPortIndex);
            }
        }
        
        // Re-index input ports
        QStringList newInputPorts;
        QMap<int, QString> newInputPortTypes;
        
        for (int i = 0; i < m_inputPorts.size(); ++i) {
            QString port = m_inputPorts[i];
            if (port.startsWith("value")) {
                QString numStr = port.mid(5);
                bool ok;
                int num = numStr.toInt(&ok);
                if (ok && num > rowIndex) {
                    newInputPorts.append(QString("value%1").arg(num - 1));
                    newInputPortTypes[num - 1] = m_inputPortTypes.value(num, "Number");
                } else {
                    newInputPorts.append(port);
                    if (m_inputPortTypes.contains(i)) {
                        newInputPortTypes[i] = m_inputPortTypes[i];
                    }
                }
            } else {
                newInputPorts.append(port);
                if (m_inputPortTypes.contains(i)) {
                    newInputPortTypes[i] = m_inputPortTypes[i];
                }
            }
        }
        
        m_inputPorts = newInputPorts;
        m_inputPortTypes = newInputPortTypes;
        
        emit rowConfigurationsChanged();
        emit inputPortsChanged();
        emit elementChanged();
    }
}

QString Node::getPortValue(int portIndex) const
{
    // For dynamic nodes, return port-specific value
    if (m_isDynamic && m_portValues.contains(portIndex)) {
        return m_portValues[portIndex];
    }
    // For non-dynamic nodes or if no specific value, return the general value
    return m_value;
}

void Node::setPortValue(int portIndex, const QString &value)
{
    if (m_isDynamic) {
        // For dynamic nodes, store value per port
        m_portValues[portIndex] = value;
    } else {
        // For non-dynamic nodes, use the general value
        m_value = value;
    }
    emit valueChanged();
    emit elementChanged();
}