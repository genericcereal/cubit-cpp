import QtQuick
import Cubit.UI 1.0

// A reusable component for throttling updates during drag operations
// This improves performance by batching rapid mouse move events
Item {
    id: root
    
    // Properties
    property int interval: Config.throttleInterval // Use global throttle interval from Config
    property bool active: false
    
    // Signals
    signal update(var data)
    
    // Internal state
    property var pendingData: null
    property bool hasPendingUpdate: false
    
    // Timer for throttling
    Timer {
        id: throttleTimer
        interval: root.interval
        repeat: true
        running: root.active && root.hasPendingUpdate
        
        onTriggered: {
            if (hasPendingUpdate && pendingData !== null) {
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
            root.update(pendingData)
            hasPendingUpdate = false
            pendingData = null
        }
    }
    
    // Clean up when deactivated
    onActiveChanged: {
        if (!active) {
            forceUpdate() // Ensure final update is sent
            throttleTimer.stop()
        }
    }
}