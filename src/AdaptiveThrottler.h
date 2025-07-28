#ifndef ADAPTIVETHROTTLER_H
#define ADAPTIVETHROTTLER_H

#include <QObject>
#include <QElapsedTimer>
#include <deque>
#include <chrono>
#include "Config.h"

/**
 * AdaptiveThrottler dynamically adjusts throttling interval based on actual FPS
 * to maintain a target frame rate. It monitors performance and adapts the 
 * interval to achieve optimal performance.
 */
class AdaptiveThrottler : public QObject
{
    Q_OBJECT
    Q_PROPERTY(int targetFps READ targetFps WRITE setTargetFps NOTIFY targetFpsChanged)
    Q_PROPERTY(int currentFps READ currentFps NOTIFY currentFpsChanged)
    Q_PROPERTY(int currentInterval READ currentInterval NOTIFY currentIntervalChanged)
    Q_PROPERTY(bool adaptive READ isAdaptive WRITE setAdaptive NOTIFY adaptiveChanged)
    
public:
    explicit AdaptiveThrottler(QObject* parent = nullptr);
    ~AdaptiveThrottler() = default;
    
    // Target FPS management
    int targetFps() const { return m_targetFps; }
    void setTargetFps(int fps);
    
    // Current performance metrics
    int currentFps() const { return m_currentFps; }
    int currentInterval() const { return m_currentInterval; }
    
    // Adaptive mode
    bool isAdaptive() const { return m_adaptive; }
    void setAdaptive(bool adaptive);
    
    // Record an update event
    void recordUpdate();
    
    // Get the current recommended interval
    int recommendedInterval() const;
    
    // Reset statistics
    void reset();
    
signals:
    void targetFpsChanged();
    void currentFpsChanged();
    void currentIntervalChanged();
    void adaptiveChanged();
    void intervalAdjusted(int newInterval);
    
private:
    void calculateFps();
    void adjustInterval();
    
    // Configuration
    int m_targetFps = Config::TARGET_FPS;
    bool m_adaptive = Config::ADAPTIVE_THROTTLING_DEFAULT;
    
    // Current state
    int m_currentFps = 0;
    int m_currentInterval = Config::THROTTLE_INTERVAL;
    
    // FPS calculation
    std::deque<std::chrono::steady_clock::time_point> m_updateTimes;
    QElapsedTimer m_performanceTimer;
    
    // Adjustment parameters
    static constexpr int MIN_INTERVAL = 8;   // 125fps max
    static constexpr int MAX_INTERVAL = 100; // 10fps min
    static constexpr double ADJUSTMENT_FACTOR = 0.1; // How aggressively to adjust
    static constexpr size_t SAMPLE_WINDOW = 30; // Number of samples to average
};

#endif // ADAPTIVETHROTTLER_H