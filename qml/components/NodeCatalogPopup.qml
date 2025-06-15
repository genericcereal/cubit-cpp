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
        var viewportX = (root.position.x - canvas.canvasMinX) * canvas.zoom - canvas.flickable.contentX;
        return viewportX;
    }
    y: {
        // Convert canvas position to viewport position
        var viewportY = (root.position.y - canvas.canvasMinY) * canvas.zoom - canvas.flickable.contentY;
        return viewportY;
    }
    width: 250 * canvas.zoom
    height: 400 * canvas.zoom
    color: "white"
    border.color: "#cccccc"
    border.width: 1 * canvas.zoom
    radius: 8 * canvas.zoom
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
        anchors.margins: 8 * canvas.zoom
        height: 32 * canvas.zoom
        font.pixelSize: 13 * canvas.zoom
        placeholderText: "Search nodes..."
        selectByMouse: true
        
        background: Rectangle {
            color: "#f5f5f5"
            border.color: searchField.activeFocus ? "#2196F3" : "#cccccc"
            border.width: 1 * canvas.zoom
            radius: 4 * canvas.zoom
        }
        
        // Removed auto-focus behavior
        
    }

    // Node list
    ScrollView {
        anchors.top: searchField.bottom
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.bottom: parent.bottom
        anchors.margins: 8 * canvas.zoom
        anchors.topMargin: 4 * canvas.zoom
        clip: true
        

        ListView {
            id: nodeListView
            model: filteredModel
            
            property var filteredModel: {
                if (!root.nodeCatalog || !root.nodeCatalog.catalog) {
                    console.log("Warning: nodeCatalog or catalog is not available")
                    return []
                }
                
                var allTypes = Object.keys(root.nodeCatalog.catalog)
                var searchText = searchField.text
                
                if (!searchText || searchText === "") {
                    return allTypes
                }
                
                // Filter based on search text (case insensitive)
                var searchLower = searchText.toLowerCase()
                return allTypes.filter(function(type) {
                    var nodeType = root.nodeCatalog.catalog[type]
                    return nodeType.name.toLowerCase().indexOf(searchLower) !== -1
                })
            }
            spacing: 4 * canvas.zoom

            delegate: Rectangle {
                width: nodeListView.width
                height: 36 * canvas.zoom
                color: mouseArea.containsMouse ? "#f0f0f0" : "transparent"
                radius: 4 * canvas.zoom

                property var nodeType: root.nodeCatalog.catalog[modelData]
                
                Component.onCompleted: {
                    if (!nodeType) {
                        console.log("Warning: nodeType is undefined for", modelData)
                    }
                }

                MouseArea {
                    id: mouseArea
                    anchors.fill: parent
                    hoverEnabled: true

                    onClicked: {
                        console.log("Selected node type:", modelData);

                        // Create the node at the catalog position
                        var nodeData = root.nodeCatalog.createNodeData(modelData, root.position.x, root.position.y);

                        if (nodeData) {
                            // Create node from JSON
                            var jsonData = JSON.stringify(nodeData);
                            var newNodeId = root.canvas.controller.createNodeFromJson(jsonData);

                            if (newNodeId && root.dragSourceNode) {
                                // Create edge from source to new node
                                console.log("Creating edge from", root.dragSourceNode.nodeTitle, "to new node");

                                // Determine target port based on source type
                                var targetPortIndex = 0;
                                // Check if source is a data type (not Flow)
                                if (root.dragSourcePortType !== "Flow") {
                                    // Find first matching data type input port
                                    var targets = nodeType.targets || [];
                                    for (var i = 0; i < targets.length; i++) {
                                        // Match exact type or find first non-Flow port
                                        if (targets[i].type === root.dragSourcePortType || 
                                            (targets[i].type !== "Flow" && root.dragSourcePortType !== "Flow")) {
                                            targetPortIndex = i;
                                            break;
                                        }
                                    }
                                }

                                root.canvas.controller.createEdge(root.dragSourceNode.elementId, newNodeId, root.dragSourceHandleType, "left", root.dragSourcePortIndex, targetPortIndex);
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
                    anchors.leftMargin: 12 * canvas.zoom
                    text: nodeType.name
                    font.pixelSize: 13 * canvas.zoom
                    color: "#333333"
                }
            }
        }
    }
}
