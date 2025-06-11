import QtQuick
import QtTest
import Cubit 1.0
import "../qml"
import "../qml/components"

TestCase {
    id: testCase
    name: "CanvasInteractionTests"
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

    function test_canvasZoomWithMouseWheel() {
        var initialZoom = canvasView.zoomLevel
        compare(initialZoom, 1.0, "Initial zoom should be 1.0")
        
        // Create an element to have a reference point
        canvasController.setActiveType("Frame")
        mousePress(canvasView, 200, 200)
        mouseMove(canvasView, 300, 300)
        mouseRelease(canvasView, 300, 300)
        wait(50)
        
        // Zoom in with Ctrl+wheel
        var centerX = 400
        var centerY = 300
        
        // Simulate wheel event with Ctrl modifier
        canvasView.forceActiveFocus()
        keyPress(Qt.Key_Control)
        wheel(canvasView, centerX, centerY, 0, 120) // 120 = one notch up
        keyRelease(Qt.Key_Control)
        wait(100)
        
        // Verify zoom increased
        verify(canvasView.zoomLevel > initialZoom, "Zoom level should increase")
        fuzzyCompare(canvasView.zoomLevel, 1.1, 0.01, "Zoom should be approximately 1.1")
        
        // Zoom out
        keyPress(Qt.Key_Control)
        wheel(canvasView, centerX, centerY, 0, -240) // Two notches down
        keyRelease(Qt.Key_Control)
        wait(100)
        
        // Verify zoom decreased
        verify(canvasView.zoomLevel < 1.0, "Zoom level should be less than 1.0")
    }

    function test_canvasPanWithMiddleMouse() {
        // Create elements to have reference points
        canvasController.setActiveType("Frame")
        mousePress(canvasView, 100, 100)
        mouseMove(canvasView, 200, 200)
        mouseRelease(canvasView, 200, 200)
        wait(50)
        
        // Get initial scroll position
        var initialContentX = canvasView.contentX
        var initialContentY = canvasView.contentY
        
        // Pan with middle mouse button
        mousePress(canvasView, 400, 300, Qt.MiddleButton)
        mouseMove(canvasView, 300, 200)
        mouseRelease(canvasView, 300, 200, Qt.MiddleButton)
        wait(100)
        
        // Verify canvas panned
        verify(canvasView.contentX !== initialContentX || canvasView.contentY !== initialContentY, 
               "Canvas should have panned")
        compare(canvasView.contentX, initialContentX + 100, "Content should pan by mouse delta X")
        compare(canvasView.contentY, initialContentY + 100, "Content should pan by mouse delta Y")
    }

    function test_coordinateConversion() {
        // Set specific zoom and scroll
        canvasView.zoomLevel = 2.0
        canvasView.contentX = 50
        canvasView.contentY = 100
        wait(50)
        
        // Test viewport to canvas conversion
        var viewportPoint = Qt.point(200, 150)
        var canvasPoint = canvasView.viewportToCanvas(viewportPoint)
        
        // Expected: (200/2 + 50, 150/2 + 100) = (150, 175)
        compare(canvasPoint.x, 150, "Canvas X coordinate should be correct")
        compare(canvasPoint.y, 175, "Canvas Y coordinate should be correct")
        
        // Test canvas to viewport conversion
        var backToViewport = canvasView.canvasToViewport(canvasPoint)
        compare(backToViewport.x, viewportPoint.x, "Round-trip X should match")
        compare(backToViewport.y, viewportPoint.y, "Round-trip Y should match")
    }

    function test_elementCreationModes() {
        // Test Frame creation mode
        canvasController.setActiveType("Frame")
        compare(canvasController.activeType, "Frame", "Active type should be Frame")
        
        mousePress(canvasView, 100, 100)
        mouseMove(canvasView, 200, 200)
        wait(50) // Allow preview to appear
        
        // Preview should be visible during drag
        verify(canvasView.creatingElement, "Should be creating element")
        
        mouseRelease(canvasView, 200, 200)
        wait(50)
        
        // Verify element was created
        compare(elementModel.rowCount(), 1, "One element should be created")
        var element = elementModel.elementAt(0)
        compare(element.type, "Frame", "Element should be a Frame")
        compare(element.x, 100, "Element X should match start position")
        compare(element.y, 100, "Element Y should match start position")
        compare(element.width, 100, "Element width should be 100")
        compare(element.height, 100, "Element height should be 100")
        
        // Test Text creation mode
        canvasController.setActiveType("Text")
        compare(canvasController.activeType, "Text", "Active type should be Text")
        
        mousePress(canvasView, 250, 250)
        mouseMove(canvasView, 350, 300)
        mouseRelease(canvasView, 350, 300)
        wait(50)
        
        // Verify text element was created
        compare(elementModel.rowCount(), 2, "Two elements should exist")
        var textElement = elementModel.elementAt(1)
        compare(textElement.type, "Text", "Second element should be Text")
    }

    function test_gridVisibilityWithZoom() {
        // Grid should not be visible at default zoom
        compare(canvasView.zoomLevel, 1.0, "Default zoom should be 1.0")
        verify(!canvasView.showGrid, "Grid should not be visible at zoom 1.0")
        
        // Zoom in past threshold
        canvasView.zoomLevel = 2.5
        wait(50)
        verify(canvasView.showGrid, "Grid should be visible at zoom 2.5")
        
        // Zoom back out
        canvasView.zoomLevel = 1.5
        wait(50)
        verify(!canvasView.showGrid, "Grid should not be visible at zoom 1.5")
    }

    function test_scrollIndicators() {
        // Create content that extends beyond viewport
        canvasController.setActiveType("Frame")
        
        // Create element at far position
        mousePress(canvasView, 700, 500)
        mouseMove(canvasView, 900, 700)
        mouseRelease(canvasView, 900, 700)
        wait(50)
        
        // Canvas content should be larger than viewport
        verify(canvasView.contentWidth > canvasView.width || 
               canvasView.contentHeight > canvasView.height,
               "Canvas content should extend beyond viewport")
        
        // Scroll indicators should be visible when content is larger
        // (This assumes scroll indicators are implemented)
    }

    function test_escapeKeyBehavior() {
        // Create an element
        canvasController.setActiveType("Frame")
        mousePress(canvasView, 100, 100)
        mouseMove(canvasView, 200, 200)
        mouseRelease(canvasView, 200, 200)
        wait(50)
        
        // Select it
        canvasController.setActiveType("Select")
        mouseClick(canvasView, 150, 150)
        wait(50)
        
        compare(selectionManager.selectedElements.length, 1, "One element should be selected")
        
        // Press Escape to clear selection
        keyPress(Qt.Key_Escape)
        wait(50)
        
        compare(selectionManager.selectedElements.length, 0, "Selection should be cleared")
        
        // Start creating a new element
        canvasController.setActiveType("Frame")
        mousePress(canvasView, 300, 300)
        mouseMove(canvasView, 400, 400)
        wait(50)
        
        verify(canvasView.creatingElement, "Should be creating element")
        
        // Press Escape to cancel creation
        keyPress(Qt.Key_Escape)
        wait(50)
        
        verify(!canvasView.creatingElement, "Element creation should be cancelled")
        
        // Release mouse after cancel
        mouseRelease(canvasView, 400, 400)
        wait(50)
        
        // No new element should be created
        compare(elementModel.rowCount(), 1, "No new element should be created after cancel")
    }

    function test_zoomMaintainsPointUnderCursor() {
        // Create an element as reference
        canvasController.setActiveType("Frame")
        mousePress(canvasView, 200, 200)
        mouseMove(canvasView, 300, 300)
        mouseRelease(canvasView, 300, 300)
        wait(50)
        
        // Get element center in canvas coordinates
        var element = elementModel.elementAt(0)
        var elementCenterCanvas = Qt.point(element.x + element.width/2, 
                                         element.y + element.height/2)
        
        // Convert to viewport coordinates
        var elementCenterViewport = canvasView.canvasToViewport(elementCenterCanvas)
        
        // Zoom in with mouse over element center
        keyPress(Qt.Key_Control)
        wheel(canvasView, elementCenterViewport.x, elementCenterViewport.y, 0, 240) // Two notches up
        keyRelease(Qt.Key_Control)
        wait(100)
        
        // Element center should still be at the same viewport position
        var newElementCenterViewport = canvasView.canvasToViewport(elementCenterCanvas)
        
        fuzzyCompare(newElementCenterViewport.x, elementCenterViewport.x, 1, 
                     "Element center X should maintain viewport position")
        fuzzyCompare(newElementCenterViewport.y, elementCenterViewport.y, 1, 
                     "Element center Y should maintain viewport position")
    }
}