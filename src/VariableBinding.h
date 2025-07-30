#ifndef VARIABLEBINDING_H
#define VARIABLEBINDING_H

#include <QObject>
#include <QString>
#include <QVariant>
#include <QMap>
#include <QSet>

class Variable;
class Element;

class VariableBinding : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QString variableId READ variableId NOTIFY variableIdChanged)
    Q_PROPERTY(QString elementId READ elementId NOTIFY elementIdChanged)
    Q_PROPERTY(QString propertyName READ propertyName NOTIFY propertyNameChanged)
    Q_PROPERTY(bool isActive READ isActive NOTIFY isActiveChanged)

public:
    explicit VariableBinding(QObject *parent = nullptr);
    
    // Create a binding between a variable and an element property
    void bind(const QString& variableId, const QString& elementId, const QString& propertyName);
    
    // Remove the binding
    void unbind();
    
    // Getters
    QString variableId() const { return m_variableId; }
    QString elementId() const { return m_elementId; }
    QString propertyName() const { return m_propertyName; }
    bool isActive() const { return m_isActive; }
    
    // Update the element property value from the variable
    void updateValue();

signals:
    void variableIdChanged();
    void elementIdChanged();
    void propertyNameChanged();
    void isActiveChanged();
    void valueUpdated(const QVariant& newValue);

private slots:
    void onVariableValueChanged();

private:
    QString m_variableId;
    QString m_elementId;
    QString m_propertyName;
    bool m_isActive = false;
    
    Variable* m_variable = nullptr;
    Element* m_element = nullptr;
    
    void connectToVariable();
    void disconnectFromVariable();
};

// Manager class to handle all bindings in a project
class VariableBindingManager : public QObject
{
    Q_OBJECT

public:
    explicit VariableBindingManager(QObject *parent = nullptr);
    ~VariableBindingManager();
    
    // Create a new binding
    Q_INVOKABLE VariableBinding* createBinding(const QString& variableId, 
                                               const QString& elementId, 
                                               const QString& propertyName);
    
    // Remove a binding
    Q_INVOKABLE void removeBinding(const QString& elementId, const QString& propertyName);
    
    // Get binding for a specific element property
    Q_INVOKABLE VariableBinding* getBinding(const QString& elementId, const QString& propertyName) const;
    
    // Check if a property has a binding
    Q_INVOKABLE bool hasBinding(const QString& elementId, const QString& propertyName) const;
    
    // Get the variable ID bound to a property
    Q_INVOKABLE QString getBoundVariableId(const QString& elementId, const QString& propertyName) const;
    
    // Get all bindings for an element
    QList<VariableBinding*> getBindingsForElement(const QString& elementId) const;
    
    // Get all bindings for a variable
    QList<VariableBinding*> getBindingsForVariable(const QString& variableId) const;
    
    // Clear all bindings
    void clearAllBindings();
    
    // Serialization
    QVariantList serialize() const;
    void deserialize(const QVariantList& data);

signals:
    void bindingCreated(VariableBinding* binding);
    void bindingRemoved(const QString& elementId, const QString& propertyName);

private:
    // Map of elementId+propertyName -> VariableBinding
    QMap<QString, VariableBinding*> m_bindings;
    
    QString makeKey(const QString& elementId, const QString& propertyName) const;
};

#endif // VARIABLEBINDING_H