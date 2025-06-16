#ifndef APPLICATION_H
#define APPLICATION_H

#include <QObject>
#include <QQmlEngine>
#include <memory>
#include <vector>
#include "CanvasController.h"
#include "SelectionManager.h"
#include "ElementModel.h"
#include "Panels.h"
#include "Scripts.h"

struct Canvas {
    std::unique_ptr<CanvasController> controller;
    std::unique_ptr<SelectionManager> selectionManager;
    std::unique_ptr<ElementModel> elementModel;
    std::unique_ptr<Scripts> scripts;
    QString name;
    QString id;
    QString currentViewMode; // "design" or "script" - just the current view, not a canvas type
};

class Application : public QObject {
    Q_OBJECT
    Q_PROPERTY(QString activeCanvasId READ activeCanvasId WRITE setActiveCanvasId NOTIFY activeCanvasIdChanged)
    Q_PROPERTY(CanvasController* activeController READ activeController NOTIFY activeControllerChanged)
    Q_PROPERTY(SelectionManager* activeSelectionManager READ activeSelectionManager NOTIFY activeSelectionManagerChanged)
    Q_PROPERTY(ElementModel* activeElementModel READ activeElementModel NOTIFY activeElementModelChanged)
    Q_PROPERTY(Scripts* activeScripts READ activeScripts NOTIFY activeScriptsChanged)
    Q_PROPERTY(QString activeCanvasViewMode READ activeCanvasViewMode WRITE setActiveCanvasViewMode NOTIFY activeCanvasViewModeChanged)
    Q_PROPERTY(QStringList canvasIds READ canvasIds NOTIFY canvasListChanged)
    Q_PROPERTY(QStringList canvasNames READ canvasNames NOTIFY canvasListChanged)
    Q_PROPERTY(Panels* panels READ panels CONSTANT)

public:
    explicit Application(QObject *parent = nullptr);
    ~Application();

    static Application* instance();

    // Canvas management
    Q_INVOKABLE QString createCanvas(const QString& name = QString());
    Q_INVOKABLE void removeCanvas(const QString& canvasId);
    Q_INVOKABLE void switchToCanvas(const QString& canvasId);
    Q_INVOKABLE QString getCanvasName(const QString& canvasId) const;
    Q_INVOKABLE void renameCanvas(const QString& canvasId, const QString& newName);

    // Property getters
    QString activeCanvasId() const;
    CanvasController* activeController() const;
    SelectionManager* activeSelectionManager() const;
    ElementModel* activeElementModel() const;
    Scripts* activeScripts() const;
    QString activeCanvasViewMode() const;
    QStringList canvasIds() const;
    QStringList canvasNames() const;
    Panels* panels() const;

    // Property setters
    void setActiveCanvasId(const QString& canvasId);
    void setActiveCanvasViewMode(const QString& viewMode);

signals:
    void activeCanvasIdChanged();
    void activeControllerChanged();
    void activeSelectionManagerChanged();
    void activeElementModelChanged();
    void activeScriptsChanged();
    void activeCanvasViewModeChanged();
    void canvasListChanged();
    void canvasCreated(const QString& canvasId);
    void canvasRemoved(const QString& canvasId);

private:
    static Application* s_instance;
    std::vector<std::unique_ptr<Canvas>> m_canvases;
    QString m_activeCanvasId;
    std::unique_ptr<Panels> m_panels;
    
    Canvas* findCanvas(const QString& canvasId);
    const Canvas* findCanvas(const QString& canvasId) const;
    QString generateCanvasId() const;
    void initializeCanvas(Canvas* canvas);
};

#endif // APPLICATION_H