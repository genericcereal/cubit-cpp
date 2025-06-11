import QtQuick
import QtTest
import Cubit 1.0
import "../qml"
import "../qml/components"

TestCase {
    id: testCase
    name: "ResizeTests"
    width: 800
    height: 600
    visible: true
    when: windowShown

    property var canvasController: null
    property var elementModel: null
    property var selectionManager: null
    property var canvasView: null
    property var controls: null

    Component {
        id: canvasComponent
        
        Item {
            width: 800
            height: 600
            
            CanvasView {
                id: canvas
                anchors.fill: parent
                controller: testCase.canvasController
                elementModel: testCase.elementModel
                selectionManager: testCase.selectionManager
            }
            
            ViewportOverlay {
                id: overlay
                anchors.fill: parent
                canvasView: canvas
                selectionManager: testCase.selectionManager
            }
        }
    }

    function initTestCase() {
        // Create C++ backend objects
        canvasController = Qt.createQmlObject('import Cubit 1.0; CanvasController {}', testCase)
        elementModel = Qt.createQmlObject('import Cubit 1.0; ElementModel {}', testCase)
        selectionManager = Qt.createQmlObject('import Cubit 1.0; SelectionManager {}', testCase)
        
        // Connect backend objects
        canvasController.elementModel = elementModel
        canvasController.selectionManager = selectionManager
    }

    function init() {
        // Create fresh canvas for each test
        var container = canvasComponent.createObject(testCase)
        verify(container !== null, "Container should be created")
        canvasView = container.children[0]
        controls = container.children[1].controls
        wait(100) // Allow components to initialize
    }

    function cleanup() {
        if (canvasView && canvasView.parent) {
            canvasView.parent.destroy()
        }
        selectionManager.clearSelection()
        elementModel.clear()
    }

    function test_resizeSingleElementFromRightBar() {
        // Create and select a frame element
        canvasController.setActiveType("Frame")
        mousePress(canvasView, 100, 100)
        mouseMove(canvasView, 200, 200)
        mouseRelease(canvasView, 200, 200)
        wait(50)
        
        canvasController.setActiveType("Select")
        mouseClick(canvasView, 150, 150)
        wait(100)
        
        // Verify controls are visible
        verify(controls.visible, "Controls should be visible when element is selected")
        
        // Get the initial element size
        var element = selectionManager.selectedElements[0]
        var initialWidth = element.width
        var initialHeight = element.height
        
        // Find the right bar position (should be at x = 200 - 5 for the overlap)
        var rightBarX = element.x + element.width - 5
        var rightBarY = element.y + element.height / 2
        
        // Drag the right bar to resize
        mousePress(canvasView, rightBarX, rightBarY)
        mouseMove(canvasView, rightBarX + 50, rightBarY)
        mouseRelease(canvasView, rightBarX + 50, rightBarY)
        wait(100)
        
        // Verify element was resized
        compare(element.width, initialWidth + 50, "Element width should increase by 50")
        compare(element.height, initialHeight, "Element height should remain unchanged")
        compare(element.x, 100, "Element x position should remain unchanged")
        compare(element.y, 100, "Element y position should remain unchanged")
    }

    function test_resizeSingleElementFromCornerJoint() {
        // Create and select a frame element
        canvasController.setActiveType("Frame")
        mousePress(canvasView, 100, 100)
        mouseMove(canvasView, 200, 200)
        mouseRelease(canvasView, 200, 200)
        wait(50)
        
        canvasController.setActiveType("Select")
        mouseClick(canvasView, 150, 150)
        wait(100)
        
        // Get the initial element size
        var element = selectionManager.selectedElements[0]
        var initialWidth = element.width
        var initialHeight = element.height
        
        // Find the bottom-right corner joint position
        var cornerX = element.x + element.width
        var cornerY = element.y + element.height
        
        // Drag the corner joint to resize
        mousePress(canvasView, cornerX, cornerY)
        mouseMove(canvasView, cornerX + 50, cornerY + 30)
        mouseRelease(canvasView, cornerX + 50, cornerY + 30)
        wait(100)
        
        // Verify element was resized
        compare(element.width, initialWidth + 50, "Element width should increase by 50")
        compare(element.height, initialHeight + 30, "Element height should increase by 30")
        compare(element.x, 100, "Element x position should remain unchanged")
        compare(element.y, 100, "Element y position should remain unchanged")
    }

    function test_resizeMultipleElementsProportionally() {
        // Create two frame elements
        canvasController.setActiveType("Frame")
        
        // First frame: 100x100
        mousePress(canvasView, 50, 50)
        mouseMove(canvasView, 150, 150)
        mouseRelease(canvasView, 150, 150)
        wait(50)
        
        // Second frame: 50x50
        mousePress(canvasView, 200, 200)
        mouseMove(canvasView, 250, 250)
        mouseRelease(canvasView, 250, 250)
        wait(50)
        
        // Select both elements
        canvasController.setActiveType("Select")
        keyPress("A", Qt.ControlModifier)
        wait(100)
        
        // Verify both are selected
        compare(selectionManager.selectedElements.length, 2, "Both elements should be selected")
        
        // Get initial sizes
        var element1 = elementModel.elementAt(0)
        var element2 = elementModel.elementAt(1)
        var initialWidth1 = element1.width
        var initialHeight1 = element1.height
        var initialWidth2 = element2.width
        var initialHeight2 = element2.height
        
        // Find the right edge of the selection bounds
        var selectionRight = Math.max(element1.x + element1.width, element2.x + element2.width)
        var selectionCenterY = (Math.min(element1.y, element2.y) + 
                               Math.max(element1.y + element1.height, element2.y + element2.height)) / 2
        
        // Drag the right bar to resize (scale by 1.5x)
        mousePress(canvasView, selectionRight - 5, selectionCenterY)
        mouseMove(canvasView, selectionRight + 50, selectionCenterY)
        mouseRelease(canvasView, selectionRight + 50, selectionCenterY)
        wait(100)
        
        // Calculate expected scale factor
        var expectedScale = (selectionRight - Math.min(element1.x, element2.x) + 50) / 
                           (selectionRight - Math.min(element1.x, element2.x))
        
        // Verify proportional scaling
        fuzzyCompare(element1.width / initialWidth1, expectedScale, 0.1, 
                     "First element should scale proportionally")
        fuzzyCompare(element2.width / initialWidth2, expectedScale, 0.1, 
                     "Second element should scale proportionally")
    }

    function test_resizeWithMinimumSize() {
        // Create a small frame element
        canvasController.setActiveType("Frame")
        mousePress(canvasView, 100, 100)
        mouseMove(canvasView, 110, 110)
        mouseRelease(canvasView, 110, 110)
        wait(50)
        
        canvasController.setActiveType("Select")
        mouseClick(canvasView, 105, 105)
        wait(100)
        
        var element = selectionManager.selectedElements[0]
        
        // Try to resize below minimum size (1x1)
        var leftBarX = element.x + 5
        var leftBarY = element.y + element.height / 2
        
        mousePress(canvasView, leftBarX, leftBarY)
        mouseMove(canvasView, leftBarX + 20, leftBarY) // Try to make it negative width
        mouseRelease(canvasView, leftBarX + 20, leftBarY)
        wait(100)
        
        // Verify minimum size is enforced
        verify(element.width >= 1, "Element width should not go below 1 pixel")
        verify(element.height >= 1, "Element height should not go below 1 pixel")
    }

    function test_resizeWithEdgeFlipping() {
        // Create a frame element
        canvasController.setActiveType("Frame")
        mousePress(canvasView, 100, 100)
        mouseMove(canvasView, 200, 200)
        mouseRelease(canvasView, 200, 200)
        wait(50)
        
        canvasController.setActiveType("Select")
        mouseClick(canvasView, 150, 150)
        wait(100)
        
        var element = selectionManager.selectedElements[0]
        var initialX = element.x
        var initialWidth = element.width
        
        // Drag left edge past right edge
        var leftBarX = element.x + 5
        var leftBarY = element.y + element.height / 2
        
        mousePress(canvasView, leftBarX, leftBarY)
        mouseMove(canvasView, leftBarX + initialWidth + 50, leftBarY)
        mouseRelease(canvasView, leftBarX + initialWidth + 50, leftBarY)
        wait(100)
        
        // Verify edge flipping occurred
        verify(element.x > initialX, "Element should have flipped and moved right")
        compare(element.width, 50, "Element should have new width after flipping")
    }

    function test_resizeRotatedElement() {
        skip("Rotation functionality not yet implemented")
        
        // Create and rotate a frame element
        canvasController.setActiveType("Frame")
        mousePress(canvasView, 100, 100)
        mouseMove(canvasView, 200, 200)
        mouseRelease(canvasView, 200, 200)
        wait(50)
        
        canvasController.setActiveType("Select")
        mouseClick(canvasView, 150, 150)
        wait(100)
        
        var element = selectionManager.selectedElements[0]
        element.rotation = 45 // Rotate 45 degrees
        wait(100)
        
        // Test that resize still works correctly with rotation
        // This would require rotation-aware coordinate transformation
    }
}