import QtQuick
import QtQuick.Window
import QtQuick.Controls
import QtQuick.Layouts
import QtTest
import "../qml"
import "../qml/components"
import "../qml/panels"
import Cubit 1.0
import Cubit.UI 1.0

TestCase {
    id: testCase
    name: "FrameOperations"
    when: windowShown
    width: 1024
    height: 768

    // Main application window
    Window {
        id: mainWindow
        width: testCase.width
        height: testCase.height
        visible: true

        // Controllers
        CanvasController {
            id: controller
        }

        ElementModel {
            id: elementModel
        }

        SelectionManager {
            id: selectionManager
        }

        // Main UI
        Rectangle {
            anchors.fill: parent
            color: "#f0f0f0"

            SplitView {
                anchors.fill: parent
                orientation: Qt.Horizontal

                // Canvas area
                Item {
                    SplitView.fillWidth: true
                    SplitView.minimumWidth: 400

                    CanvasView {
                        id: canvasView
                        anchors.fill: parent
                        controller: controller
                        elementModel: elementModel
                        selectionManager: selectionManager
                    }

                    ViewportOverlay {
                        id: viewportOverlay
                        anchors.fill: parent
                        canvasView: canvasView
                        hoveredElement: canvasView.hoveredElement
                    }

                    ActionsPanel {
                        id: actionsPanel
                        anchors.bottom: parent.bottom
                        anchors.horizontalCenter: parent.horizontalCenter
                        controller: controller
                    }
                }

                // Detail panel
                DetailPanel {
                    id: detailPanel
                    SplitView.preferredWidth: 300
                    SplitView.minimumWidth: 200
                    elementModel: elementModel
                    selectionManager: selectionManager
                }
            }
        }
    }

    // Helper function to wait for animations and updates
    function waitForRendering() {
        wait(100)
        waitForItemPolished(canvasView)
    }

    // Helper function to get canvas center
    function getCanvasCenter() {
        return Qt.point(canvasView.width / 2, canvasView.height / 2)
    }

    // Helper function to create a point relative to canvas
    function canvasPoint(x, y) {
        var canvasPos = canvasView.mapToGlobal(0, 0)
        return Qt.point(canvasPos.x + x, canvasPos.y + y)
    }

    // Test 1: Frame Creation
    function test_01_frameCreation() {
        // Initial state should be select mode
        compare(controller.mode, "select", "Should start in select mode")
        
        // Find and click the Frame button in ActionsPanel
        var frameButton = findChild(actionsPanel, "frameButton")
        verify(frameButton, "Frame button should exist")
        
        // Switch to frame mode
        mouseClick(frameButton)
        waitForRendering()
        compare(controller.mode, "frame", "Should switch to frame mode")
        
        // Draw a frame by dragging
        var startX = 100
        var startY = 100
        var endX = 300
        var endY = 250
        
        // Start drag
        mousePress(canvasView, startX, startY)
        waitForRendering()
        
        // Verify creation drag handler is active
        verify(canvasView.creationDragHandler.active, "Creation drag should be active")
        
        // Drag to create frame
        mouseMove(canvasView, endX, endY)
        waitForRendering()
        
        // Release to finish creation
        mouseRelease(canvasView, endX, endY)
        waitForRendering()
        
        // Verify frame was created
        compare(elementModel.elements.length, 1, "Should have created one element")
        var frame = elementModel.elements[0]
        verify(frame, "Frame should exist")
        compare(frame.type, Element.FrameType, "Element should be a frame")
        
        // Verify frame dimensions (approximately)
        fuzzyCompare(frame.x, startX, 5, "Frame x position")
        fuzzyCompare(frame.y, startY, 5, "Frame y position")
        fuzzyCompare(frame.width, endX - startX, 5, "Frame width")
        fuzzyCompare(frame.height, endY - startY, 5, "Frame height")
        
        // Should return to select mode
        compare(controller.mode, "select", "Should return to select mode after creation")
    }

    // Test 2: Selection with selection box
    function test_02_selection() {
        // Ensure we're in select mode
        if (controller.mode !== "select") {
            var selectButton = findChild(actionsPanel, "selectButton")
            mouseClick(selectButton)
            waitForRendering()
        }
        
        // Clear any existing selection
        selectionManager.clearSelection()
        compare(selectionManager.selectedElements.length, 0, "Should start with no selection")
        
        // Get the frame from previous test
        var frame = elementModel.elements[0]
        verify(frame, "Frame should exist from previous test")
        
        // Test 1: Selection box that includes the frame
        var boxStartX = frame.x - 20
        var boxStartY = frame.y - 20
        var boxEndX = frame.x + frame.width + 20
        var boxEndY = frame.y + frame.height + 20
        
        // Start selection box
        mousePress(canvasView, boxStartX, boxStartY)
        waitForRendering()
        
        // Verify selection box is active
        verify(canvasView.selectionBoxHandler.active, "Selection box should be active")
        
        // Drag selection box
        mouseMove(canvasView, boxEndX, boxEndY)
        waitForRendering()
        
        // Verify live selection (frame should be selected while dragging)
        verify(frame.selected, "Frame should be selected during selection box drag")
        
        // Release to finish selection
        mouseRelease(canvasView, boxEndX, boxEndY)
        waitForRendering()
        
        // Verify final selection
        compare(selectionManager.selectedElements.length, 1, "Should have one selected element")
        verify(frame.selected, "Frame should remain selected")
        
        // Test 2: Selection box that doesn't include the frame
        selectionManager.clearSelection()
        waitForRendering()
        
        boxStartX = frame.x + frame.width + 50
        boxStartY = frame.y + frame.height + 50
        boxEndX = boxStartX + 100
        boxEndY = boxStartY + 100
        
        mousePress(canvasView, boxStartX, boxStartY)
        mouseMove(canvasView, boxEndX, boxEndY)
        waitForRendering()
        
        // Frame should not be selected
        verify(!frame.selected, "Frame should not be selected by non-overlapping box")
        
        mouseRelease(canvasView, boxEndX, boxEndY)
        waitForRendering()
        
        compare(selectionManager.selectedElements.length, 0, "Should have no selected elements")
        
        // Test 3: Direct click selection
        mouseClick(canvasView, frame.x + frame.width/2, frame.y + frame.height/2)
        waitForRendering()
        
        verify(frame.selected, "Frame should be selected by direct click")
        compare(selectionManager.selectedElements.length, 1, "Should have one selected element")
        
        // Test 4: Selection box clearing when moved to empty area
        // Start with frame selected
        verify(frame.selected, "Frame should still be selected")
        
        // Drag selection box in empty area (should clear selection during drag)
        boxStartX = frame.x + frame.width + 100
        boxStartY = frame.y + frame.height + 100
        boxEndX = boxStartX + 50
        boxEndY = boxStartY + 50
        
        mousePress(canvasView, boxStartX, boxStartY)
        waitForRendering()
        
        // During drag, selection should update live
        mouseMove(canvasView, boxEndX, boxEndY)
        waitForRendering()
        
        // Frame should be deselected while dragging empty selection box
        verify(!frame.selected, "Frame should be deselected during empty selection box drag")
        
        mouseRelease(canvasView, boxEndX, boxEndY)
        waitForRendering()
        
        compare(selectionManager.selectedElements.length, 0, "Should have no selected elements after empty selection box")
    }

    // Test 3: Frame Resizing
    function test_03_frameResizing() {
        // Ensure frame is selected
        var frame = elementModel.elements[0]
        if (!frame.selected) {
            mouseClick(canvasView, frame.x + frame.width/2, frame.y + frame.height/2)
            waitForRendering()
        }
        
        // Store original dimensions
        var originalX = frame.x
        var originalY = frame.y
        var originalWidth = frame.width
        var originalHeight = frame.height
        
        // Test 1: Resize using top bar
        var topBarY = frame.y - 5 // Just above the frame
        var topBarX = frame.x + frame.width/2
        
        mousePress(canvasView, topBarX, topBarY)
        waitForRendering()
        
        // Drag up to make frame taller
        var dragDistance = 50
        mouseMove(canvasView, topBarX, topBarY - dragDistance)
        waitForRendering()
        
        mouseRelease(canvasView, topBarX, topBarY - dragDistance)
        waitForRendering()
        
        // Verify resize
        fuzzyCompare(frame.y, originalY - dragDistance, 5, "Frame should move up")
        fuzzyCompare(frame.height, originalHeight + dragDistance, 5, "Frame should be taller")
        fuzzyCompare(frame.x, originalX, 5, "Frame x should not change")
        fuzzyCompare(frame.width, originalWidth, 5, "Frame width should not change")
        
        // Test 2: Resize using right bar
        originalX = frame.x
        originalY = frame.y
        originalWidth = frame.width
        originalHeight = frame.height
        
        var rightBarX = frame.x + frame.width + 5
        var rightBarY = frame.y + frame.height/2
        
        mousePress(canvasView, rightBarX, rightBarY)
        mouseMove(canvasView, rightBarX + dragDistance, rightBarY)
        mouseRelease(canvasView, rightBarX + dragDistance, rightBarY)
        waitForRendering()
        
        fuzzyCompare(frame.width, originalWidth + dragDistance, 5, "Frame should be wider")
        fuzzyCompare(frame.x, originalX, 5, "Frame x should not change")
        fuzzyCompare(frame.y, originalY, 5, "Frame y should not change")
        fuzzyCompare(frame.height, originalHeight, 5, "Frame height should not change")
        
        // Test 3: Flip test - drag left bar past right edge
        originalX = frame.x
        originalWidth = frame.width
        
        var leftBarX = frame.x - 5
        var leftBarY = frame.y + frame.height/2
        var beyondRightEdge = frame.x + frame.width + 20
        
        mousePress(canvasView, leftBarX, leftBarY)
        mouseMove(canvasView, beyondRightEdge, leftBarY)
        mouseRelease(canvasView, beyondRightEdge, leftBarY)
        waitForRendering()
        
        // Frame should have flipped
        verify(frame.width > 0, "Frame width should remain positive")
        verify(frame.x > originalX + originalWidth, "Frame should have flipped to the right")
        
        // Test 4: Corner resize using joint
        originalX = frame.x
        originalY = frame.y
        originalWidth = frame.width
        originalHeight = frame.height
        
        // Find bottom-right resize joint
        var bottomRightX = frame.x + frame.width
        var bottomRightY = frame.y + frame.height
        
        mousePress(canvasView, bottomRightX, bottomRightY)
        mouseMove(canvasView, bottomRightX + 30, bottomRightY + 30)
        mouseRelease(canvasView, bottomRightX + 30, bottomRightY + 30)
        waitForRendering()
        
        // Both dimensions should increase
        verify(frame.width > originalWidth, "Frame should be wider from corner resize")
        verify(frame.height > originalHeight, "Frame should be taller from corner resize")
        fuzzyCompare(frame.x, originalX, 5, "Frame x should not change")
        fuzzyCompare(frame.y, originalY, 5, "Frame y should not change")
    }

    // Test 4: Multiple element selection and resize
    function test_04_multipleSelection() {
        // Create a second frame
        var frameButton = findChild(actionsPanel, "frameButton")
        mouseClick(frameButton)
        waitForRendering()
        
        mousePress(canvasView, 400, 100)
        mouseMove(canvasView, 550, 200)
        mouseRelease(canvasView, 550, 200)
        waitForRendering()
        
        compare(elementModel.elements.length, 2, "Should have two frames")
        
        // Select both frames with selection box
        selectionManager.clearSelection()
        
        mousePress(canvasView, 50, 50)
        mouseMove(canvasView, 600, 300)
        mouseRelease(canvasView, 600, 300)
        waitForRendering()
        
        compare(selectionManager.selectedElements.length, 2, "Should have two selected elements")
        
        // Store original positions
        var frame1 = elementModel.elements[0]
        var frame2 = elementModel.elements[1]
        var originalRatio = frame2.width / frame1.width
        
        // Resize both using right bar
        var boundingRight = Math.max(frame1.x + frame1.width, frame2.x + frame2.width)
        var centerY = (Math.min(frame1.y, frame2.y) + 
                      Math.max(frame1.y + frame1.height, frame2.y + frame2.height)) / 2
        
        mousePress(canvasView, boundingRight + 5, centerY)
        mouseMove(canvasView, boundingRight + 55, centerY)
        mouseRelease(canvasView, boundingRight + 55, centerY)
        waitForRendering()
        
        // Both frames should have scaled proportionally
        var newRatio = frame2.width / frame1.width
        fuzzyCompare(newRatio, originalRatio, 0.1, "Frames should maintain relative proportions")
    }

    // Cleanup
    function cleanup() {
        // Clear selection after each test
        selectionManager.clearSelection()
        waitForRendering()
    }

    function cleanupTestCase() {
        // Clear all elements
        while (elementModel.elements.length > 0) {
            elementModel.removeElement(elementModel.elements[0])
        }
    }
}