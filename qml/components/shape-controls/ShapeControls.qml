import QtQuick 2.15
import QtQuick.Controls 2.15
import Cubit 1.0
import "../.."
import "../shared"
import "../../CanvasUtils.js" as CanvasUtils

Item {
    id: root
    
    // Properties
    property var selectedElement: null
    property var controller: null  // ShapeControlsController
    property var canvasController: null  // CanvasController for checking mode
    property real controlSize: 30
    
    // During line creation mode, don't block hover events
    property bool isInLineCreationMode: canvasController && canvasController.mode === 7
    property color jointControlColor: Qt.rgba(1, 1, 0, 0)  // Transparent background
    property color jointHoverColor: Qt.rgba(1, 1, 0, 0)  // Transparent on hover
    property color edgeColor: ConfigObject.controlBarLineColor  // Use same blue as joints from ConfigObject
    property color previewColor: Qt.rgba(0, 0, 1, 0.3)  // Semi-transparent blue for preview
    property alias shapeCanvas: shapeCanvas
    
    // Get preview properties from controller
    property point previewPoint: controller ? controller.linePreviewPoint : Qt.point(0, 0)
    property bool showPreview: controller ? controller.showLinePreview : false
    
    onPreviewPointChanged: {
    }
    
    onShowPreviewChanged: {
    }
    
    visible: selectedElement !== null && selectedElement.elementType === "Shape"
    
    // Expose dragging state for parent components
    readonly property bool isDragging: controller ? controller.isDragging : dragging
    
    // Sync controller with selected element and update shape
    onSelectedElementChanged: {
        if (controller) {
            controller.selectedShape = selectedElement
        }
        
        if (selectedElement && visible) {
            // ShapeControls selected element changed, requesting repaint
            shapeCanvas.requestPaint()
            
            // Update bounding box preview
            if (selectedElement.joints && selectedElement.joints.length > 0) {
                updateBoundingBoxPreview(selectedElement.joints)
            }
        }
    }
    
    onControllerChanged: {
        if (controller && selectedElement) {
            controller.selectedShape = selectedElement
        }
    }
    
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
        x: previewX * (root.width - jointPadding * 2) + jointPadding
        y: previewY * (root.height - jointPadding * 2) + jointPadding
        width: previewWidth * (root.width - jointPadding * 2)
        height: previewHeight * (root.height - jointPadding * 2)
    }
    
    // Padding to accommodate joints that extend beyond shape bounds
    property real jointPadding: controlSize / 2
    
    // Draw the shape edges
    Canvas {
        id: shapeCanvas
        // Make canvas larger to accommodate preview lines that extend beyond shape bounds
        anchors.centerIn: parent
        width: Math.max(parent.width, parent.width + 500)  // Extra space for preview
        height: Math.max(parent.height, parent.height + 500)  // Extra space for preview
        clip: false  // Allow drawing outside bounds
        
        onPaint: {
            var ctx = getContext("2d")
            ctx.clearRect(0, 0, width, height)
            
            if (!selectedElement || !selectedElement.joints) return
            
            var joints = selectedElement.joints
            if (joints.length < 2 && !showPreview) return
            
            // Calculate offset since canvas is centered and larger than parent
            var canvasOffsetX = (width - parent.width) / 2
            var canvasOffsetY = (height - parent.height) / 2
            
            // Adjust for padding - joints are normalized to shape bounds, not control bounds
            var shapeWidth = parent.width - (jointPadding * 2)
            var shapeHeight = parent.height - (jointPadding * 2)
            
            ctx.strokeStyle = edgeColor
            ctx.lineWidth = 1
            
            // Draw edges based on shape type
            var isClosedShape = selectedElement.shapeType !== 2 // 2 = Line (from Shape.h enum)
            var edgeCount = isClosedShape ? joints.length : joints.length - 1
            
            for (var i = 0; i < edgeCount; i++) {
                ctx.beginPath()
                
                var currentJoint = joints[i]
                var nextJoint = isClosedShape ? 
                    joints[(i + 1) % joints.length] :  // Closed: wrap around to first joint
                    joints[i + 1]                       // Open: just next joint
                
                ctx.moveTo(currentJoint.x * shapeWidth + jointPadding + canvasOffsetX, 
                          currentJoint.y * shapeHeight + jointPadding + canvasOffsetY)
                ctx.lineTo(nextJoint.x * shapeWidth + jointPadding + canvasOffsetX, 
                          nextJoint.y * shapeHeight + jointPadding + canvasOffsetY)
                
                ctx.stroke()
            }
            
            // Draw preview line from last joint to preview point
            if (showPreview && joints.length > 0 && previewPoint.x !== 0 && previewPoint.y !== 0) {
                var lastJoint = joints[joints.length - 1]
                var lastJointX = lastJoint.x * shapeWidth + jointPadding + canvasOffsetX
                var lastJointY = lastJoint.y * shapeHeight + jointPadding + canvasOffsetY
                
                // Convert preview point from canvas coordinates to local coordinates
                // The ShapeControls is positioned at (selectedElement.x/y - padding) in canvas space
                // and scaled by zoom. We need to convert the canvas point to local ShapeControls coordinates.
                var zoom = root.parent && root.parent.zoomLevel ? root.parent.zoomLevel : 1
                
                // First, get the position relative to the shape's origin in canvas space
                var canvasRelativeX = previewPoint.x - selectedElement.x
                var canvasRelativeY = previewPoint.y - selectedElement.y
                
                // Then scale by zoom and offset by padding and canvas offset
                var localPreviewX = canvasRelativeX * zoom + jointPadding + canvasOffsetX
                var localPreviewY = canvasRelativeY * zoom + jointPadding + canvasOffsetY
                
                
                ctx.strokeStyle = previewColor
                ctx.lineWidth = 1
                ctx.setLineDash([5, 5])  // Dashed line for preview
                ctx.beginPath()
                ctx.moveTo(lastJointX, lastJointY)
                ctx.lineTo(localPreviewX, localPreviewY)
                ctx.stroke()
                ctx.setLineDash([])  // Reset to solid line
            }
        }
        
        // Redraw when joints change
        Connections {
            target: selectedElement
            enabled: selectedElement !== null
            ignoreUnknownSignals: true
            function onJointsChanged() {
                // Canvas: Joints changed, requesting repaint
                shapeCanvas.requestPaint()
            }
        }
        
        // Add connections to controller for preview changes
        Connections {
            target: controller
            enabled: controller !== null
            ignoreUnknownSignals: true
            function onLinePreviewPointChanged() {
                if (showPreview) {
                    shapeCanvas.requestPaint()
                }
            }
            function onShowLinePreviewChanged() {
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
    
    
    // Properties for tracking drag state
    property bool dragging: false
    property int draggedJointIndex: -1
    property point dragStartPos
    property var originalJoints: []
    property var originalBounds: null
    
    // Store controller reference to prevent it from being lost
    property var cachedController: null
    
    Component.onCompleted: {
        // Cache the controller when the component is created
        if (root.parent && root.parent.controller) {
            cachedController = root.parent.controller
        }
    }
    
    // Notify controller when dragging state changes - this needs to happen immediately
    onDraggingChanged: {
        // ShapeControls dragging changed
        var ctrl = (root.parent && root.parent.controller) ? root.parent.controller : cachedController
        if (ctrl) {
            // Keep shape editing mode active while dragging joints
            if (dragging) {
                ctrl.isEditingShape = true
                ctrl.isShapeControlDragging = true
            } else {
                ctrl.isShapeControlDragging = false
            }
        }
    }
    
    // Main mouse area that handles all mouse events
    MouseArea {
        id: mainMouseArea
        anchors.fill: parent
        hoverEnabled: true
        acceptedButtons: Qt.LeftButton
        propagateComposedEvents: true  // Allow events to pass through
        
        // Don't accept hover events during line creation mode
        enabled: !(root.canvasController && root.canvasController.mode === 7)
        
        property int hoveredJointIndex: -1
        
        // Find which joint (if any) is at the given position
        function jointIndexAt(x, y) {
            if (!selectedElement || !selectedElement.joints) return -1
            
            var shapeWidth = root.width - (jointPadding * 2)
            var shapeHeight = root.height - (jointPadding * 2)
            
            for (var i = 0; i < selectedElement.joints.length; i++) {
                var jointX = selectedElement.joints[i].x * shapeWidth + jointPadding
                var jointY = selectedElement.joints[i].y * shapeHeight + jointPadding
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
                mouse.accepted = true
                
                if (controller) {
                    // Select the joint immediately on press
                    controller.setSelectedJointIndex(jointIndex)
                    
                    // Use controller for state management
                    var viewportOverlay = root.parent
                    if (viewportOverlay && viewportOverlay.flickable) {
                        // root.x/y are relative to the viewport, mouse.x/y are relative to root
                        var viewportPoint = Qt.point(root.x + mouse.x, root.y + mouse.y)
                        
                        // Check if zoom is valid
                        if (!viewportOverlay.zoomLevel || viewportOverlay.zoomLevel === 0) {
                            return
                        }
                        
                        // Use the actual canvas bounds, not the defaults
                        var actualMinX = viewportOverlay.canvasMinX !== undefined ? viewportOverlay.canvasMinX : -10000
                        var actualMinY = viewportOverlay.canvasMinY !== undefined ? viewportOverlay.canvasMinY : -10000
                        
                        var canvasPoint = CanvasUtils.viewportToCanvas(
                            viewportPoint,
                            viewportOverlay.flickable.contentX,
                            viewportOverlay.flickable.contentY,
                            viewportOverlay.zoomLevel,
                            actualMinX,
                            actualMinY
                        )
                        controller.startJointDrag(jointIndex, canvasPoint)
                    }
                } else {
                    // Fallback to local state management
                    root.dragging = true
                    root.draggedJointIndex = jointIndex
                    
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
                }
                
                // Force shape editing mode
                var ctrl = (root.parent && root.parent.controller) ? root.parent.controller : cachedController
                if (ctrl) {
                    ctrl.isEditingShape = true
                    ctrl.isShapeControlDragging = true
                }
                // Joint drag started
            } else {
                // Start moving the whole shape
                mouse.accepted = false // Let moveArea handle it
            }
        }
        
        onPositionChanged: (mouse) => {
            if (controller && controller.isDragging) {
                // Use controller to update position
                var viewportOverlay = root.parent
                if (viewportOverlay && viewportOverlay.flickable) {
                    var viewportPoint = Qt.point(root.x + mouse.x, root.y + mouse.y)
                    
                    // Use the actual canvas bounds, not the defaults
                    var actualMinX = viewportOverlay.canvasMinX !== undefined ? viewportOverlay.canvasMinX : -10000
                    var actualMinY = viewportOverlay.canvasMinY !== undefined ? viewportOverlay.canvasMinY : -10000
                    
                    var canvasPoint = CanvasUtils.viewportToCanvas(
                        viewportPoint,
                        viewportOverlay.flickable.contentX,
                        viewportOverlay.flickable.contentY,
                        viewportOverlay.zoomLevel,
                        actualMinX,
                        actualMinY
                    )
                    controller.updateJointPosition(canvasPoint)
                }
            } else if (root.dragging && root.draggedJointIndex >= 0) {
                // Fallback to local update
                root.updateJointPosition(mouse.x, mouse.y)
            } else {
                // Update hover state
                hoveredJointIndex = jointIndexAt(mouse.x, mouse.y)
            }
        }
        
        onReleased: {
            if (controller && controller.isDragging) {
                controller.endJointDrag()
                
                // Keep shape editing mode active after releasing
                var viewportOverlay = root.parent
                var ctrl = (viewportOverlay && viewportOverlay.controller) ? viewportOverlay.controller : cachedController
                if (ctrl) {
                    ctrl.isEditingShape = true
                    ctrl.isShapeControlDragging = false
                }
                
                // Force controls to stay visible
                root.visible = true
                
                // Ensure shape remains selected
                if (viewportOverlay && viewportOverlay.selectionManager && selectedElement) {
                    viewportOverlay.selectionManager.selectElement(selectedElement)
                }
            } else if (root.dragging) {
                root.dragging = false
                root.draggedJointIndex = -1
                root.originalBounds = null
                // Joint drag ended
                
                // Keep shape editing mode active after releasing
                var viewportOverlay = root.parent
                var ctrl = (viewportOverlay && viewportOverlay.controller) ? viewportOverlay.controller : cachedController
                if (ctrl) {
                    ctrl.isEditingShape = true
                    ctrl.isShapeControlDragging = false
                }
                
                // Force controls to stay visible
                root.visible = true
            }
        }
        
        onClicked: (mouse) => {
            var jointIndex = jointIndexAt(mouse.x, mouse.y)
            if (jointIndex >= 0 && controller) {
                // This is a click on a joint, not a drag
                controller.setSelectedJointIndex(jointIndex)
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
        
        // Ensure shape editing mode stays active during updates
        var ctrl = (viewportOverlay && viewportOverlay.controller) ? viewportOverlay.controller : cachedController
        if (ctrl) {
            ctrl.isEditingShape = true
        }
        
        // Mouse position is relative to ShapeControls, convert to absolute viewport position
        // Note: root.x/y already includes the padding offset from DesignControlsOverlay
        var absoluteViewportX = root.x + mouseX
        var absoluteViewportY = root.y + mouseY
        
        // Convert to canvas coordinates using CanvasUtils
        var viewportPoint = Qt.point(absoluteViewportX, absoluteViewportY)
        
        // Use the actual canvas bounds, not the defaults
        var actualMinX = viewportOverlay.canvasMinX !== undefined ? viewportOverlay.canvasMinX : -10000
        var actualMinY = viewportOverlay.canvasMinY !== undefined ? viewportOverlay.canvasMinY : -10000
        
        var canvasPoint = CanvasUtils.viewportToCanvas(
            viewportPoint,
            viewportOverlay.flickable.contentX,
            viewportOverlay.flickable.contentY,
            viewportOverlay.zoomLevel,
            actualMinX,
            actualMinY
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
        
        ControlJoint {
            id: jointVisual
            size: controlSize
            showInnerCircle: true  // Show inner circle for consistency with DesignControls
            baseColor: jointControlColor
            hoverColor: jointHoverColor
            activeColor: jointHoverColor
            
            property int jointIndex: index
            isHovered: mainMouseArea.hoveredJointIndex === jointIndex
            isActive: root.dragging && root.draggedJointIndex === jointIndex
            isSelected: controller && controller.selectedJointIndex === jointIndex
            
            // Position at the joint location (normalized coords to actual position)
            // Account for padding since joints are normalized to shape bounds
            x: {
                if (!selectedElement || !selectedElement.joints || jointIndex >= selectedElement.joints.length) return 0
                var shapeWidth = root.width - (jointPadding * 2)
                return selectedElement.joints[jointIndex].x * shapeWidth + jointPadding - size / 2
            }
            y: {
                if (!selectedElement || !selectedElement.joints || jointIndex >= selectedElement.joints.length) return 0
                var shapeHeight = root.height - (jointPadding * 2)
                return selectedElement.joints[jointIndex].y * shapeHeight + jointPadding - size / 2
            }
        }
    }
    
    
    // Add a mouse area for moving the whole shape (behind the main mouse area)
    MouseArea {
        id: moveArea
        anchors.fill: parent
        z: -1  // Behind mainMouseArea
        cursorShape: Qt.SizeAllCursor
        enabled: {
            // Disable during joint dragging
            if (root.dragging) return false
            
            // Also disable during line creation mode to allow hover tracking
            if (root.canvasController && root.canvasController.mode === 7) { // 7 = ShapeLine mode
                return false
            }
            
            return true
        }
        
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