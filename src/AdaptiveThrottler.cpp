#include "AdaptiveThrottler.h"
#include <algorithm>

AdaptiveThrottler::AdaptiveThrottler(QObject* parent)
    : QObject(parent)
{
    m_performanceTimer.start();
}

void AdaptiveThrottler::setTargetFps(int fps)
{
    if (m_targetFps != fps && fps > 0 && fps <= 120) {
        m_targetFps = fps;
        m_currentInterval = 1000 / fps; // Initial interval based on target
        emit targetFpsChanged();
        emit currentIntervalChanged();
    }
}

void AdaptiveThrottler::setAdaptive(bool adaptive)
{
    if (m_adaptive != adaptive) {
        m_adaptive = adaptive;
        if (!adaptive) {
            // Reset to target interval when disabling adaptive mode
            m_currentInterval = 1000 / m_targetFps;
            emit currentIntervalChanged();
        }
        emit adaptiveChanged();
    }
}

void AdaptiveThrottler::recordUpdate()
{
    auto now = std::chrono::steady_clock::now();
    m_updateTimes.push_back(now);
    
    // Keep only recent samples
    while (m_updateTimes.size() > SAMPLE_WINDOW) {
        m_updateTimes.pop_front();
    }
    
    // Calculate FPS and adjust if we have enough samples
    if (m_updateTimes.size() >= 2) {
        calculateFps();
        if (m_adaptive) {
            adjustInterval();
        }
    }
}

void AdaptiveThrottler::calculateFps()
{
    if (m_updateTimes.size() < 2) {
        return;
    }
    
    // Calculate average FPS over the sample window
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(
        m_updateTimes.back() - m_updateTimes.front()
    ).count();
    
    if (duration > 0) {
        int newFps = static_cast<int>((m_updateTimes.size() - 1) * 1000.0 / duration);
        if (newFps != m_currentFps) {
            m_currentFps = newFps;
            emit currentFpsChanged();
        }
    }
}

void AdaptiveThrottler::adjustInterval()
{
    if (m_currentFps == 0 || !m_adaptive) {
        return;
    }
    
    // Calculate the error between current and target FPS
    double error = static_cast<double>(m_targetFps - m_currentFps) / m_targetFps;
    
    // If we're within 10% of target, don't adjust
    if (std::abs(error) < 0.1) {
        return;
    }
    
    // Calculate new interval
    // If FPS is too low, decrease interval (increase frequency)
    // If FPS is too high, increase interval (decrease frequency)
    double adjustment = m_currentInterval * error * ADJUSTMENT_FACTOR;
    int newInterval = static_cast<int>(m_currentInterval + adjustment);
    
    // Clamp to reasonable bounds
    newInterval = std::clamp(newInterval, MIN_INTERVAL, MAX_INTERVAL);
    
    // Only update if the change is significant (at least 1ms)
    if (std::abs(newInterval - m_currentInterval) >= 1) {
        m_currentInterval = newInterval;
        emit currentIntervalChanged();
        emit intervalAdjusted(newInterval);
    }
}

int AdaptiveThrottler::recommendedInterval() const
{
    return m_currentInterval;
}

void AdaptiveThrottler::reset()
{
    m_updateTimes.clear();
    m_currentFps = 0;
    m_currentInterval = 1000 / m_targetFps;
    emit currentFpsChanged();
    emit currentIntervalChanged();
}