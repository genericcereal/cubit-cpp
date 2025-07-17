import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import Cubit

Item {
    id: root
    
    property var elementModel
    property var selectionManager
    
    ColumnLayout {
        anchors.fill: parent
        anchors.margins: 0
        spacing: 0
        
        // Tab bar at the top
        TabBar {
            id: tabBar
            Layout.fillWidth: true
            Layout.preferredHeight: 40
            
            TabButton {
                text: "Elements"
                font.pixelSize: 14
            }
            
            TabButton {
                text: "Components"
                font.pixelSize: 14
            }
        }
        
        // StackLayout to switch between Elements and Components views
        StackLayout {
            Layout.fillWidth: true
            Layout.fillHeight: true
            currentIndex: tabBar.currentIndex
            
            // Elements tab content
            Item {
                // Create filtered proxy model for non-component elements
                ElementFilterProxy {
                    id: elementsFilteredModel
                    sourceModel: root.elementModel
                    viewMode: Application.activeCanvas ? Application.activeCanvas.viewMode : "design"
                    editingElement: Application.activeCanvas ? Application.activeCanvas.editingElement : null
                    filterComponentsOut: true
                }
                
                // Blue divider for drop indication
                Rectangle {
                    id: elementsDropIndicator
                    width: parent.width
                    height: 2
                    color: "#2196F3"
                    visible: false
                    z: 1000
                }
                
                ScrollView {
                    anchors.fill: parent
                    
                    ListView {
                        id: elementsListView
                        model: elementsFilteredModel
                        spacing: 1
                        
                        delegate: Rectangle {
                            id: elementsDelegateRect
                            width: elementsListView.width
                            height: visible ? 28 : 0
                            color: model.selected ? "#e3f2fd" : (elementsMouseArea.containsMouse ? "#f5f5f5" : "#ffffff")
                            antialiasing: true
                            
                            // Visual states for dragging
                            states: [
                                State {
                                    name: "dragging"
                                    // No visual changes - the drag ghost shows what's being dragged
                                }
                            ]
                            
                            // Drop properties for frame parenting
                            property bool isDropTarget: false
                            property bool isFrame: model.elementType === "Frame"
                            border.width: isDropTarget && isFrame ? 2 : 0
                            border.color: "#2196F3"
                            
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
                                    for (var k = 0; k < elementsListView.count; k++) {
                                        var item = elementsListView.itemAtIndex(k)
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
                                id: elementsExpandBox
                                x: 6 + (indentLevel * 16)
                                y: 7
                                width: 14
                                height: 14
                                visible: hasChildren
                                color: elementsExpandMouseArea.containsMouse ? "#e0e0e0" : "#f5f5f5"
                                border.width: 1
                                border.color: "#cccccc"
                                radius: 2
                                z: 1  // Above the main mouse area
                                
                                Text {
                                    anchors.centerIn: parent
                                    text: elementsDelegateRect.isExpanded ? "−" : "+"
                                    font.pixelSize: 10
                                    font.weight: Font.Medium
                                    color: "#444444"
                                }
                                
                                MouseArea {
                                    id: elementsExpandMouseArea
                                    anchors.fill: parent
                                    hoverEnabled: true
                                    onClicked: {
                                        elementsDelegateRect.isExpanded = !elementsDelegateRect.isExpanded
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
                                id: elementsMouseArea
                                anchors.fill: parent
                                hoverEnabled: true
                                preventStealing: true
                                
                                property bool isDragging: false
                                property point dragStartPos
                                property var dragItem: null
                                
                                onPressed: (mouse) => {
                                    dragStartPos = Qt.point(mouse.x, mouse.y)
                                    isDragging = false
                                    mouse.accepted = true
                                }
                                
                                onPositionChanged: (mouse) => {
                                    if (pressed) {
                                        if (!isDragging && Math.abs(mouse.y - dragStartPos.y) > 5) {
                                            isDragging = true
                                            elementsDelegateRect.state = "dragging"
                                            
                                            // Get the main window reference
                                            var mainWindow = root.Window.window
                                            
                                            if (mainWindow && mainWindow.dragOverlay) {
                                                // Set up the drag overlay
                                                mainWindow.dragOverlay.draggedElementName = model.name || "Unnamed"
                                                mainWindow.dragOverlay.draggedElementType = model.elementType
                                                mainWindow.dragOverlay.draggedElement = model.element
                                                mainWindow.dragOverlay.visible = true
                                                
                                                // Position the drag ghost at cursor
                                                var globalPos = elementsMouseArea.mapToItem(mainWindow.contentItem, mouse.x, mouse.y)
                                                mainWindow.dragOverlay.ghostPosition = globalPos
                                            }
                                        }
                                        
                                        if (isDragging) {
                                            // Update drag overlay position
                                            var mainWindow = root.Window.window
                                            if (mainWindow && mainWindow.dragOverlay && mainWindow.dragOverlay.visible) {
                                                var globalPos = elementsMouseArea.mapToItem(mainWindow.contentItem, mouse.x, mouse.y)
                                                mainWindow.dragOverlay.ghostPosition = globalPos
                                            }
                                            
                                            // Calculate drop position
                                            var listPos = elementsListView.mapFromItem(elementsMouseArea, mouse.x, mouse.y)
                                            var dropY = listPos.y
                                            var targetIndex = -1
                                            var indicatorY = 0
                                            var dropTargetItem = null
                                            
                                            // Clear all drop targets
                                            for (var j = 0; j < elementsListView.count; j++) {
                                                var clearItem = elementsListView.itemAtIndex(j)
                                                if (clearItem && clearItem !== elementsDelegateRect) {
                                                    clearItem.isDropTarget = false
                                                }
                                            }
                                            
                                            // Find the target position or drop target
                                            for (var i = 0; i < elementsListView.count; i++) {
                                                var item = elementsListView.itemAtIndex(i)
                                                if (item && item.visible && item !== elementsDelegateRect) {
                                                    var itemPos = elementsListView.mapFromItem(item, 0, 0)
                                                    // Check if hovering over a frame
                                                    if (dropY >= itemPos.y && dropY <= itemPos.y + item.height && item.isFrame) {
                                                        dropTargetItem = item
                                                        item.isDropTarget = true
                                                        elementsDropIndicator.visible = false
                                                        break
                                                    } else if (dropY < itemPos.y + 5) {
                                                        targetIndex = i
                                                        indicatorY = itemPos.y
                                                        break
                                                    } else if (dropY < itemPos.y + item.height) {
                                                        targetIndex = i + 1
                                                        indicatorY = itemPos.y + item.height
                                                        break
                                                    }
                                                }
                                            }
                                            
                                            if (!dropTargetItem && targetIndex >= 0) {
                                                elementsDropIndicator.y = indicatorY
                                                elementsDropIndicator.visible = true
                                            } else if (!dropTargetItem) {
                                                // Check if we're below the last element
                                                var lastVisibleIndex = -1
                                                var lastVisibleItemBottom = 0
                                                
                                                // Find the last visible element
                                                for (var k = elementsListView.count - 1; k >= 0; k--) {
                                                    var lastItem = elementsListView.itemAtIndex(k)
                                                    if (lastItem && lastItem.visible && lastItem !== elementsDelegateRect) {
                                                        lastVisibleIndex = k
                                                        var lastItemPos = elementsListView.mapFromItem(lastItem, 0, 0)
                                                        lastVisibleItemBottom = lastItemPos.y + lastItem.height
                                                        break
                                                    }
                                                }
                                                
                                                // If we're below the last element, show indicator there
                                                if (lastVisibleIndex >= 0 && dropY > lastVisibleItemBottom - 5) {
                                                    elementsDropIndicator.y = lastVisibleItemBottom
                                                    elementsDropIndicator.visible = true
                                                } else {
                                                    elementsDropIndicator.visible = false
                                                }
                                            }
                                        }
                                    }
                                }
                                
                                onReleased: {
                                    if (isDragging) {
                                        elementsDropIndicator.visible = false
                                        
                                        // Reset visual state
                                        elementsDelegateRect.state = ""
                                        
                                        // Hide drag overlay
                                        var mainWindow = root.Window.window
                                        if (mainWindow && mainWindow.dragOverlay) {
                                            mainWindow.dragOverlay.visible = false
                                            mainWindow.dragOverlay.draggedElement = null
                                        }
                                        
                                        // Clear all drop targets
                                        for (var j = 0; j < elementsListView.count; j++) {
                                            var clearItem = elementsListView.itemAtIndex(j)
                                            if (clearItem) {
                                                clearItem.isDropTarget = false
                                            }
                                        }
                                        
                                        // Calculate final drop position based on mouse position
                                        var listPos = elementsListView.mapFromItem(elementsMouseArea, mouseX, mouseY)
                                        var dropY = listPos.y
                                        var targetIndex = -1
                                        var dropTargetFrame = null
                                        var lastVisibleIndex = -1
                                        var lastVisibleItemBottom = 0
                                        
                                        // First, find the last visible element
                                        for (var j = elementsListView.count - 1; j >= 0; j--) {
                                            var lastItem = elementsListView.itemAtIndex(j)
                                            if (lastItem && lastItem.visible && lastItem !== elementsDelegateRect) {
                                                lastVisibleIndex = j
                                                var lastItemPos = elementsListView.mapFromItem(lastItem, 0, 0)
                                                lastVisibleItemBottom = lastItemPos.y + lastItem.height
                                                break
                                            }
                                        }
                                        
                                        for (var i = 0; i < elementsListView.count; i++) {
                                            var item = elementsListView.itemAtIndex(i)
                                            if (item && item.visible && item !== elementsDelegateRect) {
                                                var itemPos = elementsListView.mapFromItem(item, 0, 0)
                                                // Check if dropping onto a frame
                                                if (dropY >= itemPos.y && dropY <= itemPos.y + item.height && item.isFrame) {
                                                    // Get the frame element
                                                    var frameElement = elementsFilteredModel.elementAt(i)
                                                    if (frameElement && frameElement.elementType === "Frame") {
                                                        dropTargetFrame = frameElement
                                                        break
                                                    }
                                                } else if (dropY < itemPos.y + 5) {
                                                    targetIndex = i
                                                    break
                                                } else if (dropY < itemPos.y + item.height) {
                                                    targetIndex = i + 1
                                                    break
                                                }
                                            }
                                        }
                                        
                                        // Check if we're dropping below the last element
                                        if (targetIndex < 0 && !dropTargetFrame && lastVisibleIndex >= 0 && dropY > lastVisibleItemBottom - 5) {
                                            targetIndex = elementsFilteredModel.rowCount()
                                        }
                                        
                                        if (model.element) {
                                            var sourceElement = elementsFilteredModel.elementAt(model.index)
                                            if (sourceElement) {
                                                if (dropTargetFrame) {
                                                    // Parent the element to the frame
                                                    sourceElement.parentId = dropTargetFrame.elementId
                                                } else if (targetIndex >= 0) {
                                                    // Unparent the element if it was parented
                                                    if (sourceElement.parentId) {
                                                        sourceElement.parentId = ""
                                                    }
                                                    // Reorder the element
                                                    var sourceTargetIndex = targetIndex
                                                    if (targetIndex < elementsFilteredModel.rowCount()) {
                                                        var targetElement = elementsFilteredModel.elementAt(targetIndex)
                                                        if (targetElement) {
                                                            // Find the target element's index in the source model
                                                            for (var k = 0; k < root.elementModel.rowCount(); k++) {
                                                                if (root.elementModel.elementAt(k) === targetElement) {
                                                                    sourceTargetIndex = k
                                                                    break
                                                                }
                                                            }
                                                        } else {
                                                            // If dropping at the end, use the source model's row count
                                                            sourceTargetIndex = root.elementModel.rowCount()
                                                        }
                                                    }
                                                    root.elementModel.reorderElement(sourceElement, sourceTargetIndex)
                                                }
                                            }
                                        }
                                        isDragging = false
                                    } else {
                                        // Regular click
                                        if (root.selectionManager) {
                                            root.selectionManager.selectOnly(model.element)
                                        }
                                    }
                                }
                                
                                onCanceled: {
                                    elementsDropIndicator.visible = false
                                    isDragging = false
                                    
                                    // Reset visual state
                                    elementsDelegateRect.state = ""
                                    
                                    // Hide drag overlay
                                    var mainWindow = root.Window.window
                                    if (mainWindow && mainWindow.dragOverlay) {
                                        mainWindow.dragOverlay.visible = false
                                        mainWindow.dragOverlay.draggedElement = null
                                    }
                                    
                                    // Clear all drop targets
                                    for (var j = 0; j < elementsListView.count; j++) {
                                        var clearItem = elementsListView.itemAtIndex(j)
                                        if (clearItem) {
                                            clearItem.isDropTarget = false
                                        }
                                    }
                                }
                            }
                        }
                    }
                }
            }
            
            // Components tab content
            Item {
                // Create filtered proxy model for components only
                ElementFilterProxy {
                    id: componentsFilteredModel
                    sourceModel: root.elementModel
                    viewMode: Application.activeCanvas ? Application.activeCanvas.viewMode : "design"
                    editingElement: Application.activeCanvas ? Application.activeCanvas.editingElement : null
                    filterComponentsOnly: true
                }
                
                ScrollView {
                    anchors.fill: parent
                    
                    ListView {
                        id: componentsListView
                        model: componentsFilteredModel
                        spacing: 1
                        
                        delegate: Rectangle {
                            id: componentsDelegateRect
                            width: componentsListView.width
                            height: visible ? 28 : 0
                            color: model.selected ? "#e3f2fd" : (componentsMouseArea.containsMouse ? "#f5f5f5" : "#ffffff")
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
                                    for (var k = 0; k < componentsListView.count; k++) {
                                        var item = componentsListView.itemAtIndex(k)
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
                                id: componentsExpandBox
                                x: 6 + (indentLevel * 16)
                                y: 7
                                width: 14
                                height: 14
                                visible: hasChildren
                                color: componentsExpandMouseArea.containsMouse ? "#e0e0e0" : "#f5f5f5"
                                border.width: 1
                                border.color: "#cccccc"
                                radius: 2
                                z: 1  // Above the main mouse area
                                
                                Text {
                                    anchors.centerIn: parent
                                    text: componentsDelegateRect.isExpanded ? "−" : "+"
                                    font.pixelSize: 10
                                    font.weight: Font.Medium
                                    color: "#444444"
                                }
                                
                                MouseArea {
                                    id: componentsExpandMouseArea
                                    anchors.fill: parent
                                    hoverEnabled: true
                                    onClicked: {
                                        componentsDelegateRect.isExpanded = !componentsDelegateRect.isExpanded
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
                                id: componentsMouseArea
                                anchors.fill: parent
                                hoverEnabled: true
                                
                                onClicked: {
                                    if (root.selectionManager) {
                                        root.selectionManager.selectOnly(model.element)
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }
    }
}