#ifndef CANVAS_H
#define CANVAS_H

#include <QObject>
#include <QString>
#include <memory>
#include "CanvasController.h"
#include "SelectionManager.h"
#include "ElementModel.h"
#include "Scripts.h"
#include "DesignElement.h"

class Canvas : public QObject {
    Q_OBJECT
    Q_PROPERTY(CanvasController* controller READ controller CONSTANT)
    Q_PROPERTY(SelectionManager* selectionManager READ selectionManager CONSTANT)
    Q_PROPERTY(ElementModel* elementModel READ elementModel CONSTANT)
    Q_PROPERTY(Scripts* scripts READ scripts CONSTANT)
    Q_PROPERTY(QString name READ name WRITE setName NOTIFY nameChanged)
    Q_PROPERTY(QString id READ id CONSTANT)
    Q_PROPERTY(QString viewMode READ viewMode WRITE setViewMode NOTIFY viewModeChanged)
    Q_PROPERTY(DesignElement* editingElement READ editingElement NOTIFY editingElementChanged)
    Q_PROPERTY(Scripts* activeScripts READ activeScripts NOTIFY activeScriptsChanged)

public:
    explicit Canvas(const QString& id, const QString& name = QString(), QObject *parent = nullptr);
    ~Canvas();

    // Property getters
    CanvasController* controller() const;
    SelectionManager* selectionManager() const;
    ElementModel* elementModel() const;
    Scripts* scripts() const;
    QString name() const;
    QString id() const;
    QString viewMode() const;
    DesignElement* editingElement() const;
    Scripts* activeScripts() const;

    // Property setters
    void setName(const QString& name);
    void setViewMode(const QString& viewMode);
    void setEditingElement(DesignElement* element);

    // Initialize the canvas components
    void initialize();

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
    QString m_name;
    QString m_id;
    QString m_viewMode;
    DesignElement* m_editingElement = nullptr;
    
    void updateActiveScripts();
    void loadScriptsIntoElementModel();
    void saveElementModelToScripts();
    void clearScriptElementsFromModel();
};

#endif // CANVAS_H