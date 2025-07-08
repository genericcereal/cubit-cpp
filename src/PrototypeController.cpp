#include "PrototypeController.h"
#include "ElementModel.h"
#include "SelectionManager.h"
#include "ConsoleMessageRepository.h"

PrototypeController::PrototypeController(ElementModel& model,
                                       SelectionManager& sel,
                                       QObject *parent)
    : QObject(parent)
    , m_elementModel(model)
    , m_selectionManager(sel)
{
    // Initialize default viewable area for Web mode
    m_viewableArea = calculateViewportForMode("Web");
}

void PrototypeController::setIsPrototyping(bool value) {
    if (m_isPrototyping != value) {
        m_isPrototyping = value;
        emit isPrototypingChanged();
        
        if (value) {
            emit prototypingStarted();
        } else {
            emit prototypingStopped();
        }
    }
}

void PrototypeController::setViewableArea(const QRectF& area) {
    if (m_viewableArea != area) {
        m_viewableArea = area;
        emit viewableAreaChanged();
    }
}

void PrototypeController::setPrototypeMode(const QString& mode) {
    if (m_prototypeMode != mode) {
        m_prototypeMode = mode;
        
        // Update viewable area based on new mode
        m_viewableArea = calculateViewportForMode(mode);
        
        emit prototypeModeChanged();
        emit viewableAreaChanged();
    }
}

void PrototypeController::setPrototypingStartPosition(const QPointF& position) {
    if (m_prototypingStartPosition != position) {
        m_prototypingStartPosition = position;
        emit prototypingStartPositionChanged();
    }
}

void PrototypeController::setPrototypingStartZoom(qreal zoom) {
    if (m_prototypingStartZoom != zoom) {
        m_prototypingStartZoom = zoom;
        emit prototypingStartZoomChanged();
    }
}

void PrototypeController::startPrototyping(const QPointF& canvasCenter, qreal currentZoom) {
    // Store the current position and zoom for returning later
    setPrototypingStartPosition(canvasCenter);
    setPrototypingStartZoom(currentZoom);
    
    // Start prototyping
    setIsPrototyping(true);
    
    ConsoleMessageRepository::instance()->addInfo(
        QString("Started prototyping in %1 mode").arg(m_prototypeMode)
    );
}

void PrototypeController::stopPrototyping() {
    setIsPrototyping(false);
    
    // Clear the start position
    setPrototypingStartPosition(QPointF(0, 0));
    setPrototypingStartZoom(1.0);
    
    ConsoleMessageRepository::instance()->addInfo("Stopped prototyping");
}

QRectF PrototypeController::calculateViewportForMode(const QString& mode) const {
    if (mode == "Mobile") {
        return QRectF(0, 0, MOBILE_WIDTH, MOBILE_HEIGHT);
    } else {
        // Default to Web
        return QRectF(0, 0, WEB_WIDTH, WEB_HEIGHT);
    }
}

bool PrototypeController::isElementInViewableArea(qreal x, qreal y, qreal width, qreal height) const {
    if (!m_isPrototyping) {
        return true; // All elements are viewable when not prototyping
    }
    
    QRectF elementRect(x, y, width, height);
    return m_viewableArea.intersects(elementRect);
}