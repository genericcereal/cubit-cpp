import QtQuick
import QtTest
import Cubit 1.0
import "../qml"
import "../qml/components"

TestCase {
    id: testCase
    name: "SelectionTests"
    width: 800
    height: 600
    visible: true
    when: windowShown

    property var canvasController: null
    property var elementModel: null
    property var selectionManager: null
    property var canvasView: null

    Component {
        id: canvasComponent
        
        CanvasView {
            width: 800
            height: 600
            controller: testCase.canvasController
            elementModel: testCase.elementModel
            selectionManager: testCase.selectionManager
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
        canvasView = canvasComponent.createObject(testCase)
        verify(canvasView !== null, "Canvas view should be created")
        wait(100) // Allow canvas to initialize
    }

    function cleanup() {
        if (canvasView) {
            canvasView.destroy()
            canvasView = null
        }
        selectionManager.clearSelection()
        elementModel.clear()
    }

    function test_singleElementSelection() {
        // Create a frame element
        canvasController.setActiveType("Frame")
        var startX = 100
        var startY = 100
        var endX = 200
        var endY = 200
        
        // Simulate drag to create element
        mousePress(canvasView, startX, startY)
        mouseMove(canvasView, endX, endY)
        mouseRelease(canvasView, endX, endY)
        
        wait(100)
        
        // Verify element was created
        compare(elementModel.rowCount(), 1, "One element should be created")
        
        // Click on the element to select it
        mouseClick(canvasView, 150, 150)
        wait(100)
        
        // Verify selection
        compare(selectionManager.selectedElements.length, 1, "One element should be selected")
        var selectedElement = selectionManager.selectedElements[0]
        verify(selectedElement !== null, "Selected element should not be null")
        compare(selectedElement.type, "Frame", "Selected element should be a Frame")
    }

    function test_multipleElementSelection() {
        // Create multiple frame elements
        canvasController.setActiveType("Frame")
        
        // Create first frame
        mousePress(canvasView, 50, 50)
        mouseMove(canvasView, 150, 150)
        mouseRelease(canvasView, 150, 150)
        wait(50)
        
        // Create second frame
        mousePress(canvasView, 200, 50)
        mouseMove(canvasView, 300, 150)
        mouseRelease(canvasView, 300, 150)
        wait(50)
        
        // Create third frame
        mousePress(canvasView, 50, 200)
        mouseMove(canvasView, 150, 300)
        mouseRelease(canvasView, 150, 300)
        wait(50)
        
        // Verify all elements were created
        compare(elementModel.rowCount(), 3, "Three elements should be created")
        
        // Switch to select mode
        canvasController.setActiveType("Select")
        
        // Draw selection box around all elements
        mousePress(canvasView, 25, 25)
        mouseMove(canvasView, 325, 325)
        mouseRelease(canvasView, 325, 325)
        wait(100)
        
        // Verify all elements are selected
        compare(selectionManager.selectedElements.length, 3, "All three elements should be selected")
    }

    function test_selectionBoxPartialOverlap() {
        // Create two frame elements
        canvasController.setActiveType("Frame")
        
        // Create first frame
        mousePress(canvasView, 50, 50)
        mouseMove(canvasView, 150, 150)
        mouseRelease(canvasView, 150, 150)
        wait(50)
        
        // Create second frame
        mousePress(canvasView, 200, 200)
        mouseMove(canvasView, 300, 300)
        mouseRelease(canvasView, 300, 300)
        wait(50)
        
        // Switch to select mode
        canvasController.setActiveType("Select")
        
        // Draw selection box that partially overlaps both elements
        mousePress(canvasView, 100, 100)
        mouseMove(canvasView, 250, 250)
        mouseRelease(canvasView, 250, 250)
        wait(100)
        
        // Verify both elements are selected (assuming partial overlap selects)
        compare(selectionManager.selectedElements.length, 2, "Both elements should be selected with partial overlap")
    }

    function test_clearSelection() {
        // Create a frame element
        canvasController.setActiveType("Frame")
        mousePress(canvasView, 50, 50)
        mouseMove(canvasView, 150, 150)
        mouseRelease(canvasView, 150, 150)
        wait(50)
        
        // Select the element
        canvasController.setActiveType("Select")
        mouseClick(canvasView, 100, 100)
        wait(50)
        
        // Verify selection
        compare(selectionManager.selectedElements.length, 1, "One element should be selected")
        
        // Click on empty space to clear selection
        mouseClick(canvasView, 400, 400)
        wait(50)
        
        // Verify selection is cleared
        compare(selectionManager.selectedElements.length, 0, "Selection should be cleared")
    }

    function test_selectAllShortcut() {
        // Create multiple elements
        canvasController.setActiveType("Frame")
        
        for (var i = 0; i < 3; i++) {
            mousePress(canvasView, 50 + i * 100, 50)
            mouseMove(canvasView, 100 + i * 100, 100)
            mouseRelease(canvasView, 100 + i * 100, 100)
            wait(50)
        }
        
        // Verify elements were created
        compare(elementModel.rowCount(), 3, "Three elements should be created")
        
        // Press Ctrl+A
        keyPress("A", Qt.ControlModifier)
        wait(100)
        
        // Verify all elements are selected
        compare(selectionManager.selectedElements.length, 3, "All elements should be selected with Ctrl+A")
    }

    function test_deleteSelectedElements() {
        // Create two elements
        canvasController.setActiveType("Frame")
        
        mousePress(canvasView, 50, 50)
        mouseMove(canvasView, 150, 150)
        mouseRelease(canvasView, 150, 150)
        wait(50)
        
        mousePress(canvasView, 200, 200)
        mouseMove(canvasView, 300, 300)
        mouseRelease(canvasView, 300, 300)
        wait(50)
        
        compare(elementModel.rowCount(), 2, "Two elements should be created")
        
        // Select first element
        canvasController.setActiveType("Select")
        mouseClick(canvasView, 100, 100)
        wait(50)
        
        // Press Delete key
        keyPress(Qt.Key_Delete)
        wait(100)
        
        // Verify element was deleted
        compare(elementModel.rowCount(), 1, "One element should remain after deletion")
        compare(selectionManager.selectedElements.length, 0, "No elements should be selected after deletion")
    }
}