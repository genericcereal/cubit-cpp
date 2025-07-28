#ifndef THROTTLEDUPDATE_H
#define THROTTLEDUPDATE_H

#include <QObject>
#include <QTimer>
#include <functional>
#include <memory>
#include "Config.h"

class AdaptiveThrottler;

/**
 * A reusable class for throttling updates during high-frequency operations.
 * This improves performance by batching rapid updates at a controlled rate.
 * 
 * Usage:
 * 1. Create a ThrottledUpdate instance
 * 2. Connect to the update() signal or provide a callback
 * 3. Call requestUpdate() whenever an update is needed
 * 4. Updates will be batched and emitted at the specified interval
 */
class ThrottledUpdate : public QObject
{
    Q_OBJECT
    
public:
    explicit ThrottledUpdate(QObject* parent = nullptr, int interval = Config::THROTTLE_INTERVAL);
    ~ThrottledUpdate() = default;
    
    // Properties
    void setInterval(int interval);
    int interval() const { return m_interval; }
    
    void setActive(bool active);
    bool isActive() const { return m_active; }
    
    // Adaptive throttling
    void setAdaptiveThrottler(AdaptiveThrottler* throttler);
    AdaptiveThrottler* adaptiveThrottler() const { return m_adaptiveThrottler; }
    void setAdaptiveMode(bool enabled);
    bool adaptiveMode() const { return m_adaptiveMode; }
    
    // Request an update (will be throttled)
    void requestUpdate();
    
    // Force immediate update (useful for final positions)
    void forceUpdate();
    
    // Set a callback function instead of using signals
    void setUpdateCallback(std::function<void()> callback);
    
signals:
    void update();
    
private slots:
    void performUpdate();
    
private:
    void updateTimerInterval();
    
    QTimer* m_timer;
    int m_interval;
    bool m_active;
    bool m_hasPendingUpdate;
    bool m_adaptiveMode;
    std::function<void()> m_updateCallback;
    AdaptiveThrottler* m_adaptiveThrottler;
};

#endif // THROTTLEDUPDATE_H