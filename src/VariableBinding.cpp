#include "VariableBinding.h"
#include "Variable.h"
#include "Element.h"
#include "CanvasElement.h"
#include "ElementModel.h"
#include "Application.h"
#include "Project.h"
#include <QDebug>

VariableBinding::VariableBinding(QObject *parent)
    : QObject(parent)
{
}

void VariableBinding::bind(const QString& variableId, const QString& elementId, const QString& propertyName)
{
    if (m_variableId == variableId && m_elementId == elementId && m_propertyName == propertyName) {
        return; // Already bound to the same variable and property
    }
    
    // Disconnect from previous variable if any
    disconnectFromVariable();
    
    m_variableId = variableId;
    m_elementId = elementId;
    m_propertyName = propertyName;
    
    emit variableIdChanged();
    emit elementIdChanged();
    emit propertyNameChanged();
    
    // Connect to the new variable
    connectToVariable();
    
    // Update the initial value
    updateValue();
}

void VariableBinding::unbind()
{
    disconnectFromVariable();
    
    m_variableId.clear();
    m_elementId.clear();
    m_propertyName.clear();
    m_isActive = false;
    
    emit variableIdChanged();
    emit elementIdChanged();
    emit propertyNameChanged();
    emit isActiveChanged();
}

void VariableBinding::updateValue()
{
    if (!m_variable || !m_element || m_propertyName.isEmpty()) {
        // Missing components - cannot update
        return;
    }
    
    // Get the variable value
    QVariant value = m_variable->property("value");
    // Setting property value from variable
    
    // Convert the value if needed based on the variable type
    QString variableType = m_variable->property("variableType").toString();
    if (variableType == "number") {
        // Convert to double for numeric properties
        bool ok;
        double numValue = value.toDouble(&ok);
        if (ok) {
            value = QVariant(numValue);
        } else {
            // Failed to convert value to number - could be empty or invalid string
            return; // Don't update property with invalid value
        }
    }
    
    // Set the property on the element
    const char* propName = m_propertyName.toUtf8().constData();
    // Set the property value
    
    // Check if this is a standard property that needs special handling
    if (m_propertyName == "x" && m_element->inherits("CanvasElement")) {
        qobject_cast<CanvasElement*>(m_element)->setX(value.toDouble());
    } else if (m_propertyName == "y" && m_element->inherits("CanvasElement")) {
        qobject_cast<CanvasElement*>(m_element)->setY(value.toDouble());
    } else if (m_propertyName == "width" && m_element->inherits("CanvasElement")) {
        qobject_cast<CanvasElement*>(m_element)->setWidth(value.toDouble());
    } else if (m_propertyName == "height" && m_element->inherits("CanvasElement")) {
        qobject_cast<CanvasElement*>(m_element)->setHeight(value.toDouble());
    } else {
        // Use setProperty for other properties
        m_element->setProperty(propName, value);
    }
    
    // Verify the property was set
    QVariant newValue = m_element->property(propName);
    // Verify property was set
    
    emit valueUpdated(value);
}

void VariableBinding::onVariableValueChanged()
{
    updateValue();
}

void VariableBinding::connectToVariable()
{
    if (m_variableId.isEmpty() || m_elementId.isEmpty()) {
        // Empty IDs - cannot connect
        return;
    }
    
    // Get the element model from the parent (VariableBindingManager's parent should be Project)
    auto* bindingManager = qobject_cast<VariableBindingManager*>(parent());
    if (!bindingManager) {
        // Parent is not VariableBindingManager
        return;
    }
    
    auto* project = qobject_cast<Project*>(bindingManager->parent());
    if (!project) {
        // BindingManager's parent is not Project
        return;
    }
    
    auto* elementModel = project->elementModel();
    if (!elementModel) {
        // No element model
        return;
    }
    
    // Find the variable
    auto* variable = elementModel->getElementById(m_variableId);
    if (variable && variable->inherits("Variable")) {
        m_variable = qobject_cast<Variable*>(variable);
        // Found variable
        
        // Find the element
        m_element = elementModel->getElementById(m_elementId);
        // Found element
        
        if (m_variable && m_element) {
            // Connect to value changes
            connect(m_variable, &Variable::valueChanged,
                    this, &VariableBinding::onVariableValueChanged,
                    Qt::UniqueConnection);
            
            // Successfully connected variable to element property
            
            m_isActive = true;
            emit isActiveChanged();
            return;
        }
    }
    
    qWarning() << "Could not find variable" << m_variableId << "or element" << m_elementId;
}

