#ifndef PANELS_H
#define PANELS_H

#include <QObject>
#include <QQmlEngine>

class Panels : public QObject {
    Q_OBJECT
    Q_PROPERTY(bool detailPanelVisible READ isDetailPanelVisible WRITE setDetailPanelVisible NOTIFY detailPanelVisibleChanged)
    Q_PROPERTY(bool actionsPanelVisible READ isActionsPanelVisible WRITE setActionsPanelVisible NOTIFY actionsPanelVisibleChanged)
    Q_PROPERTY(bool prototypePanelVisible READ isPrototypePanelVisible WRITE setPrototypePanelVisible NOTIFY prototypePanelVisibleChanged)

public:
    explicit Panels(QObject *parent = nullptr);
    ~Panels() = default;

    // Panel visibility getters
    bool isDetailPanelVisible() const;
    bool isActionsPanelVisible() const;
    bool isPrototypePanelVisible() const;

    // Panel visibility setters
    void setDetailPanelVisible(bool visible);
    void setActionsPanelVisible(bool visible);
    void setPrototypePanelVisible(bool visible);

    // Panel management
    Q_INVOKABLE void toggleDetailPanel();
    Q_INVOKABLE void toggleActionsPanel();
    Q_INVOKABLE void togglePrototypePanel();
    Q_INVOKABLE void showAllPanels();
    Q_INVOKABLE void hideAllPanels();

signals:
    void detailPanelVisibleChanged();
    void actionsPanelVisibleChanged();
    void prototypePanelVisibleChanged();

private:
    bool m_detailPanelVisible = true;
    bool m_actionsPanelVisible = true;
    bool m_prototypePanelVisible = false;
};

#endif // PANELS_H