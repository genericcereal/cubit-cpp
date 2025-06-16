#ifndef APPLICATION_H
#define APPLICATION_H

#include <QObject>
#include <QQmlEngine>
#include <memory>
#include <vector>
#include "Canvas.h"
#include "Panels.h"

class Application : public QObject {
    Q_OBJECT
    Q_PROPERTY(QString activeCanvasId READ activeCanvasId WRITE setActiveCanvasId NOTIFY activeCanvasIdChanged)
    Q_PROPERTY(Canvas* activeCanvas READ activeCanvas NOTIFY activeCanvasChanged)
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
    Canvas* activeCanvas() const;
    QStringList canvasIds() const;
    QStringList canvasNames() const;
    Panels* panels() const;

    // Property setters
    void setActiveCanvasId(const QString& canvasId);

signals:
    void activeCanvasIdChanged();
    void activeCanvasChanged();
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
};

#endif // APPLICATION_H