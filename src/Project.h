#ifndef PROJECT_H
#define PROJECT_H

#include <QObject>
#include <QString>
#include <memory>
#include "CanvasController.h"
#include "SelectionManager.h"
#include "ElementModel.h"
#include "Scripts.h"
#include "DesignElement.h"
#include "ScriptExecutor.h"

class Component;
Q_DECLARE_OPAQUE_POINTER(Component*)

class Project : public QObject {
    Q_OBJECT
    Q_PROPERTY(CanvasController* controller READ controller CONSTANT)
    Q_PROPERTY(SelectionManager* selectionManager READ selectionManager CONSTANT)
    Q_PROPERTY(ElementModel* elementModel READ elementModel CONSTANT)
    Q_PROPERTY(Scripts* scripts READ scripts CONSTANT)
    Q_PROPERTY(QString name READ name WRITE setName NOTIFY nameChanged)
    Q_PROPERTY(QString id READ id CONSTANT)
    Q_PROPERTY(QString viewMode READ viewMode WRITE setViewMode NOTIFY viewModeChanged)
    Q_PROPERTY(QObject* editingElement READ editingElement NOTIFY editingElementChanged)
    Q_PROPERTY(Scripts* activeScripts READ activeScripts NOTIFY activeScriptsChanged)

public:
    explicit Project(const QString& id, const QString& name = QString(), QObject *parent = nullptr);
    ~Project();

    // Property getters
    CanvasController* controller() const;
    SelectionManager* selectionManager() const;
    ElementModel* elementModel() const;
    Scripts* scripts() const;
    QString name() const;
    QString id() const;
    QString viewMode() const;
    QObject* editingElement() const;
    Scripts* activeScripts() const;

    // Property setters
    void setName(const QString& name);
    void setViewMode(const QString& viewMode);
    Q_INVOKABLE void setEditingElement(DesignElement* element, const QString& viewMode = QString());
    Q_INVOKABLE void setEditingComponent(Component* component, const QString& viewMode = QString());

    // Initialize the canvas components
    void initialize();
    
    // Execute a script event
    Q_INVOKABLE void executeScriptEvent(const QString& eventName);

signals:
    void nameChanged();
    void viewModeChanged();
    void editingElementChanged();
    void activeScriptsChanged();

private:
    std::unique_ptr<CanvasController> m_controller;
    std::unique_ptr<SelectionManager> m_selectionManager;
    std::unique_ptr<ElementModel> m_elementModel;
    std::unique_ptr<Scripts> m_scripts;
    std::unique_ptr<ScriptExecutor> m_scriptExecutor;
    QString m_name;
    QString m_id;
    QString m_viewMode;
    QObject* m_editingElement = nullptr; // Can be DesignElement* or Component*
    
    void updateActiveScripts();
    void loadScriptsIntoElementModel();
    void saveElementModelToScripts();
    void clearScriptElementsFromModel();
};

#endif // PROJECT_H