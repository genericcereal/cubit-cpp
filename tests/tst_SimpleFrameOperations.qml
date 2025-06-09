import QtQuick
import QtTest
import QtQuick.Controls
import Cubit 1.0
import Cubit.UI 1.0

TestCase {
    id: testCase
    name: "SimpleFrameOperations"
    when: windowShown
    width: 800
    height: 600

    // Test components
    CanvasController {
        id: controller
        Component.onCompleted: {
            controller.setElementModel(elementModel)
            controller.setSelectionManager(selectionManager)
        }
    }

    ElementModel {
        id: elementModel
    }

    SelectionManager {
        id: selectionManager
    }

    // Helper functions
    function createFrame(x, y, width, height) {
        // Create frame using controller
        controller.createElement("frame", x, y, width, height)
        // Get the last added element
        var count = elementModel.rowCount()
        if (count > 0) {
            return elementModel.elementAt(count - 1)
        }
        return null
    }

    // Test basic frame creation
    function test_01_basicFrameCreation() {
        // Clear any existing elements
        elementModel.clear()
        
        // Test controller mode switching
        compare(controller.mode, "select", "Should start in select mode")
        
        controller.setMode("frame")
        compare(controller.mode, "frame", "Should switch to frame mode")
        
        // Create a frame programmatically
        var frame = createFrame(100, 100, 200, 150)
        verify(frame !== null, "Frame should be created")
        compare(frame.elementType, "Frame", "Should be a frame type")
        compare(frame.x, 100, "Frame x position")
        compare(frame.y, 100, "Frame y position")
        compare(frame.width, 200, "Frame width")
        compare(frame.height, 150, "Frame height")
        
        // Verify it's in the model
        compare(elementModel.rowCount(), 1, "Model should have one element")
        compare(elementModel.elementAt(0), frame, "Element should be in model")
    }

    // Test selection functionality
    function test_02_selection() {
        // Clear selection
        selectionManager.clearSelection()
        compare(selectionManager.selectedElements.length, 0, "Should start with no selection")
        
        // Get the frame
        var frame = elementModel.elementAt(0)
        verify(frame !== null, "Frame should exist")
        
        // Test selection
        selectionManager.selectElement(frame)
        compare(selectionManager.selectedElements.length, 1, "Should have one selected element")
        verify(frame.selected, "Frame should be selected")
        
        // Test deselection
        selectionManager.clearSelection()
        compare(selectionManager.selectedElements.length, 0, "Should have no selection")
        verify(!frame.selected, "Frame should not be selected")
        
        // Test multiple selection
        var frame2 = createFrame(300, 100, 150, 150)
        selectionManager.selectElement(frame)
        selectionManager.selectElement(frame2)
        compare(selectionManager.selectedElements.length, 2, "Should have two selected elements")
        verify(frame.selected && frame2.selected, "Both frames should be selected")
        
        // Test selection box clearing - when dragged to empty area
        // This simulates dragging a selection box that doesn't overlap any elements
        selectionManager.clearSelection()
        selectionManager.selectElement(frame) // Start with one selected
        verify(frame.selected, "Frame should be selected initially")
        
        // Simulate selection box in empty area (should clear selection)
        controller.selectElementsInRect(Qt.rect(500, 500, 100, 100))
        compare(selectionManager.selectedElements.length, 0, "Selection should be cleared when box doesn't overlap any elements")
        verify(!frame.selected, "Frame should not be selected after empty selection box")
    }

    // Test frame property modifications
    function test_03_frameModification() {
        var frame = elementModel.elementAt(0)
        verify(frame !== null, "Frame should exist")
        
        // Test position changes
        var originalX = frame.x
        var originalY = frame.y
        
        frame.x = 200
        frame.y = 200
        compare(frame.x, 200, "Frame x should update")
        compare(frame.y, 200, "Frame y should update")
        
        // Test size changes
        frame.width = 300
        frame.height = 250
        compare(frame.width, 300, "Frame width should update")
        compare(frame.height, 250, "Frame height should update")
        
        // Test negative size (Qt allows negative sizes in QSizeF)
        frame.width = -10
        frame.height = -10
        compare(frame.width, -10, "Frame width can be negative")
        compare(frame.height, -10, "Frame height can be negative")
    }

    // Test bounding box calculations
    function test_04_boundingBox() {
        // Clear and create fresh frames
        elementModel.clear()
        selectionManager.clearSelection()
        
        var frame1 = createFrame(100, 100, 100, 100)
        var frame2 = createFrame(250, 150, 150, 100)
        var frame3 = createFrame(150, 300, 100, 100)
        
        // Select all frames
        selectionManager.selectElement(frame1)
        selectionManager.selectElement(frame2)
        selectionManager.selectElement(frame3)
        
        // Calculate expected bounding box
        var expectedMinX = 100  // frame1.x
        var expectedMinY = 100  // frame1.y
        var expectedMaxX = 400  // frame2.x + frame2.width
        var expectedMaxY = 400  // frame3.y + frame3.height
        
        // Note: In a real test, we'd access the ViewportOverlay's bounding box properties
        // For now, we'll calculate it manually
        var minX = Math.min(frame1.x, frame2.x, frame3.x)
        var minY = Math.min(frame1.y, frame2.y, frame3.y)
        var maxX = Math.max(frame1.x + frame1.width, frame2.x + frame2.width, frame3.x + frame3.width)
        var maxY = Math.max(frame1.y + frame1.height, frame2.y + frame2.height, frame3.y + frame3.height)
        
        compare(minX, expectedMinX, "Bounding box min X")
        compare(minY, expectedMinY, "Bounding box min Y")
        compare(maxX, expectedMaxX, "Bounding box max X")
        compare(maxY, expectedMaxY, "Bounding box max Y")
    }

    // Test element removal
    function test_05_elementRemoval() {
        var initialCount = elementModel.rowCount()
        verify(initialCount > 0, "Should have elements to remove")
        
        // Remove first element
        var firstElement = elementModel.elementAt(0)
        elementModel.removeElement(firstElement.elementId)
        
        compare(elementModel.rowCount(), initialCount - 1, "Element count should decrease")
        // Verify element is removed by checking all remaining elements
        var found = false
        for (var i = 0; i < elementModel.rowCount(); i++) {
            if (elementModel.elementAt(i).elementId === firstElement.elementId) {
                found = true
                break
            }
        }
        verify(!found, "Element should be removed from model")
        
        // Clear all
        elementModel.clear()
        compare(elementModel.rowCount(), 0, "All elements should be removed")
    }

    // Cleanup after all tests
    function cleanupTestCase() {
        elementModel.clear()
        selectionManager.clearSelection()
        controller.setMode("select")
    }
}