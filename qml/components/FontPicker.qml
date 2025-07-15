import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

Item {
    id: root
    
    implicitHeight: 400
    implicitWidth: 230
    
    property string currentFontFamily: ""
    property string searchText: ""
    
    signal fontSelected(string fontFamily)
    
    function clearSearch() {
        searchField.text = ""
        searchText = ""
    }
    
    ColumnLayout {
        anchors.fill: parent
        spacing: 10
        
        // Search field
        TextField {
            id: searchField
            Layout.fillWidth: true
            placeholderText: "Search fonts..."
            onTextChanged: root.searchText = text.toLowerCase()
        }
        
        // Font list
        ScrollView {
            Layout.fillWidth: true
            Layout.fillHeight: true
            
            ListView {
                id: fontListView
                model: filteredFontModel
                clip: true
                spacing: 2
                
                delegate: Rectangle {
                    width: fontListView.width
                    height: 30
                    color: {
                        if (mouseArea.containsMouse) return "#e0e0e0"
                        if (model.display === root.currentFontFamily) return "#d0d0ff"
                        return "transparent"
                    }
                    
                    Text {
                        anchors.left: parent.left
                        anchors.leftMargin: 10
                        anchors.verticalCenter: parent.verticalCenter
                        text: model.display
                        font.family: model.display
                        font.pixelSize: 14
                        elide: Text.ElideRight
                        width: parent.width - 20
                    }
                    
                    MouseArea {
                        id: mouseArea
                        anchors.fill: parent
                        hoverEnabled: true
                        cursorShape: Qt.PointingHandCursor
                        onClicked: {
                            root.currentFontFamily = model.display
                            root.fontSelected(model.display)
                        }
                    }
                }
            }
        }
    }
    
    // Filtered font model
    ListModel {
        id: filteredFontModel
    }
    
    // Update filtered model when search text changes
    function updateFilteredModel() {
        filteredFontModel.clear()
        var fonts = Qt.fontFamilies()
        
        for (var i = 0; i < fonts.length; i++) {
            if (searchText === "" || fonts[i].toLowerCase().indexOf(searchText) !== -1) {
                filteredFontModel.append({"display": fonts[i]})
            }
        }
    }
    
    Component.onCompleted: {
        updateFilteredModel()
    }
    
    onSearchTextChanged: {
        updateFilteredModel()
    }
    
    // Clear search when component becomes visible
    onVisibleChanged: {
        if (visible) {
            clearSearch()
        }
    }
}