void VariableBinding::disconnectFromVariable()
{
    if (m_variable) {
        disconnect(m_variable, &Variable::valueChanged,
                   this, &VariableBinding::onVariableValueChanged);
        m_variable = nullptr;
    }
    
    m_element = nullptr;
    m_isActive = false;
    emit isActiveChanged();
}

// VariableBindingManager implementation

VariableBindingManager::VariableBindingManager(QObject *parent)
    : QObject(parent)
{
}

VariableBindingManager::~VariableBindingManager()
{
    clearAllBindings();
}

VariableBinding* VariableBindingManager::createBinding(const QString& variableId, 
                                                       const QString& elementId, 
                                                       const QString& propertyName)
{
    // Creating new binding
    
    QString key = makeKey(elementId, propertyName);
    
    // Remove existing binding if any
    if (m_bindings.contains(key)) {
        removeBinding(elementId, propertyName);
    }
    
    // Create new binding
    auto* binding = new VariableBinding(this);
    binding->bind(variableId, elementId, propertyName);
    
    m_bindings[key] = binding;
    emit bindingCreated(binding);
    
    // Binding created successfully
    
    return binding;
}

void VariableBindingManager::removeBinding(const QString& elementId, const QString& propertyName)
{
    QString key = makeKey(elementId, propertyName);
    
    if (auto* binding = m_bindings.take(key)) {
        binding->unbind();
        binding->deleteLater();
        emit bindingRemoved(elementId, propertyName);
    }
}

VariableBinding* VariableBindingManager::getBinding(const QString& elementId, const QString& propertyName) const
{
    QString key = makeKey(elementId, propertyName);
    return m_bindings.value(key, nullptr);
}

bool VariableBindingManager::hasBinding(const QString& elementId, const QString& propertyName) const
{
    return getBinding(elementId, propertyName) != nullptr;
}

QString VariableBindingManager::getBoundVariableId(const QString& elementId, const QString& propertyName) const
{
    if (auto* binding = getBinding(elementId, propertyName)) {
        return binding->variableId();
    }
    return QString();
}

QList<VariableBinding*> VariableBindingManager::getBindingsForElement(const QString& elementId) const
{
    QList<VariableBinding*> result;
    for (auto it = m_bindings.constBegin(); it != m_bindings.constEnd(); ++it) {
        if (it.value()->elementId() == elementId) {
            result.append(it.value());
        }
    }
    return result;
}

QList<VariableBinding*> VariableBindingManager::getBindingsForVariable(const QString& variableId) const
{
    QList<VariableBinding*> result;
    for (auto it = m_bindings.constBegin(); it != m_bindings.constEnd(); ++it) {
        if (it.value()->variableId() == variableId) {
            result.append(it.value());
        }
    }
    return result;
}

void VariableBindingManager::clearAllBindings()
{
    for (auto it = m_bindings.constBegin(); it != m_bindings.constEnd(); ++it) {
        it.value()->unbind();
        it.value()->deleteLater();
    }
    m_bindings.clear();
}

QVariantList VariableBindingManager::serialize() const
{
    QVariantList result;
    for (auto it = m_bindings.constBegin(); it != m_bindings.constEnd(); ++it) {
        QVariantMap bindingData;
        bindingData["variableId"] = it.value()->variableId();
        bindingData["elementId"] = it.value()->elementId();
        bindingData["propertyName"] = it.value()->propertyName();
        result.append(bindingData);
    }
    return result;
}

void VariableBindingManager::deserialize(const QVariantList& data)
{
    clearAllBindings();
    
    for (const QVariant& item : data) {
        QVariantMap bindingData = item.toMap();
        QString variableId = bindingData["variableId"].toString();
        QString elementId = bindingData["elementId"].toString();
        QString propertyName = bindingData["propertyName"].toString();
        
        if (!variableId.isEmpty() && !elementId.isEmpty() && !propertyName.isEmpty()) {
            createBinding(variableId, elementId, propertyName);
        }
    }
}

QString VariableBindingManager::makeKey(const QString& elementId, const QString& propertyName) const
{
    return elementId + "::" + propertyName;
}