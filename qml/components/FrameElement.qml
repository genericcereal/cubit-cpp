import QtQuick
import QtQuick.Controls
import QtQuick.Effects
import Cubit 1.0
import ".."
import "."

Item {
    id: root
    
    // Element component mapper
    property ElementComponentMapper mapper: ElementComponentMapper {}
    
    // The C++ Element object
    property var element
    property var elementModel
    property var canvas: null  // Will be set by parent
    property Frame frameElement: element as Frame
    property alias contentContainer: contentContainer
    property real canvasMinX: 0
    property real canvasMinY: 0
    
    // Size properties bound to C++ element (position is handled by parent Loader)
    width: element ? element.width : 100
    height: element ? element.height : 100
    
    // Selection state
    property bool selected: element ? element.selected : false
    
    // Check if we need rounded corner clipping
    property bool needsClipping: frameElement && frameElement.borderRadius > 0 && frameElement.overflow !== 2
    
    // Check if this is the active outer frame during prototyping
    property bool isActiveOuterFrame: {
        var prototypeController = root.canvas ? root.canvas.prototypeController : null
        return prototypeController && 
               prototypeController.isPrototyping && 
               prototypeController.activeOuterFrame === element.elementId
    }
    
    // Reset scroll tracking when prototyping stops
    onIsActiveOuterFrameChanged: {
        if (!isActiveOuterFrame) {
            hasScrollStarted = false
            framePositionWhenScrollStarted = 0
        }
    }
    
    // Track the frame position when scrolling started
    property real framePositionWhenScrollStarted: 0
    property bool hasScrollStarted: false
    
    // Monitor frame position changes
    Connections {
        target: root.element
        enabled: root.isActiveOuterFrame
        
        function onYChanged() {
            if (!root.isActiveOuterFrame) return
            
            var prototypeController = root.canvas ? root.canvas.prototypeController : null
            var initialPos = prototypeController.getSnapshotElementPosition(root.element.elementId)
            
            // Check if this is the first scroll movement
            if (!root.hasScrollStarted && Math.abs(root.element.y - initialPos.y) > 5) {
                root.hasScrollStarted = true
                // Use the initial position, not the current position after movement
                root.framePositionWhenScrollStarted = initialPos.y
            }
        }
    }
    
    // Frame visual representation (background)
    Rectangle {
        id: frameRect
        anchors.fill: parent
        
        // Frame-specific properties - use the fill property
        color: frameElement ? frameElement.fill : ConfigObject.elementBackgroundColor
        // Only use Rectangle border for solid style
        border.width: (frameElement && frameElement.borderStyle === "Solid") ? frameElement.borderWidth : 0
        border.color: frameElement ? frameElement.borderColor : Qt.rgba(0, 0, 0, 1)
        radius: frameElement ? frameElement.borderRadius : 0
        antialiasing: true
        
        // Apply shadow effect if enabled
        layer.enabled: element && element.boxShadow && element.boxShadow.enabled
        layer.effect: MultiEffect {
            shadowEnabled: true
            shadowColor: element && element.boxShadow ? element.boxShadow.color : "#444444"
            shadowHorizontalOffset: element && element.boxShadow ? element.boxShadow.offsetX : 0
            shadowVerticalOffset: element && element.boxShadow ? element.boxShadow.offsetY : 0
            shadowBlur: element && element.boxShadow ? element.boxShadow.blurRadius / 10.0 : 0 // Scale down blur radius
            shadowScale: element && element.boxShadow ? 1.0 + (element.boxShadow.spreadRadius / 100.0) : 1.0 // Convert spread to scale
        }
    }
    
    // Content that needs to be masked
    Item {
        id: contentWrapper
        anchors.fill: parent
        visible: !needsClipping
        
        // Content container for child elements
        Item {
            id: contentContainer
            anchors.fill: parent
            anchors.margins: frameRect.border.width
            
            // Apply clipping based on overflow mode
            clip: frameElement ? frameElement.overflow !== 2 : false  // clip for Hidden (0) and Scroll (1) modes
            
            // Render child elements
            Repeater {
                model: root.elementModel
                
                delegate: Loader {
                    property var childElement: model.element
                    property string childElementType: model.elementType
                    property string childParentId: childElement ? childElement.parentId : ""
                    
                    // Only render elements that are children of this frame
                    active: childParentId === root.element.elementId
                    
                    // Position child elements relative to parent
                    x: childElement && active ? childElement.x - root.element.x : 0
                    y: childElement && active ? childElement.y - root.element.y : 0
                    
                    Component.onCompleted: {
                        if (active && root.element.elementType === "FrameComponentInstance") {
                                       "parent elementId:", root.element.elementId,
                                       "child type:", childElementType,
                                       "child showInList:", childElement.showInElementList)
                        }
                    }
                    
                    source: {
                        if (!active || !childElement) return ""
                        return root.mapper.getComponentPath(childElement)
                    }
                    
                    onLoaded: {
                        if (item && childElement) {
                            item.element = childElement
                            item.elementModel = root.elementModel
                            if (item.hasOwnProperty("canvasMinX")) {
                                item.canvasMinX = root.canvasMinX
                                item.canvasMinY = root.canvasMinY
                            }
                        }
                    }
                }
            }
        }
    }
    
    // Mask shape for rounded corners
    Rectangle {
        id: mask
        anchors.fill: parent
        radius: frameElement ? frameElement.borderRadius : 0
        visible: false
        antialiasing: true
        smooth: true
        color: "white"
    }
    
    // Shader effect sources
    ShaderEffectSource {
        id: contentSource
        sourceItem: contentWrapper
        hideSource: needsClipping
        visible: false
        live: true
        recursive: true
    }
    
    ShaderEffectSource {
        id: maskSource
        sourceItem: mask
        hideSource: true
        visible: false
    }
    
    // Apply the mask using shader effect
    ShaderEffect {
        anchors.fill: parent
        visible: needsClipping
        
        property variant source: contentSource
        property variant maskSource: maskSource
        
        fragmentShader: "qrc:/shaders/roundedmask.frag.qsb"
    }
    
    // Opacity mask for parts outside the viewable area during prototyping
    Item {
        id: prototypeMask
        anchors.fill: parent
        visible: root.isActiveOuterFrame
        
        // Simple approach: The viewable area is always centered horizontally and starts at Y=0 of the frame
        property real viewableX: {
            if (!root.isActiveOuterFrame) return 0
            var prototypeController = root.canvas ? root.canvas.prototypeController : null
            var viewableWidth = prototypeController.viewableArea.width
            // Center the viewable area horizontally within the frame
            return (root.width - viewableWidth) / 2
        }
        
        property real viewableY: {
            if (!root.isActiveOuterFrame) return 0
            
            if (!root.hasScrollStarted) {
                // Before scrolling starts, viewable area is at the top of the frame
                return 0
            } else {
                // After scrolling starts, calculate offset based on movement
                var currentY = root.element.y
                var scrollOffset = root.framePositionWhenScrollStarted - currentY
                return Math.max(0, scrollOffset)
            }
        }
        
        property real viewableWidth: {
            if (!root.isActiveOuterFrame) return 0
            var prototypeController = root.canvas ? root.canvas.prototypeController : null
            return Math.min(prototypeController.viewableArea.width, root.width)
        }
        
        property real viewableHeight: {
            if (!root.isActiveOuterFrame) return 0
            var prototypeController = root.canvas ? root.canvas.prototypeController : null
            var availableHeight = root.height - viewableY
            return Math.min(prototypeController.viewableArea.height, availableHeight)
        }
        
        // Top overlay (above viewable area)
        Rectangle {
            x: 0
            y: 0
            width: parent.width
            height: prototypeMask.viewableY
            color: "white"
            opacity: 0.6
            visible: height > 0
        }
        
        // Left overlay
        Rectangle {
            x: 0
            y: prototypeMask.viewableY
            width: prototypeMask.viewableX
            height: prototypeMask.viewableHeight
            color: "white"
            opacity: 0.6
            visible: width > 0
        }
        
        // Right overlay
        Rectangle {
            x: prototypeMask.viewableX + prototypeMask.viewableWidth
            y: prototypeMask.viewableY
            width: parent.width - x
            height: prototypeMask.viewableHeight
            color: "white"
            opacity: 0.6
            visible: width > 0
        }
        
        // Bottom overlay
        Rectangle {
            x: 0
            y: prototypeMask.viewableY + prototypeMask.viewableHeight
            width: parent.width
            height: parent.height - y
            color: "white"
            opacity: 0.6
            visible: height > 0
        }
    }
    
    // Custom border for dashed and dotted styles
    StyledBorder {
        id: styledBorder
        anchors.fill: parent
        visible: frameElement && frameElement.borderWidth > 0 && frameElement.borderStyle !== "Solid"
        borderWidth: frameElement ? frameElement.borderWidth : 0
        borderColor: frameElement ? frameElement.borderColor : Qt.rgba(0, 0, 0, 1)
        borderStyle: frameElement ? frameElement.borderStyle : "Solid"
        borderRadius: frameElement ? frameElement.borderRadius : 0
        z: 1 // Ensure border is above the background
    }
}