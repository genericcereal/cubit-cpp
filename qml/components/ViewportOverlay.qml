import QtQuick
import Cubit 1.0
import Cubit.UI 1.0

// ViewportOverlay provides a layer for UI elements that should maintain
// their visual size regardless of canvas zoom level
Item {
    id: root
    
    property var canvasView
    property real zoomLevel: canvasView ? canvasView.zoomLevel : 1.0
    property var flickable: canvasView ? canvasView.flickable : null
    property var hoveredElement: null
    property var selectionManager: canvasView ? canvasView.selectionManager : null
    property var selectedElements: selectionManager ? selectionManager.selectedElements : []
    property var creationDragHandler: canvasView ? canvasView.creationDragHandler : null
    property var controller: canvasView ? canvasView.controller : null
    
    // Selection box visual
    Rectangle {
        id: selectionBox
        visible: canvasView && canvasView.selectionBoxHandler.active
        color: "transparent"
        border.color: Config.selectionColor
        border.width: 1
        
        // Convert canvas coordinates to viewport coordinates
        x: {
            if (!canvasView || !canvasView.selectionBoxHandler.active) return 0
            var handler = canvasView.selectionBoxHandler
            return Math.min(handler.startPoint.x, handler.currentPoint.x) * zoomLevel - flickable.contentX
        }
        
        y: {
            if (!canvasView || !canvasView.selectionBoxHandler.active) return 0
            var handler = canvasView.selectionBoxHandler
            return Math.min(handler.startPoint.y, handler.currentPoint.y) * zoomLevel - flickable.contentY
        }
        
        width: {
            if (!canvasView || !canvasView.selectionBoxHandler.active) return 0
            var handler = canvasView.selectionBoxHandler
            return Math.abs(handler.currentPoint.x - handler.startPoint.x) * zoomLevel
        }
        
        height: {
            if (!canvasView || !canvasView.selectionBoxHandler.active) return 0
            var handler = canvasView.selectionBoxHandler
            return Math.abs(handler.currentPoint.y - handler.startPoint.y) * zoomLevel
        }
    }
    
    // Selection rectangles and handles
    Repeater {
        model: selectedElements
        
        Item {
            property var element: modelData
            
            // Control bars and joints
            Item {
                x: element ? (element.x * zoomLevel - flickable.contentX) : 0
                y: element ? (element.y * zoomLevel - flickable.contentY) : 0
                width: element ? (element.width * zoomLevel) : 0
                height: element ? (element.height * zoomLevel) : 0
                
                // Inner rectangle (yellow with 5% opacity)
                Rectangle {
                    anchors.fill: parent
                    color: Config.controlInnerRectColor
                    z: -1  // Behind other controls
                }
                
                // Top bar
                Rectangle {
                    x: 0
                    y: -Config.controlBarHeight/2
                    width: parent.width
                    height: Config.controlBarHeight
                    color: Config.controlBarColor
                    
                    // Center line
                    Rectangle {
                        anchors.centerIn: parent
                        width: parent.width
                        height: Config.controlLineWidth
                        color: Config.controlBarColor
                    }
                }
                
                // Bottom bar
                Rectangle {
                    x: 0
                    y: parent.height - Config.controlBarHeight/2
                    width: parent.width
                    height: Config.controlBarHeight
                    color: Config.controlBarColor
                    
                    // Center line
                    Rectangle {
                        anchors.centerIn: parent
                        width: parent.width
                        height: Config.controlLineWidth
                        color: Config.controlBarColor
                    }
                }
                
                // Left bar
                Rectangle {
                    x: -Config.controlBarWidth/2
                    y: 0
                    width: Config.controlBarWidth
                    height: parent.height
                    color: Config.controlBarColor
                    
                    // Center line
                    Rectangle {
                        anchors.centerIn: parent
                        width: Config.controlLineWidth
                        height: parent.height
                        color: Config.controlBarColor
                    }
                }
                
                // Right bar
                Rectangle {
                    x: parent.width - Config.controlBarWidth/2
                    y: 0
                    width: Config.controlBarWidth
                    height: parent.height
                    color: Config.controlBarColor
                    
                    // Center line
                    Rectangle {
                        anchors.centerIn: parent
                        width: Config.controlLineWidth
                        height: parent.height
                        color: Config.controlBarColor
                    }
                }
                
                // Joints container - only visible for single selection
                Item {
                    anchors.fill: parent
                    visible: selectedElements.length === 1
                    
                    // Rotation joints (blue, larger) - positioned with overlap
                    Repeater {
                        model: 4
                        Rectangle {
                            width: Config.controlRotationJointSize
                            height: Config.controlRotationJointSize
                            color: Config.controlRotationJointColor
                            
                            x: {
                                var overlap = Config.controlJointOverlap
                                switch(index) {
                                    case 0: return -Config.controlBarWidth/2 - (Config.controlRotationJointSize - overlap) // top-left
                                    case 1: return parent.width - Config.controlBarWidth/2 // top-right
                                    case 2: return parent.width - Config.controlBarWidth/2 // bottom-right
                                    case 3: return -Config.controlBarWidth/2 - (Config.controlRotationJointSize - overlap) // bottom-left
                                }
                            }
                            
                            y: {
                                var overlap = Config.controlJointOverlap
                                switch(index) {
                                    case 0: return -Config.controlBarHeight/2 - (Config.controlRotationJointSize - overlap) // top-left
                                    case 1: return -Config.controlBarHeight/2 - (Config.controlRotationJointSize - overlap) // top-right
                                    case 2: return parent.height - Config.controlBarHeight/2 // bottom-right
                                    case 3: return parent.height - Config.controlBarHeight/2 // bottom-left
                                }
                            }
                        }
                    }
                    
                    // Resize joints (yellow, smaller) - positioned at bar intersections
                    Repeater {
                        model: 4
                        Rectangle {
                            width: Config.controlResizeJointSize
                            height: Config.controlResizeJointSize
                            color: Config.controlResizeJointColor
                            
                            x: {
                                switch(index) {
                                    case 0: return -Config.controlBarWidth/2 // top-left
                                    case 1: return parent.width - Config.controlBarWidth/2 // top-right
                                    case 2: return parent.width - Config.controlBarWidth/2 // bottom-right
                                    case 3: return -Config.controlBarWidth/2 // bottom-left
                                }
                            }
                            
                            y: {
                                switch(index) {
                                    case 0: return -Config.controlBarHeight/2 // top-left
                                    case 1: return -Config.controlBarHeight/2 // top-right
                                    case 2: return parent.height - Config.controlBarHeight/2 // bottom-right
                                    case 3: return parent.height - Config.controlBarHeight/2 // bottom-left
                                }
                            }
                        }
                    }
                }
            }
        }
    }
    
    // Hover indicator
    Rectangle {
        id: hoverIndicator
        visible: hoveredElement !== null && 
                (!canvasView.selectionManager || 
                 hoveredElement && !hoveredElement.selected)
        color: "transparent"
        border.color: Config.hoverColor
        border.width: 1
        
        x: hoveredElement ? hoveredElement.x * zoomLevel - flickable.contentX : 0
        y: hoveredElement ? hoveredElement.y * zoomLevel - flickable.contentY : 0
        width: hoveredElement ? hoveredElement.width * zoomLevel : 0
        height: hoveredElement ? hoveredElement.height * zoomLevel : 0
    }
}