import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import Cubit

GroupBox {
    id: root
    Layout.fillWidth: true
    Layout.margins: 10
    title: "Position"
    
    property var selectedElement
    property var editableProperties: []
    property var selectedDesignElement: selectedElement && selectedElement.isDesignElement ? selectedElement : null
    
    visible: selectedElement && selectedElement.isDesignElement && selectedElement.parentId
    
    GridLayout {
        anchors.fill: parent
        columns: 2
        columnSpacing: 10
        rowSpacing: 5
        
        Label { 
            text: "Position:" 
            visible: selectedElement && (selectedElement.elementType === "Frame" || selectedElement.elementType === "Text")
        }
        ComboBox {
            Layout.fillWidth: true
            visible: selectedElement && (selectedElement.elementType === "Frame" || selectedElement.elementType === "Text")
            model: ["Relative", "Absolute", "Fixed"]
            currentIndex: {
                if (selectedElement && (selectedElement.elementType === "Frame" || selectedElement.elementType === "Text")) {
                    switch (selectedElement.position) {
                        case 0: return 0
                        case 1: return 1
                        case 2: return 2
                        default: return 0
                    }
                }
                return 0
            }
            onActivated: function(index) {
                if (selectedElement && (selectedElement.elementType === "Frame" || selectedElement.elementType === "Text")) {
                    selectedElement.position = index
                }
            }
        }
        
        Label { text: "Left:" }
        RowLayout {
            Layout.fillWidth: true
            spacing: 5
            
            SpinBox {
                id: leftSpinBox
                Layout.fillWidth: true
                from: -9999
                to: 9999
                value: selectedDesignElement && selectedDesignElement.left !== undefined ? Math.round(selectedDesignElement.left) : 0
                onValueModified: function(value) {
                    if (selectedDesignElement && selectedDesignElement.left !== undefined && value !== Math.round(selectedDesignElement.left)) {
                        selectedDesignElement.left = value
                    }
                }
            }
            
            Button {
                id: leftAnchorButton
                Layout.preferredWidth: 30
                Layout.preferredHeight: 30
                checkable: true
                checked: selectedDesignElement && selectedDesignElement.leftAnchored
                text: "⚓"
                font.pixelSize: 16
                
                background: Rectangle {
                    color: leftAnchorButton.checked ? "#4080ff" : (leftAnchorButton.hovered ? "#f0f0f0" : "#ffffff")
                    border.color: leftAnchorButton.checked ? "#2060ff" : "#d0d0d0"
                    border.width: 1
                    radius: 4
                }
                
                onToggled: {
                    if (selectedDesignElement) {
                        if (checked) {
                            selectedDesignElement.left = leftSpinBox.value
                        }
                        selectedDesignElement.leftAnchored = checked
                    }
                }
            }
        }
        
        Label { text: "Top:" }
        RowLayout {
            Layout.fillWidth: true
            spacing: 5
            
            SpinBox {
                id: topSpinBox
                Layout.fillWidth: true
                from: -9999
                to: 9999
                value: selectedDesignElement && selectedDesignElement.top !== undefined ? Math.round(selectedDesignElement.top) : 0
                onValueModified: function(value) {
                    if (selectedDesignElement && selectedDesignElement.top !== undefined && value !== Math.round(selectedDesignElement.top)) {
                        selectedDesignElement.top = value
                    }
                }
            }
            
            Button {
                id: topAnchorButton
                Layout.preferredWidth: 30
                Layout.preferredHeight: 30
                checkable: true
                checked: selectedDesignElement && selectedDesignElement.topAnchored
                text: "⚓"
                font.pixelSize: 16
                
                background: Rectangle {
                    color: topAnchorButton.checked ? "#4080ff" : (topAnchorButton.hovered ? "#f0f0f0" : "#ffffff")
                    border.color: topAnchorButton.checked ? "#2060ff" : "#d0d0d0"
                    border.width: 1
                    radius: 4
                }
                
                onToggled: {
                    if (selectedDesignElement) {
                        if (checked) {
                            selectedDesignElement.top = topSpinBox.value
                        }
                        selectedDesignElement.topAnchored = checked
                    }
                }
            }
        }
        
        Label { text: "Right:" }
        RowLayout {
            Layout.fillWidth: true
            spacing: 5
            
            SpinBox {
                id: rightSpinBox
                Layout.fillWidth: true
                from: -9999
                to: 9999
                value: selectedDesignElement && selectedDesignElement.right !== undefined ? Math.round(selectedDesignElement.right) : 0
                onValueModified: function(value) {
                    if (selectedDesignElement && selectedDesignElement.right !== undefined && value !== Math.round(selectedDesignElement.right)) {
                        selectedDesignElement.right = value
                    }
                }
            }
            
            Button {
                id: rightAnchorButton
                Layout.preferredWidth: 30
                Layout.preferredHeight: 30
                checkable: true
                checked: selectedDesignElement && selectedDesignElement.rightAnchored
                text: "⚓"
                font.pixelSize: 16
                
                background: Rectangle {
                    color: rightAnchorButton.checked ? "#4080ff" : (rightAnchorButton.hovered ? "#f0f0f0" : "#ffffff")
                    border.color: rightAnchorButton.checked ? "#2060ff" : "#d0d0d0"
                    border.width: 1
                    radius: 4
                }
                
                onToggled: {
                    if (selectedDesignElement) {
                        if (checked) {
                            selectedDesignElement.right = rightSpinBox.value
                        }
                        selectedDesignElement.rightAnchored = checked
                    }
                }
            }
        }
        
        Label { text: "Bottom:" }
        RowLayout {
            Layout.fillWidth: true
            spacing: 5
            
            SpinBox {
                id: bottomSpinBox
                Layout.fillWidth: true
                from: -9999
                to: 9999
                value: selectedDesignElement && selectedDesignElement.bottom !== undefined ? Math.round(selectedDesignElement.bottom) : 0
                onValueModified: function(value) {
                    if (selectedDesignElement && selectedDesignElement.bottom !== undefined && value !== Math.round(selectedDesignElement.bottom)) {
                        selectedDesignElement.bottom = value
                    }
                }
            }
            
            Button {
                id: bottomAnchorButton
                Layout.preferredWidth: 30
                Layout.preferredHeight: 30
                checkable: true
                checked: selectedDesignElement && selectedDesignElement.bottomAnchored
                text: "⚓"
                font.pixelSize: 16
                
                background: Rectangle {
                    color: bottomAnchorButton.checked ? "#4080ff" : (bottomAnchorButton.hovered ? "#f0f0f0" : "#ffffff")
                    border.color: bottomAnchorButton.checked ? "#2060ff" : "#d0d0d0"
                    border.width: 1
                    radius: 4
                }
                
                onToggled: {
                    if (selectedDesignElement) {
                        if (checked) {
                            selectedDesignElement.bottom = bottomSpinBox.value
                        }
                        selectedDesignElement.bottomAnchored = checked
                    }
                }
            }
        }
    }
}