#ifndef PANELS_H
#define PANELS_H

#include <QObject>
#include <QQmlEngine>

class Panels : public QObject {
    Q_OBJECT
    Q_PROPERTY(bool detailPanelVisible READ isDetailPanelVisible WRITE setDetailPanelVisible NOTIFY detailPanelVisibleChanged)
    Q_PROPERTY(bool actionsPanelVisible READ isActionsPanelVisible WRITE setActionsPanelVisible NOTIFY actionsPanelVisibleChanged)
    Q_PROPERTY(bool fpsMonitorVisible READ isFpsMonitorVisible WRITE setFpsMonitorVisible NOTIFY fpsMonitorVisibleChanged)

public:
    explicit Panels(QObject *parent = nullptr);
    ~Panels() = default;

    // Panel visibility getters
    bool isDetailPanelVisible() const;
    bool isActionsPanelVisible() const;
    bool isFpsMonitorVisible() const;

    // Panel visibility setters
    void setDetailPanelVisible(bool visible);
    void setActionsPanelVisible(bool visible);
    void setFpsMonitorVisible(bool visible);

    // Panel management
    Q_INVOKABLE void toggleDetailPanel();
    Q_INVOKABLE void toggleActionsPanel();
    Q_INVOKABLE void toggleFpsMonitor();
    Q_INVOKABLE void showAllPanels();
    Q_INVOKABLE void hideAllPanels();

signals:
    void detailPanelVisibleChanged();
    void actionsPanelVisibleChanged();
    void fpsMonitorVisibleChanged();

private:
    bool m_detailPanelVisible = true;
    bool m_actionsPanelVisible = true;
    bool m_fpsMonitorVisible = true;
};

#endif // PANELS_H