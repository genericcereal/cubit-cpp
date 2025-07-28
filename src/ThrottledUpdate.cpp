#include "ThrottledUpdate.h"
#include "AdaptiveThrottler.h"

ThrottledUpdate::ThrottledUpdate(QObject* parent, int interval)
    : QObject(parent)
    , m_interval(interval)
    , m_active(true)
    , m_hasPendingUpdate(false)
    , m_adaptiveMode(false)
    , m_adaptiveThrottler(nullptr)
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

void ThrottledUpdate::setAdaptiveThrottler(AdaptiveThrottler* throttler)
{
    if (m_adaptiveThrottler) {
        disconnect(m_adaptiveThrottler, nullptr, this, nullptr);
    }
    
    m_adaptiveThrottler = throttler;
    
    if (m_adaptiveThrottler) {
        connect(m_adaptiveThrottler, &AdaptiveThrottler::intervalAdjusted,
                this, &ThrottledUpdate::updateTimerInterval);
    }
}

void ThrottledUpdate::setAdaptiveMode(bool enabled)
{
    if (m_adaptiveMode != enabled) {
        m_adaptiveMode = enabled;
        updateTimerInterval();
    }
}

void ThrottledUpdate::updateTimerInterval()
{
    if (m_adaptiveMode && m_adaptiveThrottler) {
        int newInterval = m_adaptiveThrottler->recommendedInterval();
        if (newInterval != m_timer->interval()) {
            m_timer->setInterval(newInterval);
        }
    } else {
        m_timer->setInterval(m_interval);
    }
}

void ThrottledUpdate::performUpdate()
{
    if (m_hasPendingUpdate) {
        m_hasPendingUpdate = false;
        
        // Record update in adaptive throttler if enabled
        if (m_adaptiveMode && m_adaptiveThrottler) {
            m_adaptiveThrottler->recordUpdate();
        }
        
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