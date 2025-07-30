import QtQuick
import QtQuick.Controls

Item {
    id: root
    objectName: "VariableAwareSpinBox"
    
    // The property name this SpinBox is editing (e.g., "x", "width", "borderRadius")
    property string propertyName: ""
    
    // Expose SpinBox properties
    property alias value: spinBox.value
    property alias from: spinBox.from
    property alias to: spinBox.to
    property alias stepSize: spinBox.stepSize
    property alias editable: spinBox.editable
    property alias valueFromText: spinBox.valueFromText
    property alias textFromValue: spinBox.textFromValue
    
    // SpinBox valueModified signal
    signal valueModified()
    
    implicitWidth: spinBox.implicitWidth
    implicitHeight: spinBox.implicitHeight
    
    // Get the dragged variable type from the current project/canvas
    property string draggedVariableType: {
        // Try to find the canvas through the window hierarchy
        var win = Window.window
        if (win && win.canvas) {
            return win.canvas.draggedVariableType || ""
        }
        return ""
    }
    
    // Get the element ID that this spinbox is editing
    property string elementId: ""
    
    // Check if this property has a variable binding
    property bool hasBinding: false
    
    // The bound variable (if any)
    property var boundVariable: null
    
    onBoundVariableChanged: {
        // Bound variable changed
    }
    
    // Update hasBinding when needed
    function updateHasBinding() {
        if (!elementId || !propertyName) {
            hasBinding = false
            boundVariable = null
            return
        }
        var win = projectWindow
        if (win && win.canvas && win.canvas.bindingManager) {
            hasBinding = win.canvas.bindingManager.hasBinding(elementId, propertyName)
            // Check if property has binding
            if (hasBinding) {
                // Get the bound variable ID
                var variableId = win.canvas.bindingManager.getBoundVariableId(elementId, propertyName)
                // Found variable ID
                if (variableId && win.canvas.elementModel) {
                    boundVariable = win.canvas.elementModel.getElementById(variableId)
                    // Found bound variable
                } else {
                    boundVariable = null
                }
            } else {
                boundVariable = null
            }
        } else {
            hasBinding = false
            boundVariable = null
        }
    }
    
    function getBindingManager() {
        try {
            var win = projectWindow
            return (win && win.canvas && win.canvas.bindingManager) ? win.canvas.bindingManager : null
        } catch (e) {
            return null
        }
    }
    
    // Update when elementId or propertyName changes
    onElementIdChanged: {
        // Element ID changed
        updateHasBinding()
    }
    onPropertyNameChanged: updateHasBinding()
    
    // Connect to bindingManager signals
    Connections {
        target: root.getBindingManager()
        enabled: target !== null
        
        function onBindingCreated(binding) {
            // Check if this binding is for our element/property
            if (binding && binding.elementId === root.elementId && binding.propertyName === root.propertyName) {
                // This binding is for us - update hasBinding
                root.updateHasBinding()
            }
        }
        
        function onBindingRemoved(elementId, propertyName) {
            // Check if this removal is for our element/property
            // Check if this removal is for our element/property
            if (elementId === root.elementId && propertyName === root.propertyName) {
                // This removal is for us - update hasBinding
                root.updateHasBinding()
            }
        }
    }
    
    // Connect to bound variable value changes
    Connections {
        target: root.boundVariable
        
        function onValueChanged() {
            // Force spinbox to update when variable value changes
            spinBox.valueChanged()
        }
    }
    
    onDraggedVariableTypeChanged: {
        // Variable type changed
    }
    
    // Check if the ghost is over this spinbox
    property bool isGhostOver: false
    
    // Find the project window that contains the dragOverlay
    property var projectWindow: {
        var win = Window.window
        while (win) {
            if (win.dragOverlay !== undefined) {
                return win
            }
            win = win.parent
        }
        return null
    }
    
    // Watch for ghost position changes
    Connections {
        target: projectWindow ? projectWindow.dragOverlay : null
        
        
        function onGhostPositionChanged() {
            if (!target || !target.visible) {
                root.isGhostOver = false
                return
            }
            
            // Get the ghost position in window coordinates
            var ghostPos = target.ghostPosition
            
            // Map the ghost position from the drag overlay to this spinbox's coordinate space
            var localPos = root.mapFromItem(target, ghostPos.x, ghostPos.y)
            
            // Check if ghost center is within this spinbox
            var wasOver = root.isGhostOver
            root.isGhostOver = localPos.x >= 0 && 
                              localPos.x <= root.width &&
                              localPos.y >= 0 && 
                              localPos.y <= root.height
            
            // Update hoveredVariableTarget when hover state changes
            if (root.isGhostOver !== wasOver) {
                // Hover state changed
                
                var win = projectWindow
                if (win && win.canvas) {
                    if (root.isGhostOver && root.acceptsDraggedType) {
                        // Set this spinbox as the hovered target
                        // Set this spinbox as the hovered target
                        win.canvas.hoveredVariableTarget = {
                            "elementId": root.elementId,
                            "propertyName": root.propertyName
                        }
                    } else if (!root.isGhostOver && win.canvas.hoveredVariableTarget.elementId === root.elementId && 
                               win.canvas.hoveredVariableTarget.propertyName === root.propertyName) {
                        // Clear if we were the hovered target
                        // Clear if we were the hovered target
                        win.canvas.hoveredVariableTarget = {}
                    }
                } else {
                    // No win.canvas available
                }
            }
        }
        
        function onVisibleChanged() {
            if (!target || !target.visible) {
                root.isGhostOver = false
            }
        }
    }
    
    // For now, hardcode that number properties accept number variables
    // This avoids the singleton access issues during destruction
    property bool acceptsDraggedType: {
        if (!propertyName || !draggedVariableType) return false
        
        // List of properties that accept number type
        var numberProperties = ["x", "y", "width", "height", "left", "top", "right", "bottom", 
                               "borderRadius", "borderWidth", "gap", "edgeWidth"]
        
        // Check if this is a number property and we're dragging a number variable
        var accepts = numberProperties.indexOf(propertyName) !== -1 && draggedVariableType === "number"
        return accepts
    }
    
    onAcceptsDraggedTypeChanged: {
        // Compatibility changed
    }
    
    // Visual feedback rectangle - blue border when dragging
    Rectangle {
        anchors.fill: parent
        color: "transparent"
        border.color: root.acceptsDraggedType ? "#2196F3" : "transparent"
        border.width: root.acceptsDraggedType ? 3 : 0
        radius: 4
        visible: root.acceptsDraggedType
        z: -1  // Behind the spinbox
    }
    
    // The actual SpinBox
    SpinBox {
        id: spinBox
        anchors.fill: parent
        
        // Disable when bound to a variable
        enabled: !root.hasBinding
        
        // Override the value when bound to show the variable value
        value: {
            if (root.hasBinding && root.boundVariable) {
                var varValue = root.boundVariable.value
                // Convert variable value to number if needed
                if (typeof varValue === 'string') {
                    var numValue = parseFloat(varValue)
                    // Convert string to number
                    return isNaN(numValue) ? 0 : numValue
                }
                return varValue
            } else {
                return root.value
            }
        }
        
        onValueModified: {
            // Only allow modifications when not bound
            if (!root.hasBinding) {
                root.valueModified()
            }
        }
        
        onValueChanged: {
            // Value changed
        }
    }
    
    // Invisible drop area overlay on top
    Rectangle {
        anchors.fill: parent
        color: "transparent"
        visible: root.acceptsDraggedType  // Only visible when dragging compatible variables
        
        // Handle drop events
        DropArea {
            anchors.fill: parent
            enabled: true  // Always enabled to test if drops are working
            
            onEntered: {
                // Drag entered
            }
            
            onExited: {
                // Drag exited
            }
            
            onDropped: {
                // Drop event received
                
                if (projectWindow && projectWindow.dragOverlay && projectWindow.dragOverlay.draggedElement) {
                    var variable = projectWindow.dragOverlay.draggedElement
                    
                    if (variable && variable.elementType === "Variable" && elementId && propertyName) {
                        // Create binding through the binding manager
                        var win = Window.window
                        
                        if (win && win.canvas && win.canvas.bindingManager) {
                            win.canvas.bindingManager.createBinding(variable.elementId, elementId, propertyName)
                        }
                    }
                }
            }
        }
    }
}