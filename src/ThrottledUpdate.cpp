#include "ThrottledUpdate.h"

ThrottledUpdate::ThrottledUpdate(QObject* parent, int interval)
    : QObject(parent)
    , m_interval(interval)
    , m_active(true)
    , m_hasPendingUpdate(false)
{
    m_timer = new QTimer(this);
    m_timer->setInterval(m_interval);
    m_timer->setSingleShot(false); // Repeat mode
    
    connect(m_timer, &QTimer::timeout, this, &ThrottledUpdate::performUpdate);
}

void ThrottledUpdate::setInterval(int interval)
{
    if (m_interval != interval) {
        m_interval = interval;
        m_timer->setInterval(m_interval);
    }
}

void ThrottledUpdate::setActive(bool active)
{
    if (m_active != active) {
        m_active = active;
        
        if (!m_active) {
            // When deactivating, ensure final update is sent
            forceUpdate();
            m_timer->stop();
        }
    }
}

void ThrottledUpdate::requestUpdate()
{
    m_hasPendingUpdate = true;
    
    // If timer is not running and we're active, start it
    if (m_active && !m_timer->isActive()) {
        m_timer->start();
        // Trigger immediately for the first update
        performUpdate();
    }
}

void ThrottledUpdate::forceUpdate()
{
    if (m_hasPendingUpdate) {
        m_timer->stop();
        performUpdate();
    }
}

void ThrottledUpdate::setUpdateCallback(std::function<void()> callback)
{
    m_updateCallback = callback;
}

void ThrottledUpdate::performUpdate()
{
    if (m_hasPendingUpdate) {
        m_hasPendingUpdate = false;
        
        // Call callback if set
        if (m_updateCallback) {
            m_updateCallback();
        }
        
        // Emit signal
        emit update();
        
        // Stop timer if no more updates pending
        if (!m_hasPendingUpdate && m_timer->isActive()) {
            m_timer->stop();
        }
    }
}