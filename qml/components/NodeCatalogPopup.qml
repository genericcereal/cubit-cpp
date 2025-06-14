import QtQuick
import QtQuick.Controls
import "."

Rectangle {
    id: root

    // Required properties
    required property var canvas
    required property var nodeCatalog
    required property bool showCatalog
    required property point position
    required property var dragSourceNode
    required property string dragSourceHandleType
    required property int dragSourcePortIndex
    required property string dragSourcePortType

    // Signals
    signal nodeSelected(string nodeType)
    signal dismissed

    // Appearance
    visible: root.showCatalog
    
    // MouseArea to catch clicks on the popup itself
    MouseArea {
        anchors.fill: parent
        // Prevent clicks from going through to canvas
        onClicked: {
            // Do nothing - just consume the click
        }
    }
    x: {
        // Convert canvas position to viewport position
        var viewportX = (root.position.x - canvas.canvasMinX) * canvas.zoomLevel - canvas.flickable.contentX;
        return viewportX;
    }
    y: {
        // Convert canvas position to viewport position
        var viewportY = (root.position.y - canvas.canvasMinY) * canvas.zoomLevel - canvas.flickable.contentY;
        return viewportY;
    }
    width: 250 * canvas.zoomLevel
    height: 400 * canvas.zoomLevel
    color: "white"
    border.color: "#cccccc"
    border.width: 1 * canvas.zoomLevel
    radius: 8 * canvas.zoomLevel
    z: 1000  // Ensure it's on top
    transformOrigin: Item.TopLeft

    onVisibleChanged: {
        console.log("Node catalog visible changed to:", visible);
        if (visible) {
            console.log("  Position:", x, y);
            console.log("  Size:", width, "x", height);
            console.log("  Parent:", parent);
            console.log("  Z:", z);
            
            // Clear previous search but don't focus
            searchField.text = ""
        } else {
            // When hiding, remove focus from search field
            searchField.focus = false
            if (searchField.activeFocus) {
                searchField.parent.forceActiveFocus()
            }
        }
    }

    
    // Search input
    TextField {
        id: searchField
        anchors.top: parent.top
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.margins: 8 * canvas.zoomLevel
        height: 32 * canvas.zoomLevel
        font.pixelSize: 13 * canvas.zoomLevel
        placeholderText: "Search nodes..."
        selectByMouse: true
        
        background: Rectangle {
            color: "#f5f5f5"
            border.color: searchField.activeFocus ? "#2196F3" : "#cccccc"
            border.width: 1 * canvas.zoomLevel
            radius: 4 * canvas.zoomLevel
        }
        
        // Removed auto-focus behavior
        
    }

    // Node list
    ScrollView {
        anchors.top: searchField.bottom
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.bottom: parent.bottom
        anchors.margins: 8 * canvas.zoomLevel
        anchors.topMargin: 4 * canvas.zoomLevel
        clip: true
        

        ListView {
            id: nodeListView
            model: filteredModel
            
            property var filteredModel: {
                var allTypes = Object.keys(nodeCatalog.catalog)
                var searchText = searchField.text
                
                if (!searchText || searchText === "") {
                    return allTypes
                }
                
                // Filter based on search text (case insensitive)
                var searchLower = searchText.toLowerCase()
                return allTypes.filter(function(type) {
                    var nodeType = nodeCatalog.catalog[type]
                    return nodeType.name.toLowerCase().indexOf(searchLower) !== -1
                })
            }
            spacing: 4 * canvas.zoomLevel

            delegate: Rectangle {
                width: nodeListView.width
                height: 36 * canvas.zoomLevel
                color: mouseArea.containsMouse ? "#f0f0f0" : "transparent"
                radius: 4 * canvas.zoomLevel

                property var nodeType: nodeCatalog.catalog[modelData]

                MouseArea {
                    id: mouseArea
                    anchors.fill: parent
                    hoverEnabled: true

                    onClicked: {
                        console.log("Selected node type:", modelData);

                        // Create the node at the catalog position
                        var nodeData = nodeCatalog.createNodeData(modelData, root.position.x, root.position.y);

                        if (nodeData) {
                            // Create node from JSON
                            var jsonData = JSON.stringify(nodeData);
                            var newNodeId = canvas.controller.createNodeFromJson(jsonData);

                            if (newNodeId && root.dragSourceNode) {
                                // Create edge from source to new node
                                console.log("Creating edge from", root.dragSourceNode.nodeTitle, "to new node");

                                // Determine target port based on source type
                                var targetPortIndex = 0;
                                if (root.dragSourcePortType === "Variable") {
                                    // Find first variable input port
                                    var targets = nodeType.targets || [];
                                    for (var i = 0; i < targets.length; i++) {
                                        if (targets[i].type === "Variable") {
                                            targetPortIndex = i;
                                            break;
                                        }
                                    }
                                }

                                canvas.controller.createEdge(root.dragSourceNode.elementId, newNodeId, root.dragSourceHandleType, "left", root.dragSourcePortIndex, targetPortIndex);
                            }
                        }

                        // Emit signal to clear catalog and drag state
                        root.nodeSelected(modelData);
                    }
                }

                // Node name
                Text {
                    anchors.verticalCenter: parent.verticalCenter
                    anchors.left: parent.left
                    anchors.leftMargin: 12 * canvas.zoomLevel
                    text: nodeType.name
                    font.pixelSize: 13 * canvas.zoomLevel
                    color: "#333333"
                }
            }
        }
    }
}
