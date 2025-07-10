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

bool Panels::isPrototypePanelVisible() const {
    return m_prototypePanelVisible;
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

void Panels::setPrototypePanelVisible(bool visible) {
    if (m_prototypePanelVisible != visible) {
        m_prototypePanelVisible = visible;
        emit prototypePanelVisibleChanged();
    }
}

// Panel management methods
void Panels::toggleDetailPanel() {
    setDetailPanelVisible(!m_detailPanelVisible);
}

void Panels::toggleActionsPanel() {
    setActionsPanelVisible(!m_actionsPanelVisible);
}

void Panels::togglePrototypePanel() {
    setPrototypePanelVisible(!m_prototypePanelVisible);
}

void Panels::showAllPanels() {
    setDetailPanelVisible(true);
    setActionsPanelVisible(true);
    setPrototypePanelVisible(true);
}

void Panels::hideAllPanels() {
    setDetailPanelVisible(false);
    setActionsPanelVisible(false);
    setPrototypePanelVisible(false);
}