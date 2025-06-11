import QtQuick
import QtTest
import Cubit 1.0
import "../qml"
import "../qml/components"

TestCase {
    id: testCase
    name: "MovementTests"
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

    function test_moveSingleElement() {
        // Create and select a frame element
        canvasController.setActiveType("Frame")
        mousePress(canvasView, 100, 100)
        mouseMove(canvasView, 200, 200)
        mouseRelease(canvasView, 200, 200)
        wait(50)
        
        canvasController.setActiveType("Select")
        mouseClick(canvasView, 150, 150)
        wait(100)
        
        // Get initial position
        var element = selectionManager.selectedElements[0]
        var initialX = element.x
        var initialY = element.y
        
        // Drag the control surface to move
        mousePress(canvasView, 150, 150)
        mouseMove(canvasView, 250, 250)
        mouseRelease(canvasView, 250, 250)
        wait(100)
        
        // Verify element moved
        compare(element.x, initialX + 100, "Element x should move by 100")
        compare(element.y, initialY + 100, "Element y should move by 100")
    }

    function test_moveMultipleElements() {
        // Create two frame elements
        canvasController.setActiveType("Frame")
        
        // First frame
        mousePress(canvasView, 50, 50)
        mouseMove(canvasView, 150, 150)
        mouseRelease(canvasView, 150, 150)
        wait(50)
        
        // Second frame
        mousePress(canvasView, 200, 200)
        mouseMove(canvasView, 300, 300)
        mouseRelease(canvasView, 300, 300)
        wait(50)
        
        // Select both elements
        canvasController.setActiveType("Select")
        keyPress("A", Qt.ControlModifier)
        wait(100)
        
        // Get initial positions
        var element1 = elementModel.elementAt(0)
        var element2 = elementModel.elementAt(1)
        var initialX1 = element1.x
        var initialY1 = element1.y
        var initialX2 = element2.x
        var initialY2 = element2.y
        
        // Calculate center of selection
        var centerX = (element1.x + element1.width/2 + element2.x + element2.width/2) / 2
        var centerY = (element1.y + element1.height/2 + element2.y + element2.height/2) / 2
        
        // Drag from center to move both
        mousePress(canvasView, centerX, centerY)
        mouseMove(canvasView, centerX + 50, centerY + 75)
        mouseRelease(canvasView, centerX + 50, centerY + 75)
        wait(100)
        
        // Verify both elements moved by the same amount
        compare(element1.x, initialX1 + 50, "First element x should move by 50")
        compare(element1.y, initialY1 + 75, "First element y should move by 75")
        compare(element2.x, initialX2 + 50, "Second element x should move by 50")
        compare(element2.y, initialY2 + 75, "Second element y should move by 75")
    }

    function test_moveWithCanvasScroll() {
        // Create element near edge of canvas
        canvasController.setActiveType("Frame")
        mousePress(canvasView, 50, 50)
        mouseMove(canvasView, 150, 150)
        mouseRelease(canvasView, 150, 150)
        wait(50)
        
        canvasController.setActiveType("Select")
        mouseClick(canvasView, 100, 100)
        wait(100)
        
        var element = selectionManager.selectedElements[0]
        var initialX = element.x
        var initialY = element.y
        
        // Scroll the canvas
        canvasView.contentX = 100
        canvasView.contentY = 100
        wait(50)
        
        // Move element (coordinates should be canvas-relative, not viewport-relative)
        mousePress(canvasView, 100, 100) // This is now at canvas position 200, 200
        mouseMove(canvasView, 150, 150)  // Move to canvas position 250, 250
        mouseRelease(canvasView, 150, 150)
        wait(100)
        
        // Verify element moved correctly in canvas coordinates
        compare(element.x, initialX + 50, "Element should move by 50 in canvas coordinates")
        compare(element.y, initialY + 50, "Element should move by 50 in canvas coordinates")
    }

    function test_moveWithZoom() {
        // Create element
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
        var initialY = element.y
        
        // Zoom in 2x
        canvasView.zoomLevel = 2.0
        wait(50)
        
        // Move element (mouse movement should be scaled by zoom)
        var viewX = (150 - canvasView.contentX) * canvasView.zoomLevel
        var viewY = (150 - canvasView.contentY) * canvasView.zoomLevel
        
        mousePress(canvasView, viewX, viewY)
        mouseMove(canvasView, viewX + 100, viewY + 100) // 100 pixels in view = 50 in canvas
        mouseRelease(canvasView, viewX + 100, viewY + 100)
        wait(100)
        
        // Verify element moved correctly accounting for zoom
        compare(element.x, initialX + 50, "Element should move by 50 canvas pixels")
        compare(element.y, initialY + 50, "Element should move by 50 canvas pixels")
    }

    function test_clickToSelectFromMultiple() {
        // Create three overlapping frame elements
        canvasController.setActiveType("Frame")
        
        // First frame (bottom)
        mousePress(canvasView, 50, 50)
        mouseMove(canvasView, 200, 200)
        mouseRelease(canvasView, 200, 200)
        wait(50)
        
        // Second frame (middle)
        mousePress(canvasView, 100, 100)
        mouseMove(canvasView, 250, 250)
        mouseRelease(canvasView, 250, 250)
        wait(50)
        
        // Third frame (top)
        mousePress(canvasView, 150, 150)
        mouseMove(canvasView, 300, 300)
        mouseRelease(canvasView, 300, 300)
        wait(50)
        
        // Select all elements
        canvasController.setActiveType("Select")
        keyPress("A", Qt.ControlModifier)
        wait(100)
        
        compare(selectionManager.selectedElements.length, 3, "All three elements should be selected")
        
        // Click on overlapping area to select single element
        mouseClick(canvasView, 175, 175)
        wait(100)
        
        // Should select only one element (the topmost one under cursor)
        compare(selectionManager.selectedElements.length, 1, "Only one element should be selected")
        var selectedElement = selectionManager.selectedElements[0]
        
        // Verify it's the topmost element (third one created)
        compare(selectedElement, elementModel.elementAt(2), "Topmost element should be selected")
    }

    function test_movePreservesRelativePositions() {
        // Create elements in specific arrangement
        canvasController.setActiveType("Frame")
        
        // Create square arrangement
        var positions = [
            {x: 100, y: 100}, // Top-left
            {x: 200, y: 100}, // Top-right
            {x: 100, y: 200}, // Bottom-left
            {x: 200, y: 200}  // Bottom-right
        ]
        
        for (var i = 0; i < positions.length; i++) {
            mousePress(canvasView, positions[i].x, positions[i].y)
            mouseMove(canvasView, positions[i].x + 50, positions[i].y + 50)
            mouseRelease(canvasView, positions[i].x + 50, positions[i].y + 50)
            wait(50)
        }
        
        // Select all elements
        canvasController.setActiveType("Select")
        keyPress("A", Qt.ControlModifier)
        wait(100)
        
        // Calculate relative positions
        var elements = []
        var relativePositions = []
        for (var j = 0; j < 4; j++) {
            elements.push(elementModel.elementAt(j))
            if (j > 0) {
                relativePositions.push({
                    dx: elements[j].x - elements[0].x,
                    dy: elements[j].y - elements[0].y
                })
            }
        }
        
        // Move all elements
        mousePress(canvasView, 175, 175) // Center of arrangement
        mouseMove(canvasView, 275, 275)
        mouseRelease(canvasView, 275, 275)
        wait(100)
        
        // Verify relative positions are preserved
        for (var k = 1; k < 4; k++) {
            compare(elements[k].x - elements[0].x, relativePositions[k-1].dx, 
                    "Relative x position should be preserved for element " + k)
            compare(elements[k].y - elements[0].y, relativePositions[k-1].dy, 
                    "Relative y position should be preserved for element " + k)
        }
    }
}