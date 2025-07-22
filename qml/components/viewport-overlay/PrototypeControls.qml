import QtQuick
import Cubit 1.0

// PrototypeControls contains the prototype-related overlay elements
Item {
    id: root
    
    // Properties passed from ViewportOverlay
    property var canvasView
    property var controller
    property var flickable
    
    // Access the prototype controller
    property var prototypeController: Application.activeCanvas ? Application.activeCanvas.prototypeController : null
    
    onPrototypeControllerChanged: {
        // Console log removed - prototypeController changed
        
        // Connect requestCanvasMove signal to the canvas moveToPoint function
        if (prototypeController) {
            prototypeController.requestCanvasMove.connect(function(canvasPoint, animated) {
                if (canvasView && canvasView.moveToPoint) {
                    // Adjust the Y coordinate to account for the viewable area's position
                    // The prototype viewable area is positioned at a specific Y in the viewport
                    // We need to offset the canvas point so the frame aligns with the viewable area
                    var adjustedPoint = Qt.point(canvasPoint.x, canvasPoint.y)
                    
                    // Get the viewable area's actual position in viewport
                    if (prototypeViewableArea && prototypeViewableArea.visible) {
                        // The viewable area is now vertically centered in the viewport
                        var viewportHeight = root.flickable.height
                        var viewableAreaHeight = prototypeController.viewableArea.height
                        var viewableAreaTopInViewport = (viewportHeight - viewableAreaHeight) / 2
                        
                        // We need to find what canvas point to pass to moveToPoint
                        // so that the frame top appears at viewableAreaTopInViewport
                        
                        // Using the standard viewport formula: viewportY = (canvasY - canvasMinY) * zoom - contentY
                        // We want: viewableAreaTopInViewport = (frameTopY - canvasMinY) * zoom - contentY
                        // Solving for contentY: contentY = (frameTopY - canvasMinY) * zoom - viewableAreaTopInViewport
                        
                        // But moveToPoint sets contentY based on centering a canvas point
                        // When point P is centered: contentY = (P - canvasMinY) * zoom - viewportHeight/2
                        
                        // Setting these equal:
                        // (P - canvasMinY) * zoom - viewportHeight/2 = (frameTopY - canvasMinY) * zoom - viewableAreaTopInViewport
                        // (P - canvasMinY) * zoom = (frameTopY - canvasMinY) * zoom - viewableAreaTopInViewport + viewportHeight/2
                        // P = frameTopY + (viewportHeight/2 - viewableAreaTopInViewport) / zoom
                        
                        var frameTopY = canvasPoint.y  // This is the frame's top edge
                        var offset = (viewportHeight/2 - viewableAreaTopInViewport) / canvasView.zoom
                        
                        // Calculate the adjustment to align frame top with viewable area top
                        adjustedPoint.y = frameTopY + offset
                    }
                    
                    canvasView.moveToPoint(adjustedPoint, animated)
                }
            })
        }
    }
    
    // Prototype viewable area - visible only when prototyping
    PrototypeViewableArea {
        id: prototypeViewableArea
        visible: {
            var hasController = prototypeController !== null
            var isPrototyping = hasController && prototypeController.isPrototyping
            var isAnimating = controller && controller.isAnimating
            var shouldShow = hasController && isPrototyping && !isAnimating
            
            return shouldShow
        }
        designCanvas: canvasView
        flickable: root.flickable
        prototypeController: root.prototypeController
    }
}