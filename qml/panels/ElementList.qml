import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import Cubit

Item {
    id: root
    
    property var elementModel
    property var selectionManager
    
    // Create filtered proxy model
    ElementFilterProxy {
        id: filteredModel
        sourceModel: root.elementModel
        viewMode: Application.activeCanvas ? Application.activeCanvas.viewMode : "design"
        editingElement: Application.activeCanvas ? Application.activeCanvas.editingElement : null
    }
    
    ScrollView {
        id: scrollView
        anchors.fill: parent
        
        ListView {
            id: listView
            model: filteredModel
            spacing: 1
        
        delegate: Rectangle {
            id: delegateRect
            width: listView.width
            height: visible ? 28 : 0
            color: model.selected ? "#e3f2fd" : (mouseArea.containsMouse ? "#f5f5f5" : "#ffffff")
            antialiasing: true
            
            // Calculate indent level and check if has children
            property int indentLevel: {
                var level = 0
                var currentElement = model.element
                while (currentElement && currentElement.parentId) {
                    level++
                    // Find parent element
                    var parentFound = false
                    for (var i = 0; i < root.elementModel.rowCount(); i++) {
                        var elem = root.elementModel.elementAt(i)
                        if (elem && elem.elementId === currentElement.parentId) {
                            currentElement = elem
                            parentFound = true
                            break
                        }
                    }
                    if (!parentFound) break
                }
                return level
            }
            
            property bool hasChildren: {
                if (!model.element) return false
                for (var i = 0; i < root.elementModel.rowCount(); i++) {
                    var child = root.elementModel.elementAt(i)
                    if (child && child.parentId === model.element.elementId) {
                        return true
                    }
                }
                return false
            }
            
            property bool isExpanded: true
            
            // Check if this item should be visible based on parent expansion state
            property bool parentExpanded: {
                if (!model.element || !model.element.parentId) return true
                
                // Find all ancestors and check if they are expanded
                var ancestors = []
                var currentElement = model.element
                while (currentElement && currentElement.parentId) {
                    // Find parent element
                    var parentFound = false
                    for (var i = 0; i < root.elementModel.rowCount(); i++) {
                        var elem = root.elementModel.elementAt(i)
                        if (elem && elem.elementId === currentElement.parentId) {
                            ancestors.push(elem.elementId)
                            currentElement = elem
                            parentFound = true
                            break
                        }
                    }
                    if (!parentFound) break
                }
                
                // Check if all ancestors are expanded
                for (var j = 0; j < ancestors.length; j++) {
                    var ancestorId = ancestors[j]
                    // Find the delegate for this ancestor
                    for (var k = 0; k < listView.count; k++) {
                        var item = listView.itemAtIndex(k)
                        if (item && item.elementId === ancestorId && !item.isExpanded) {
                            return false
                        }
                    }
                }
                return true
            }
            
            property string elementId: model.element ? model.element.elementId : ""
            
            visible: parentExpanded && (model.element ? model.element.showInElementList : true)
            
            // Expand/collapse box positioned absolutely
            Rectangle {
                id: expandBox
                x: 6 + (indentLevel * 16)
                y: 7
                width: 14
                height: 14
                visible: hasChildren
                color: expandMouseArea.containsMouse ? "#e0e0e0" : "#f5f5f5"
                border.width: 1
                border.color: "#cccccc"
                radius: 2
                z: 1  // Above the main mouse area
                
                Text {
                    anchors.centerIn: parent
                    text: delegateRect.isExpanded ? "−" : "+"
                    font.pixelSize: 10
                    font.weight: Font.Medium
                    color: "#444444"
                }
                
                MouseArea {
                    id: expandMouseArea
                    anchors.fill: parent
                    hoverEnabled: true
                    onClicked: {
                        delegateRect.isExpanded = !delegateRect.isExpanded
                        treeLinesOverlay.updateExpandedState(delegateRect.elementId, delegateRect.isExpanded)
                    }
                }
            }
            
            
            RowLayout {
                anchors.fill: parent
                anchors.leftMargin: 6 + (indentLevel * 16) + 18  // Add space for expand box
                anchors.rightMargin: 6
                anchors.topMargin: 4
                anchors.bottomMargin: 4
                spacing: 4
                
                Label {
                    Layout.fillWidth: true
                    text: model.name || "Unnamed"
                    elide: Text.ElideRight
                    font.pixelSize: 12
                }
                
                Label {
                    Layout.alignment: Qt.AlignRight
                    text: model.elementType
                    color: "#666666"
                    font.pixelSize: 10
                }
            }
            
            MouseArea {
                id: mouseArea
                anchors.fill: parent
                hoverEnabled: true
                drag.target: dragItem
                
                onClicked: {
                    if (selectionManager) {
                        selectionManager.selectOnly(model.element)
                    }
                }
                
                onPressed: (mouse) => {
                    dragItem.elementType = model.elementType
                    dragItem.elementName = model.name
                    dragItem.elementId = model.element.elementId
                    // Ensure drag item starts at the correct position
                    var globalPos = mapToItem(dragItem.parent, mouse.x, mouse.y)
                    dragItem.x = globalPos.x - dragItem.width / 2
                    dragItem.y = globalPos.y - dragItem.height / 2
                }
                
                onReleased: {
                    if (drag.active) {
                        dragItem.Drag.drop()
                    }
                    // Reset position after drag
                    dragItem.x = 0
                    dragItem.y = 0
                }
            }
            
            // Drag preview that looks like a node
            Rectangle {
                id: dragItem
                width: 200
                height: 100
                visible: mouseArea.drag.active
                opacity: 0.8
                parent: root.parent // Parent to the root's parent to allow movement outside list bounds
                
                property string elementType: ""
                property string elementName: ""
                property string elementId: ""
                
                Drag.active: mouseArea.drag.active
                Drag.hotSpot.x: width / 2
                Drag.hotSpot.y: height / 2
                
                // Node-like appearance
                color: "#f5f5f5"
                border.color: "#ddd"
                border.width: 1
                radius: 4
                
                // Header
                Rectangle {
                    id: header
                    anchors.top: parent.top
                    anchors.left: parent.left
                    anchors.right: parent.right
                    height: 30
                    color: "#9C27B0"  // Purple for Variable nodes (all elements create Variable nodes)
                    radius: 4
                    
                    Rectangle {
                        anchors.bottom: parent.bottom
                        anchors.left: parent.left
                        anchors.right: parent.right
                        height: 4
                        color: parent.color
                    }
                    
                    Label {
                        anchors.centerIn: parent
                        text: dragItem.elementName  // All elements create Variable nodes
                        color: "white"
                        font.pixelSize: 12
                    }
                }
            }
        }
        }
    }
    
    // Tree lines overlay - draws vertical lines connecting parents to children
    Canvas {
        id: treeLinesOverlay
        anchors.fill: scrollView
        z: 1  // Above scrollview content
        enabled: false  // Don't intercept mouse events
        
        property var expandedStates: ({})
        
        function updateExpandedState(elementId, isExpanded) {
            expandedStates[elementId] = isExpanded
            requestPaint()
        }
            
        function repaintLines() {
            requestPaint()
        }
            
            onPaint: {
                var ctx = getContext("2d")
                if (!ctx) return
                
                ctx.clearRect(0, 0, width, height)
                
                // Set line style
                ctx.strokeStyle = "#999999"  // Medium gray
                ctx.lineWidth = 1
                ctx.setLineDash([2, 2])  // Dotted line
                
                // Simple approach: draw vertical lines based on parent-child relationships
                var itemHeight = 29  // 28px height + 1px spacing
                var visibleIndex = 0
                var itemPositions = {}
                
                // First pass: record positions of all visible items
                for (var i = 0; i < filteredModel.rowCount(); i++) {
                    var element = filteredModel.elementAt(i)
                    if (!element) continue
                    
                    // Skip elements that shouldn't be shown in the list
                    if (!element.showInElementList) continue
                    
                    // Record the item position
                    itemPositions[element.elementId] = {
                        index: visibleIndex,
                        element: element,
                        y: visibleIndex * itemHeight
                    }
                    visibleIndex++
                }
                
                // Second pass: draw vertical lines for parents with children
                for (var elementId in itemPositions) {
                    var parentInfo = itemPositions[elementId]
                    var childrenYPositions = []
                    
                    // Find all children of this element
                    for (var childId in itemPositions) {
                        var childInfo = itemPositions[childId]
                        if (childInfo.element.parentId === elementId) {
                            childrenYPositions.push(childInfo.y + itemHeight - 1)  // Bottom of child row
                        }
                    }
                    
                    // If has children AND is expanded, draw vertical line
                    if (childrenYPositions.length > 0 && treeLinesOverlay.expandedStates[elementId] !== false) {
                        // Calculate indent level
                        var indentLevel = 0
                        var currentElement = parentInfo.element
                        while (currentElement && currentElement.parentId) {
                            indentLevel++
                            currentElement = itemPositions[currentElement.parentId] ? 
                                            itemPositions[currentElement.parentId].element : null
                        }
                        
                        var x = 6 + (indentLevel * 16) + 7  // Center of expand box
                        var startY = parentInfo.y + 21  // Below expand box
                        var endY = Math.max.apply(null, childrenYPositions)
                        
                        ctx.beginPath()
                        ctx.moveTo(x, startY)
                        ctx.lineTo(x, endY)
                        ctx.stroke()
                    }
                }
            }
            
            // Repaint when model changes
            Connections {
                target: filteredModel
                function onDataChanged() { 
                    treeLinesOverlay.repaintLines()
                }
                function onRowsInserted() { 
                    treeLinesOverlay.repaintLines()
                }
                function onRowsRemoved() { 
                    treeLinesOverlay.repaintLines()
                }
            }
            
        // Initial paint
        Component.onCompleted: {
            // Initialize all elements as expanded by default
            for (var i = 0; i < filteredModel.rowCount(); i++) {
                var element = filteredModel.elementAt(i)
                if (element) {
                    expandedStates[element.elementId] = true
                }
            }
            repaintLines()
        }
    }
}