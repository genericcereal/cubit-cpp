import QtQuick
import Cubit 1.0

/**
 * QML wrapper for AdaptiveThrottler that provides adaptive FPS-based throttling
 * This component monitors actual performance and adjusts throttle intervals dynamically
 */
Item {
    id: root
    
    // Properties
    property int targetFps: ConfigObject.targetFps
    property bool adaptive: ConfigObject.adaptiveThrottlingDefault
    property bool active: false
    
    // Read-only properties
    readonly property int currentFps: throttler.currentFps
    readonly property int currentInterval: throttler.currentInterval
    
    // Signals
    signal update(var data)
    
    // Internal state
    property var pendingData: null
    property bool hasPendingUpdate: false
    
    // C++ AdaptiveThrottler instance
    AdaptiveThrottler {
        id: throttler
        targetFps: root.targetFps
        adaptive: root.adaptive
        
        onIntervalAdjusted: function(newInterval) {
            throttleTimer.interval = newInterval
        }
    }
    
    // Timer for throttling with dynamic interval
    Timer {
        id: throttleTimer
        interval: throttler.currentInterval
        repeat: true
        running: root.active && root.hasPendingUpdate
        
        onTriggered: {
            if (hasPendingUpdate && pendingData !== null) {
                throttler.recordUpdate()
                root.update(pendingData)
                hasPendingUpdate = false
                pendingData = null
            }
        }
    }
    
    // Function to request an update
    function requestUpdate(data) {
        pendingData = data
        hasPendingUpdate = true
        
        // If timer is not running and we're active, trigger immediately
        if (active && !throttleTimer.running) {
            throttleTimer.start()
            throttleTimer.triggered()
        }
    }
    
    // Function to force immediate update (useful for final positions)
    function forceUpdate() {
        if (hasPendingUpdate && pendingData !== null) {
            throttleTimer.stop()
            throttler.recordUpdate()
            root.update(pendingData)
            hasPendingUpdate = false
            pendingData = null
        }
    }
    
    // Reset statistics
    function reset() {
        throttler.reset()
    }
    
    // Clean up when deactivated
    onActiveChanged: {
        if (!active) {
            forceUpdate() // Ensure final update is sent
            throttleTimer.stop()
            throttler.reset()
        }
    }
}