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