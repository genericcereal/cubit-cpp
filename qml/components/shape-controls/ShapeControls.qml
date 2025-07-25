import QtQuick 2.15
import QtQuick.Controls 2.15
import Cubit 1.0
import "../.."

Item {
    id: root
    
    // Properties
    property var selectedElement: null
    property real controlSize: 30
    property color jointControlColor: Qt.rgba(1, 1, 0, 0.8)  // Yellow for joints
    property color jointHoverColor: Qt.rgba(1, 1, 0, 1.0)  // Brighter yellow on hover
    property color edgeColor: Qt.rgba(0, 0, 1, 1.0)  // Fully opaque blue for edges
    property color previewColor: Qt.rgba(0, 0, 1, 0.3)  // Semi-transparent blue for preview
    property alias shapeCanvas: shapeCanvas
    
    visible: selectedElement !== null && selectedElement.elementType === "Shape"
    
    // Expose dragging state for parent components
    readonly property bool isDragging: dragging
    
    // Bounding box preview properties
    property real previewX: 0
    property real previewY: 0
    property real previewWidth: 0
    property real previewHeight: 0
    
    // Don't bind position/size here - they will be set by parent
    
    // Bounding box preview (shown during drag)
    Rectangle {
        id: boundingBoxPreview
        visible: root.dragging
        color: "transparent"
        border.color: previewColor
        border.width: 2
        x: previewX * root.width
        y: previewY * root.height
        width: previewWidth * root.width
        height: previewHeight * root.height
    }
    
    // Draw the shape edges
    Canvas {
        id: shapeCanvas
        anchors.fill: parent
        
        onPaint: {
            var ctx = getContext("2d")
            ctx.clearRect(0, 0, width, height)
            
            if (!selectedElement || !selectedElement.joints) return
            
            var joints = selectedElement.joints
            if (joints.length < 2) return
            
            ctx.strokeStyle = edgeColor
            ctx.lineWidth = 2
            
            // Draw edges based on shape type
            var isClosedShape = selectedElement.shapeType !== 2 // 2 = Line (from Shape.h enum)
            var edgeCount = isClosedShape ? joints.length : joints.length - 1
            
            for (var i = 0; i < edgeCount; i++) {
                ctx.beginPath()
                
                var currentJoint = joints[i]
                var nextJoint = isClosedShape ? 
                    joints[(i + 1) % joints.length] :  // Closed: wrap around to first joint
                    joints[i + 1]                       // Open: just next joint
                
                ctx.moveTo(currentJoint.x * width, currentJoint.y * height)
                ctx.lineTo(nextJoint.x * width, nextJoint.y * height)
                
                ctx.stroke()
            }
        }
        
        // Redraw when joints change
        Connections {
            target: selectedElement
            function onJointsChanged() {
                // Canvas: Joints changed, requesting repaint
                shapeCanvas.requestPaint()
            }
        }
    }
    
    // Update shape when controls become visible (e.g. when entering shape edit mode)
    onVisibleChanged: {
        if (visible && selectedElement) {
            // ShapeControls became visible, requesting repaint
            shapeCanvas.requestPaint()
            
            // Also update bounding box preview if we have joints
            if (selectedElement.joints && selectedElement.joints.length > 0) {
                updateBoundingBoxPreview(selectedElement.joints)
            }
        }
    }
    
    // Update shape when selected element changes
    onSelectedElementChanged: {
        if (selectedElement && visible) {
            // ShapeControls selected element changed, requesting repaint
            shapeCanvas.requestPaint()
            
            // Update bounding box preview
            if (selectedElement.joints && selectedElement.joints.length > 0) {
                updateBoundingBoxPreview(selectedElement.joints)
            }
        }
    }
    
    // Properties for tracking drag state
    property bool dragging: false
    property int draggedJointIndex: -1
    property point dragStartPos
    property var originalJoints: []
    property var originalBounds: null
    
    // Notify controller when dragging state changes - this needs to happen immediately
    onDraggingChanged: {
        // ShapeControls dragging changed
        if (root.parent && root.parent.controller) {
            // Keep shape editing mode active while dragging joints
            if (dragging) {
                root.parent.controller.isEditingShape = true
            }
        }
    }
    
    // Main mouse area that handles all mouse events
    MouseArea {
        id: mainMouseArea
        anchors.fill: parent
        hoverEnabled: true
        acceptedButtons: Qt.LeftButton
        
        property int hoveredJointIndex: -1
        
        // Find which joint (if any) is at the given position
        function jointIndexAt(x, y) {
            if (!selectedElement || !selectedElement.joints) return -1
            
            for (var i = 0; i < selectedElement.joints.length; i++) {
                var jointX = selectedElement.joints[i].x * root.width
                var jointY = selectedElement.joints[i].y * root.height
                var dx = x - jointX
                var dy = y - jointY
                var distance = Math.sqrt(dx * dx + dy * dy)
                if (distance <= controlSize / 2) {
                    return i
                }
            }
            return -1
        }
        
        onPressed: (mouse) => {
            var jointIndex = jointIndexAt(mouse.x, mouse.y)
            if (jointIndex >= 0) {
                // Set dragging state immediately to prevent DesignControls from reappearing
                root.dragging = true
                root.draggedJointIndex = jointIndex
                
                // Force shape editing mode
                if (root.parent && root.parent.controller) {
                    root.parent.controller.isEditingShape = true
                }
                
                if (selectedElement && selectedElement.joints) {
                    root.originalJoints = JSON.parse(JSON.stringify(selectedElement.joints))
                    root.originalBounds = {
                        x: selectedElement.x,
                        y: selectedElement.y,
                        width: selectedElement.width,
                        height: selectedElement.height
                    }
                    // Initialize bounding box preview
                    root.updateBoundingBoxPreview(selectedElement.joints)
                }
                // Joint drag started
                mouse.accepted = true
            } else {
                // Start moving the whole shape
                mouse.accepted = false // Let moveArea handle it
            }
        }
        
        onPositionChanged: (mouse) => {
            if (root.dragging && root.draggedJointIndex >= 0) {
                root.updateJointPosition(mouse.x, mouse.y)
            } else {
                // Update hover state
                hoveredJointIndex = jointIndexAt(mouse.x, mouse.y)
            }
        }
        
        onReleased: {
            if (root.dragging) {
                root.dragging = false
                root.draggedJointIndex = -1
                root.originalBounds = null
                // Joint drag ended
            }
        }
        
        onExited: {
            hoveredJointIndex = -1
        }
        
        cursorShape: {
            if (root.dragging || hoveredJointIndex >= 0) {
                return Qt.CrossCursor
            }
            return Qt.ArrowCursor
        }
    }
    
    // Function to handle joint updates during drag
    function updateJointPosition(mouseX, mouseY) {
        if (!root.dragging || root.draggedJointIndex < 0 || !root.originalBounds) {
            // updateJointPosition called but not dragging
            return
        }
        
        // Get viewport overlay from parent hierarchy
        var viewportOverlay = root.parent
        if (!viewportOverlay || !viewportOverlay.flickable) return
        
        // Mouse position is relative to ShapeControls, convert to absolute viewport position
        var absoluteViewportX = root.x + mouseX
        var absoluteViewportY = root.y + mouseY
        
        // Convert to canvas coordinates using CanvasUtils
        var canvasPoint = CanvasUtils.viewportToCanvas(
            absoluteViewportX, 
            absoluteViewportY,
            viewportOverlay.flickable.contentX,
            viewportOverlay.flickable.contentY,
            viewportOverlay.zoomLevel
        )
        
        // Update joints with absolute canvas positions
        var absoluteJoints = []
        for (var i = 0; i < root.originalJoints.length; i++) {
            if (i === root.draggedJointIndex) {
                absoluteJoints.push(Qt.point(canvasPoint.x, canvasPoint.y))
            } else {
                // Convert normalized joint to absolute canvas position using original bounds
                var jointX = root.originalBounds.x + root.originalJoints[i].x * root.originalBounds.width
                var jointY = root.originalBounds.y + root.originalJoints[i].y * root.originalBounds.height
                absoluteJoints.push(Qt.point(jointX, jointY))
            }
        }
        
        // Calculate new bounding box from absolute joint positions
        var minX = absoluteJoints[0].x, minY = absoluteJoints[0].y
        var maxX = absoluteJoints[0].x, maxY = absoluteJoints[0].y
        
        for (var j = 1; j < absoluteJoints.length; j++) {
            minX = Math.min(minX, absoluteJoints[j].x)
            minY = Math.min(minY, absoluteJoints[j].y)
            maxX = Math.max(maxX, absoluteJoints[j].x)
            maxY = Math.max(maxY, absoluteJoints[j].y)
        }
        
        // Calculate new shape bounds
        var newX = minX
        var newY = minY
        var newWidth = Math.max(1, maxX - minX)  // Ensure minimum size of 1
        var newHeight = Math.max(1, maxY - minY)
        
        // Convert absolute joints back to normalized coordinates relative to new bounds
        var normalizedJoints = []
        for (var k = 0; k < absoluteJoints.length; k++) {
            var normX = (absoluteJoints[k].x - newX) / newWidth
            var normY = (absoluteJoints[k].y - newY) / newHeight
            normalizedJoints.push(Qt.point(normX, normY))
        }
        
        // Update shape bounds and joints
        selectedElement.x = newX
        selectedElement.y = newY
        selectedElement.width = newWidth
        selectedElement.height = newHeight
        selectedElement.setJoints(normalizedJoints)
        
        // Update bounding box preview (in normalized coordinates)
        updateBoundingBoxPreview(normalizedJoints)
        
        // Request canvas repaint to update edges
        shapeCanvas.requestPaint()
    }
    
    // Function to calculate and update bounding box preview
    function updateBoundingBoxPreview(joints) {
        if (joints.length === 0) return
        
        // Find min/max coordinates
        var minX = 1.0, minY = 1.0
        var maxX = 0.0, maxY = 0.0
        
        for (var i = 0; i < joints.length; i++) {
            minX = Math.min(minX, joints[i].x)
            minY = Math.min(minY, joints[i].y)
            maxX = Math.max(maxX, joints[i].x)
            maxY = Math.max(maxY, joints[i].y)
        }
        
        // Update preview properties
        previewX = minX
        previewY = minY
        previewWidth = maxX - minX
        previewHeight = maxY - minY
    }
    
    // Joint controls - positioned at each joint (visual only)
    Repeater {
        id: jointRepeater
        model: selectedElement ? selectedElement.joints : []
        
        Rectangle {
            id: jointVisual
            width: controlSize
            height: controlSize
            radius: controlSize / 2
            
            property int jointIndex: index
            property bool isHovered: mainMouseArea.hoveredJointIndex === jointIndex
            property bool isDragging: root.dragging && root.draggedJointIndex === jointIndex
            
            // Position at the joint location (normalized coords to actual position)
            x: {
                if (!selectedElement || !selectedElement.joints || jointIndex >= selectedElement.joints.length) return 0
                return selectedElement.joints[jointIndex].x * root.width - controlSize / 2
            }
            y: {
                if (!selectedElement || !selectedElement.joints || jointIndex >= selectedElement.joints.length) return 0
                return selectedElement.joints[jointIndex].y * root.height - controlSize / 2
            }
            
            color: isHovered || isDragging ? jointHoverColor : jointControlColor
            border.color: Qt.darker(jointControlColor)
            border.width: 1
        }
    }
    
    
    // Add a mouse area for moving the whole shape (behind the main mouse area)
    MouseArea {
        id: moveArea
        anchors.fill: parent
        z: -1  // Behind mainMouseArea
        cursorShape: Qt.SizeAllCursor
        enabled: !root.dragging  // Disable during joint dragging
        
        property point pressPos
        
        onPressed: (mouse) => {
            mouse.accepted = true
            // Check if we clicked near a line (for line shapes)
            // For now, just handle general movement
            pressPos = Qt.point(mouse.x, mouse.y)
        }
        
        onPositionChanged: (mouse) => {
            if (pressed && selectedElement) {
                // Get viewport overlay from parent hierarchy
                var viewportOverlay = root.parent
                if (!viewportOverlay) return
                
                var delta = Qt.point(mouse.x - pressPos.x, mouse.y - pressPos.y)
                
                // Convert delta to canvas space
                var canvasDelta = Qt.point(
                    delta.x / viewportOverlay.zoomLevel,
                    delta.y / viewportOverlay.zoomLevel
                )
                
                selectedElement.x += canvasDelta.x
                selectedElement.y += canvasDelta.y
            }
        }
    }
}