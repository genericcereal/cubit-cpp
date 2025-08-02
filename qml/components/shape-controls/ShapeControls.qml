import QtQuick 2.15
import QtQuick.Controls 2.15
import Cubit 1.0
import "../.."
import "../shared"
import "../design-controls"
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
            // Only assign if selectedElement is actually a Shape
            if (selectedElement && selectedElement.elementType === "Shape") {
                controller.selectedShape = selectedElement
            } else {
                controller.selectedShape = null
            }
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
    
    // Padding to accommodate joints that extend beyond shape bounds
    property real jointPadding: controlSize / 2
    
    // Draw the shape edges
    Canvas {
        id: shapeCanvas
        // Make canvas larger to accommodate preview lines and edge width
        anchors.centerIn: parent
        width: Math.max(parent.width + (selectedElement ? selectedElement.edgeWidth * 2 : 0), parent.width + 500)  // Extra space for preview and edge width
        height: Math.max(parent.height + (selectedElement ? selectedElement.edgeWidth * 2 : 0), parent.height + 500)  // Extra space for preview and edge width
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
            
            // Account for edge width in calculations
            var edgeWidth = selectedElement.edgeWidth || 1
            var edgePadding = edgeWidth
            
            // Adjust for padding - joints are normalized to shape bounds, not control bounds
            var shapeWidth = parent.width - (jointPadding * 2)
            var shapeHeight = parent.height - (jointPadding * 2)
            
            ctx.strokeStyle = edgeColor
            ctx.lineWidth = selectedElement.edgeWidth || 1
            ctx.lineJoin = selectedElement.lineJoin || "miter"
            ctx.lineCap = selectedElement.lineCap || "round"
            
            // Draw edges based on shape type
            var isClosedShape = selectedElement.shapeType !== 2 // 2 = Pen (from Shape.h enum)
            
            if (isClosedShape) {
                // For closed shapes, draw all edges including closing edge
                for (var i = 0; i < joints.length; i++) {
                    var currentJoint = joints[i]
                    var nextJoint = joints[(i + 1) % joints.length]
                    
                    ctx.beginPath()
                    ctx.moveTo(currentJoint.x * shapeWidth + jointPadding + canvasOffsetX, 
                              currentJoint.y * shapeHeight + jointPadding + canvasOffsetY)
                    ctx.lineTo(nextJoint.x * shapeWidth + jointPadding + canvasOffsetX, 
                              nextJoint.y * shapeHeight + jointPadding + canvasOffsetY)
                    ctx.stroke()
                }
            } else {
                // For pen shapes, draw edges based on the edges list if available, otherwise consecutive joints
                var edges = selectedElement.edges || []
                
                if (edges.length > 0) {
                    // First, detect and fill closed loops
                    var closedLoops = findClosedLoops(edges, joints.length)
                    for (var loopIndex = 0; loopIndex < closedLoops.length; loopIndex++) {
                        var loop = closedLoops[loopIndex]
                        if (loop.length >= 3) { // Need at least 3 joints for a fillable area
                            ctx.beginPath()
                            ctx.moveTo(joints[loop[0]].x * shapeWidth + jointPadding + canvasOffsetX, 
                                      joints[loop[0]].y * shapeHeight + jointPadding + canvasOffsetY)
                            for (var k = 1; k < loop.length; k++) {
                                ctx.lineTo(joints[loop[k]].x * shapeWidth + jointPadding + canvasOffsetX, 
                                          joints[loop[k]].y * shapeHeight + jointPadding + canvasOffsetY)
                            }
                            ctx.closePath()
                            
                            // Fill the closed loop
                            if (selectedElement.fillColor && selectedElement.fillColor.a > 0) {
                                ctx.fillStyle = selectedElement.fillColor
                                ctx.fill()
                            }
                        }
                    }
                    
                    // Then draw edges from the edges list
                    for (var i = 0; i < edges.length; i++) {
                        var edge = edges[i]
                        var fromIndex = edge.from
                        var toIndex = edge.to
                        
                        if (fromIndex >= 0 && fromIndex < joints.length && 
                            toIndex >= 0 && toIndex < joints.length) {
                            ctx.beginPath()
                            ctx.moveTo(joints[fromIndex].x * shapeWidth + jointPadding + canvasOffsetX, 
                                      joints[fromIndex].y * shapeHeight + jointPadding + canvasOffsetY)
                            ctx.lineTo(joints[toIndex].x * shapeWidth + jointPadding + canvasOffsetX, 
                                      joints[toIndex].y * shapeHeight + jointPadding + canvasOffsetY)
                            ctx.stroke()
                        }
                    }
                } else {
                    // Fallback: draw edges between consecutive joints
                    for (var i = 1; i < joints.length; i++) {
                        ctx.beginPath()
                        ctx.moveTo(joints[i-1].x * shapeWidth + jointPadding + canvasOffsetX, 
                                  joints[i-1].y * shapeHeight + jointPadding + canvasOffsetY)
                        ctx.lineTo(joints[i].x * shapeWidth + jointPadding + canvasOffsetX, 
                                  joints[i].y * shapeHeight + jointPadding + canvasOffsetY)
                        ctx.stroke()
                    }
                }
            }
            
            // Draw preview line from selected joint (or last joint if no selection) to preview point
            if (showPreview && joints.length > 0 && previewPoint.x !== 0 && previewPoint.y !== 0) {
                var selectedJointIndex = controller ? controller.selectedJointIndex : -1
                var activeJoint = (selectedJointIndex >= 0 && selectedJointIndex < joints.length) 
                    ? joints[selectedJointIndex] 
                    : joints[joints.length - 1]
                var activeJointX = activeJoint.x * shapeWidth + jointPadding + canvasOffsetX
                var activeJointY = activeJoint.y * shapeHeight + jointPadding + canvasOffsetY
                
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
                ctx.moveTo(activeJointX, activeJointY)
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
            function onSelectedJointIndexChanged() {
                if (showPreview) {
                    shapeCanvas.requestPaint()
                }
            }
            function onIsEditingShapeChanged() {
                // Clear selected joint when exiting shape editing mode
                if (controller && !controller.isEditingShape && controller.selectedJointIndex >= 0) {
                    controller.setSelectedJointIndex(-1)
                }
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
    
    // Visual position for dragged joint (updated immediately for smooth tracking)
    property point draggedJointVisualPos: Qt.point(0, 0)
    
    // Store controller reference to prevent it from being lost
    property var cachedController: null
    
    // Throttled update for joint position changes
    ThrottledUpdate {
        id: jointDragThrottle
        active: root.dragging && root.draggedJointIndex >= 0
        
        onUpdate: (data) => {
            if (data) {
                if (controller && controller.isDragging && data.canvasPoint) {
                    controller.updateJointPosition(data.canvasPoint)
                } else if (root.dragging && root.draggedJointIndex >= 0 && data.mouseX !== undefined && data.mouseY !== undefined) {
                    // Fallback to local update
                    root.updateJointPosition(data.mouseX, data.mouseY)
                }
            }
        }
        
        onActiveChanged: {
        }
    }
    
    Component.onCompleted: {
        // Cache the controller when the component is created
        if (root.parent && root.parent.controller) {
            cachedController = root.parent.controller
        }
    }
    
    // Clean up connections on destruction
    Component.onDestruction: {
        // Disconnect from controller to prevent crashes during shutdown
        if (controller) {
            controller.selectedShape = null
        }
    }
    
    // Notify controller when dragging state changes - this needs to happen immediately
    onDraggingChanged: {
        // ShapeControls dragging changed
        if (root.controller) {
            // Update shape control dragging state
            if (dragging) {
                root.controller.isShapeControlDragging = true
            } else {
                root.controller.isShapeControlDragging = false
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
        
        // Enable hover tracking in all modes
        enabled: true
        
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
            // In pen mode, don't handle press events
            if (root.canvasController && root.canvasController.mode === 7) {
                mouse.accepted = false
                return
            }
            
            var jointIndex = jointIndexAt(mouse.x, mouse.y)
            if (jointIndex >= 0) {
                mouse.accepted = true
                
                // Store the pressed joint but don't set dragging yet
                root.draggedJointIndex = jointIndex
                
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
                
                // Set shape control dragging state and ensure editing mode is true
                if (root.controller) {
                    root.controller.isShapeControlDragging = true
                    // Ensure editing mode is true during drag
                    if (!root.controller.isEditingShape) {
                        root.controller.isEditingShape = true
                    }
                }
                // Joint drag started
            } else {
                // Start moving the whole shape
                mouse.accepted = false // Let moveArea handle it
            }
        }
        
        onPositionChanged: (mouse) => {
            // Check if we need to start dragging
            if (!root.dragging && root.draggedJointIndex >= 0 && pressed) {
                // Start dragging
                root.dragging = true
                root.draggedJointVisualPos = Qt.point(mouse.x, mouse.y)
            }
            
            // In pen mode, track hover and update preview
            if (root.canvasController && root.canvasController.mode === 7) {
                var jointIndex = jointIndexAt(mouse.x, mouse.y)
                if (controller) {
                    controller.setHoveredJointIndex(jointIndex)
                    
                    // Convert mouse position to canvas coordinates and update preview
                    var viewportOverlay = root.parent
                    if (viewportOverlay && viewportOverlay.flickable) {
                        var viewportPoint = Qt.point(root.x + mouse.x, root.y + mouse.y)
                        
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
                        
                        // Update the preview point
                        controller.linePreviewPoint = canvasPoint
                        controller.showLinePreview = true
                    }
                }
                return
            }
            
            if (controller && controller.isDragging) {
                // Update visual position immediately for smooth tracking
                root.draggedJointVisualPos = Qt.point(mouse.x, mouse.y)
                
                // Request throttled update for actual position
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
                    
                    // Request throttled update
                    jointDragThrottle.requestUpdate({
                        canvasPoint: canvasPoint,
                        mouseX: mouse.x,
                        mouseY: mouse.y
                    })
                }
            } else if (root.dragging && root.draggedJointIndex >= 0) {
                // Update visual position immediately for smooth tracking
                root.draggedJointVisualPos = Qt.point(mouse.x, mouse.y)
                
                // Request throttled update for fallback local dragging
                jointDragThrottle.requestUpdate({
                    mouseX: mouse.x,
                    mouseY: mouse.y
                })
            } else {
                // Update hover state
                hoveredJointIndex = jointIndexAt(mouse.x, mouse.y)
                if (controller) {
                    controller.setHoveredJointIndex(hoveredJointIndex)
                }
            }
        }
        
        onReleased: {
            // Force final update to ensure precise final position
            jointDragThrottle.forceUpdate()
            
            if (controller && controller.isDragging) {
                controller.endJointDrag()
                
                // Clear shape control dragging state
                if (root.controller) {
                    root.controller.isShapeControlDragging = false
                }
                
                // Ensure shape remains selected
                if (viewportOverlay && viewportOverlay.selectionManager && selectedElement) {
                    viewportOverlay.selectionManager.selectElement(selectedElement)
                }
            }
            
            // Always clear dragging state and reset visual position
            if (root.dragging) {
                root.dragging = false
                root.draggedJointIndex = -1
                root.originalBounds = null
                root.draggedJointVisualPos = Qt.point(0, 0) // Reset visual position
                // Joint drag ended
                
                // Clear shape control dragging state
                if (root.controller) {
                    root.controller.isShapeControlDragging = false
                }
            }
        }
        
        onClicked: (mouse) => {
            var jointIndex = jointIndexAt(mouse.x, mouse.y)
            if (jointIndex >= 0 && controller) {
                // This is a click on a joint, not a drag
                controller.setSelectedJointIndex(jointIndex)
                // Make sure dragging state is cleared for clicks
                if (root.controller) {
                    root.controller.isShapeControlDragging = false
                }
            }
        }
        
        onExited: {
            hoveredJointIndex = -1
            if (controller) {
                controller.setHoveredJointIndex(-1)
            }
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
        
        // Get controller reference
        var ctrl = (viewportOverlay && viewportOverlay.controller) ? viewportOverlay.controller : cachedController
        
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
            innerCircleColor: isHovered ? Qt.rgba(0.5, 0.5, 0.5, 1) : ConfigObject.controlJointCircleFill  // Gray when hovered, white otherwise
            
            property int jointIndex: index
            isHovered: controller ? controller.hoveredJointIndex === jointIndex : mainMouseArea.hoveredJointIndex === jointIndex
            isActive: root.dragging && root.draggedJointIndex === jointIndex
            isSelected: controller && controller.selectedJointIndex === jointIndex
            
            // Position at the joint location (normalized coords to actual position)
            // Account for padding since joints are normalized to shape bounds
            x: {
                if (!selectedElement || !selectedElement.joints || jointIndex >= selectedElement.joints.length) return 0
                
                // Use visual position when this joint is being dragged
                if (root.dragging && root.draggedJointIndex === jointIndex) {
                    return root.draggedJointVisualPos.x - size / 2
                }
                
                var shapeWidth = root.width - (jointPadding * 2)
                return selectedElement.joints[jointIndex].x * shapeWidth + jointPadding - size / 2
            }
            y: {
                if (!selectedElement || !selectedElement.joints || jointIndex >= selectedElement.joints.length) return 0
                
                // Use visual position when this joint is being dragged
                if (root.dragging && root.draggedJointIndex === jointIndex) {
                    return root.draggedJointVisualPos.y - size / 2
                }
                
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
            
            // Also disable during pen creation mode to allow hover tracking
            if (root.canvasController && root.canvasController.mode === 7) { // 7 = ShapePen mode
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
            
            // Ensure editing mode is true when starting to drag the shape
            if (root.controller && !root.controller.isEditingShape) {
                root.controller.isEditingShape = true
            }
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
        
        onClicked: (mouse) => {
            // Clear selected joint when clicking on the shape body (not on a joint)
            if (controller) {
                controller.setSelectedJointIndex(-1)
            }
        }
    }
    
    // Function to find closed loops in the edge graph
    function findClosedLoops(edges, jointCount) {
        // Build adjacency list
        var adjacencyList = []
        for (var i = 0; i < jointCount; i++) {
            adjacencyList[i] = []
        }
        
        for (var e = 0; e < edges.length; e++) {
            var edge = edges[e]
            adjacencyList[edge.from].push(edge.to)
            adjacencyList[edge.to].push(edge.from) // Undirected graph
        }
        
        var visited = []
        var loops = []
        
        // DFS to find cycles
        function findCycles(node, path, start) {
            if (visited[node] && node === start && path.length >= 3) {
                // Found a cycle, add it to loops
                var cycle = path.slice()
                cycle.push(node) // Close the cycle
                loops.push(cycle)
                return
            }
            
            if (visited[node]) {
                return // Already processed
            }
            
            visited[node] = true
            path.push(node)
            
            var neighbors = adjacencyList[node]
            for (var n = 0; n < neighbors.length; n++) {
                var neighbor = neighbors[n]
                if (path.length === 1 || neighbor !== path[path.length - 2]) { // Avoid immediate backtrack
                    findCycles(neighbor, path, start)
                }
            }
            
            path.pop()
            visited[node] = false
        }
        
        // Try starting from each joint
        for (var startNode = 0; startNode < jointCount; startNode++) {
            visited = []
            for (var v = 0; v < jointCount; v++) {
                visited[v] = false
            }
            findCycles(startNode, [], startNode)
        }
        
        // Remove duplicate cycles and return unique ones
        var uniqueLoops = []
        for (var l = 0; l < loops.length; l++) {
            var loop = loops[l]
            var isUnique = true
            
            for (var ul = 0; ul < uniqueLoops.length; ul++) {
                if (areLoopsEqual(loop, uniqueLoops[ul])) {
                    isUnique = false
                    break
                }
            }
            
            if (isUnique) {
                uniqueLoops.push(loop.slice(0, -1)) // Remove the duplicate closing node
            }
        }
        
        return uniqueLoops
    }
    
    // Helper function to check if two loops are the same (considering rotation and direction)
    function areLoopsEqual(loop1, loop2) {
        if (loop1.length !== loop2.length) return false
        
        var len = loop1.length - 1 // Exclude duplicate closing node
        for (var offset = 0; offset < len; offset++) {
            var match = true
            for (var i = 0; i < len; i++) {
                if (loop1[i] !== loop2[(i + offset) % len]) {
                    match = false
                    break
                }
            }
            if (match) return true
            
            // Check reverse direction
            match = true
            for (var j = 0; j < len; j++) {
                if (loop1[j] !== loop2[(len - 1 - j + offset) % len]) {
                    match = false
                    break
                }
            }
            if (match) return true
        }
        
        return false
    }
}