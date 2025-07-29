import QtQuick
import QtQuick.Controls
import Cubit 1.0

Rectangle {
    id: componentVariantAddButton
    width: 30
    height: 30
    radius: 15
    color: ConfigObject.componentControlBarColor
    
    // Remove this property - now handled by designControls context property
    property var controller: null
    property var selectedVariant: null
    
    // Position it below the bottom bar
    anchors.horizontalCenter: parent.horizontalCenter
    anchors.top: parent.bottom
    anchors.topMargin: 10  // Position below the bottom bar
    
    z: 100  // Ensure it's above other controls
    
    signal addVariantClicked(var selectedVariant)
    
    Text {
        anchors.centerIn: parent
        text: "+"
        font.pixelSize: 20
        font.family: "Arial"
        font.bold: true
        color: "white"
    }
    
    MouseArea {
        anchors.fill: parent
        enabled: {
            // VariantAddButton is positioned using anchors, so its parent is DesignControls
            var designControlsRoot = componentVariantAddButton.parent
            return designControlsRoot ? !designControlsRoot.isAnyTextEditing : true
        }
        cursorShape: Qt.PointingHandCursor
        
        onClicked: {
            if (selectedVariant && selectedVariant.isComponentVariant()) {
                componentVariantAddButton.addVariantClicked(selectedVariant)
            }
        }
    }
}