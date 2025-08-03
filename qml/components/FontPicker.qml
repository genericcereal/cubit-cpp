import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import Cubit

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
        
        // Loading indicator
        BusyIndicator {
            Layout.alignment: Qt.AlignHCenter
            running: GoogleFonts.isLoading
            visible: running
        }
        
        // Font list
        ScrollView {
            Layout.fillWidth: true
            Layout.fillHeight: true
            visible: !GoogleFonts.isLoading
            
            ListView {
                id: fontListView
                model: filteredFontModel
                clip: true
                spacing: 2
                
                delegate: Rectangle {
                    width: fontListView.width
                    height: 35
                    color: {
                        if (mouseArea.containsMouse) return "#e0e0e0"
                        if (model.fontFamily === root.currentFontFamily) return "#d0d0ff"
                        return "transparent"
                    }
                    
                    // Font loader for preview
                    GoogleFontLoader {
                        id: previewFontLoader
                        fontFamily: model.fontFamily
                    }
                    
                    Text {
                        anchors.left: parent.left
                        anchors.leftMargin: 10
                        anchors.right: parent.right
                        anchors.rightMargin: 10
                        anchors.verticalCenter: parent.verticalCenter
                        text: model.fontFamily
                        font.family: previewFontLoader.isLoaded ? model.fontFamily : "Arial"
                        font.pixelSize: 16
                        elide: Text.ElideRight
                    }
                    
                    MouseArea {
                        id: mouseArea
                        anchors.fill: parent
                        hoverEnabled: true
                        cursorShape: Qt.PointingHandCursor
                        onClicked: {
                            root.currentFontFamily = model.fontFamily
                            root.fontSelected(model.fontFamily)
                        }
                    }
                    
                    // Load font when it becomes visible in the viewport
                    property bool isInViewport: {
                        if (!parent) return false
                        var delegateY = (parent.y || 0) + y
                        var viewportTop = fontListView.contentY
                        var viewportBottom = viewportTop + fontListView.height
                        return delegateY + height >= viewportTop && delegateY <= viewportBottom
                    }
                    
                    onIsInViewportChanged: {
                        if (isInViewport && !previewFontLoader.isLoaded && GoogleFonts && GoogleFonts.isFontLoaded && !GoogleFonts.isFontLoaded(model.fontFamily, "regular")) {
                            // Load regular weight for preview
                            GoogleFonts.loadFont(model.fontFamily, "regular")
                        }
                    }
                    
                    Connections {
                        target: mouseArea
                        function onClicked() {
                            // When selected, load all available weights
                            GoogleFonts.loadFontWithAllWeights(model.fontFamily)
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
    
    // Track loaded fonts
    Connections {
        target: GoogleFonts
        function onFontLoaded(fontFamily) {
            // Update the model to mark font as loaded
            for (var i = 0; i < filteredFontModel.count; i++) {
                if (filteredFontModel.get(i).fontFamily === fontFamily) {
                    filteredFontModel.setProperty(i, "isLoaded", true)
                    break
                }
            }
        }
    }
    
    // Update filtered model when search text changes or fonts become available
    function updateFilteredModel() {
        filteredFontModel.clear()
        
        if (!GoogleFonts || !GoogleFonts.availableFonts) {
            return
        }
        
        var fonts = GoogleFonts.availableFonts
        
        for (var i = 0; i < fonts.length; i++) {
            if (searchText === "" || fonts[i].toLowerCase().indexOf(searchText) !== -1) {
                filteredFontModel.append({
                    "fontFamily": fonts[i],
                    "isLoaded": GoogleFonts.isFontLoaded ? GoogleFonts.isFontLoaded(fonts[i], "regular") : false
                })
            }
        }
    }
    
    Component.onCompleted: {
        updateFilteredModel()
    }
    
    onSearchTextChanged: {
        updateFilteredModel()
    }
    
    Connections {
        target: GoogleFonts
        function onAvailableFontsChanged() {
            updateFilteredModel()
        }
    }
    
    // Clear search when component becomes visible
    onVisibleChanged: {
        if (visible) {
            clearSearch()
        }
    }
}