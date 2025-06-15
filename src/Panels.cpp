#include "Panels.h"

Panels::Panels(QObject *parent)
    : QObject(parent)
{
}

// Panel visibility getters
bool Panels::isDetailPanelVisible() const {
    return m_detailPanelVisible;
}

bool Panels::isActionsPanelVisible() const {
    return m_actionsPanelVisible;
}

bool Panels::isFpsMonitorVisible() const {
    return m_fpsMonitorVisible;
}

// Panel visibility setters
void Panels::setDetailPanelVisible(bool visible) {
    if (m_detailPanelVisible != visible) {
        m_detailPanelVisible = visible;
        emit detailPanelVisibleChanged();
    }
}

void Panels::setActionsPanelVisible(bool visible) {
    if (m_actionsPanelVisible != visible) {
        m_actionsPanelVisible = visible;
        emit actionsPanelVisibleChanged();
    }
}

void Panels::setFpsMonitorVisible(bool visible) {
    if (m_fpsMonitorVisible != visible) {
        m_fpsMonitorVisible = visible;
        emit fpsMonitorVisibleChanged();
    }
}

// Panel management methods
void Panels::toggleDetailPanel() {
    setDetailPanelVisible(!m_detailPanelVisible);
}

void Panels::toggleActionsPanel() {
    setActionsPanelVisible(!m_actionsPanelVisible);
}

void Panels::toggleFpsMonitor() {
    setFpsMonitorVisible(!m_fpsMonitorVisible);
}

void Panels::showAllPanels() {
    setDetailPanelVisible(true);
    setActionsPanelVisible(true);
    setFpsMonitorVisible(true);
}

void Panels::hideAllPanels() {
    setDetailPanelVisible(false);
    setActionsPanelVisible(false);
    setFpsMonitorVisible(false);
}