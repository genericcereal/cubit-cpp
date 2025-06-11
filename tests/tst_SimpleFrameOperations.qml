import QtQuick
import QtTest
import Cubit 1.0

TestCase {
    id: testCase
    name: "SimpleFrameOperations"
    
    property var elementModel: null
    property var frame: null

    function initTestCase() {
        // Create element model
        elementModel = Qt.createQmlObject('import Cubit 1.0; ElementModel {}', testCase)
        verify(elementModel !== null, "ElementModel should be created")
    }

    function init() {
        // Clear model before each test
        elementModel.clear()
    }

    function test_createFrame() {
        // Create a frame through the model
        frame = elementModel.createElement("Frame", 100, 100, 200, 150)
        
        // Verify frame was created
        verify(frame !== null, "Frame should be created")
        compare(frame.type, "Frame", "Element type should be Frame")
        compare(frame.x, 100, "Frame x should be 100")
        compare(frame.y, 100, "Frame y should be 100")
        compare(frame.width, 200, "Frame width should be 200")
        compare(frame.height, 150, "Frame height should be 150")
        
        // Verify it was added to the model
        compare(elementModel.rowCount(), 1, "Model should contain one element")
        compare(elementModel.elementAt(0), frame, "Model should contain the created frame")
    }

    function test_modifyFrameProperties() {
        // Create a frame
        frame = elementModel.createElement("Frame", 50, 50, 100, 100)
        
        // Modify properties
        frame.x = 200
        frame.y = 150
        frame.width = 300
        frame.height = 250
        
        // Verify changes
        compare(frame.x, 200, "Frame x should be updated to 200")
        compare(frame.y, 150, "Frame y should be updated to 150")
        compare(frame.width, 300, "Frame width should be updated to 300")
        compare(frame.height, 250, "Frame height should be updated to 250")
    }

    function test_frameSignals() {
        // Create a frame
        frame = elementModel.createElement("Frame", 0, 0, 100, 100)
        
        // Set up signal spy for geometryChanged
        var geometrySpy = createTemporaryObject(signalSpy, testCase, {
            target: frame,
            signalName: "geometryChanged"
        })
        
        // Change position
        frame.x = 50
        compare(geometrySpy.count, 1, "geometryChanged should be emitted once for x change")
        
        // Change size
        frame.width = 200
        compare(geometrySpy.count, 2, "geometryChanged should be emitted for width change")
        
        // Change multiple properties
        frame.setGeometry(100, 100, 150, 150)
        compare(geometrySpy.count, 3, "geometryChanged should be emitted for setGeometry")
    }

    function test_deleteFrame() {
        // Create multiple frames
        var frame1 = elementModel.createElement("Frame", 0, 0, 100, 100)
        var frame2 = elementModel.createElement("Frame", 200, 0, 100, 100)
        var frame3 = elementModel.createElement("Frame", 0, 200, 100, 100)
        
        compare(elementModel.rowCount(), 3, "Model should contain three elements")
        
        // Delete the middle frame
        elementModel.removeElement(frame2)
        
        compare(elementModel.rowCount(), 2, "Model should contain two elements")
        compare(elementModel.elementAt(0), frame1, "First frame should remain")
        compare(elementModel.elementAt(1), frame3, "Third frame should be at index 1")
    }

    Component {
        id: signalSpy
        SignalSpy {}
    }
}