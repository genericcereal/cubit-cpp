import QtQuick 2.15
import QtQuick.Controls 2.15
import Cubit 1.0
import Cubit.UI 1.0

Item {
    id: root
    
    width: viewableArea.width
    height: viewableArea.height + 50  // Extra height to accommodate dropdown above
    
    // Position with fixed top edge location
    anchors.horizontalCenter: parent.horizontalCenter
    y: {
        // Calculate a fixed top position that won't change with mode
        var dropdownHeight = 50  // 30px dropdown + margins
        var fixedTopPosition = (parent.height - 450) / 2  // Use Web height (400) + dropdown as baseline
        return fixedTopPosition
    }
    
    Component.onCompleted: {
    }
    
    onVisibleChanged: {
    }
    
    // Properties passed from ViewportOverlay
    property var designCanvas: null
    property var flickable: null
    property var prototypeController: null
    
    // Prototyping mode dropdown - positioned above the viewable area
    ComboBox {
        id: prototypingModeDropdown
        width: 100
        height: 30
        z: 2  // Higher z-order
        
        model: ["Web", "Mobile"]
        currentIndex: root.prototypeController && root.prototypeController.prototypeMode === "Mobile" ? 1 : 0
        
        // Position above the viewable area
        anchors.horizontalCenter: parent.horizontalCenter
        anchors.bottom: viewableArea.top
        anchors.bottomMargin: 10
        
        onCurrentTextChanged: {
            if (root.prototypeController && currentText !== root.prototypeController.prototypeMode) {
                root.prototypeController.prototypeMode = currentText
            }
        }
    }
    
    // The actual viewable area
    Rectangle {
        id: viewableArea
        
        width: root.prototypeController ? root.prototypeController.viewableArea.width : 400
        height: root.prototypeController ? root.prototypeController.viewableArea.height : 400
        color: "red"
        opacity: 0.2  // Making it transparent
        
        // Position below the dropdown
        anchors.bottom: parent.bottom
        anchors.horizontalCenter: parent.horizontalCenter
    }
    
    // Stop button positioned to the right of the viewable area
    Rectangle {
        id: inlinePrototypeStop
        width: 20
        height: 20
        color: Config.controlResizeJointColor
        antialiasing: true
        z: 2
        
        // Position to the right of the viewable area
        anchors.left: viewableArea.right
        anchors.leftMargin: 10
        anchors.verticalCenter: viewableArea.verticalCenter
        
        // Display "S" for Stop
        Text {
            anchors.centerIn: parent
            text: "S"
            font.pixelSize: 12
            font.family: "Arial"
            font.bold: true
            color: "white"
        }
        
        MouseArea {
            anchors.fill: parent
            cursorShape: Qt.PointingHandCursor
            
            onClicked: (mouse) => {
                if (!root.prototypeController) {
                    return
                }
                
                if (!root.designCanvas) {
                    return
                }
                
                // Get the snapshot values from prototype controller BEFORE stopping
                var startPos = root.prototypeController.getSnapshotCanvasPosition()
                var startZoom = root.prototypeController.getSnapshotCanvasZoom()
                
                // First restore all element positions (without animation)
                root.prototypeController.restoreElementPositionsFromSnapshot()
                
                // Stop prototyping mode through the controller BEFORE animating
                root.prototypeController.stopPrototyping()
                
                // Then animate back to the canvas position if we have one
                if (startPos && (startPos.x !== 0 || startPos.y !== 0)) {
                    // First restore the zoom level
                    root.designCanvas.zoom = startZoom
                    
                    // Then move to the original position without animation
                    // The moveToPoint function will set isAnimating = false
                    root.designCanvas.moveToPoint(startPos, false)
                }
                
                mouse.accepted = true
            }
        }
    }
}