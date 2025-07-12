import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import Cubit

ColumnLayout {
    id: root
    spacing: 0
    
    property var selectedElement
    property var editableProperties: []
    
    // Position & Size section - for elements without a parent
    GroupBox {
        Layout.fillWidth: true
        Layout.margins: 10
        title: "Position & Size"
        visible: selectedElement && selectedElement.isVisual && !(selectedElement.isDesignElement && selectedElement.parentId)
        
        GridLayout {
            anchors.fill: parent
            columns: 2
            columnSpacing: 10
            rowSpacing: 5
            
            Label { text: "X:" }
            SpinBox {
                Layout.fillWidth: true
                from: -9999
                to: 9999
                value: selectedElement && selectedElement.isVisual ? Math.round(selectedElement.x) : 0
                onValueModified: {
                    if (!selectedElement || !selectedElement.isVisual) return
                    if (value === undefined || isNaN(value)) return
                    if (value !== Math.round(selectedElement.x)) {
                        selectedElement.x = value
                    }
                }
            }
            
            Label { text: "Y:" }
            SpinBox {
                Layout.fillWidth: true
                from: -9999
                to: 9999
                value: selectedElement && selectedElement.isVisual ? Math.round(selectedElement.y) : 0
                onValueModified: {
                    if (!selectedElement || !selectedElement.isVisual) return
                    if (value === undefined || isNaN(value)) return
                    if (value !== Math.round(selectedElement.y)) {
                        selectedElement.y = value
                    }
                }
            }
            
            Label { text: "Width:" }
            RowLayout {
                Layout.fillWidth: true
                spacing: 5
                
                SpinBox {
                    id: widthSpinBox
                    Layout.fillWidth: true
                    from: 1
                    to: 9999
                    value: selectedElement && selectedElement.isVisual ? Math.round(selectedElement.width) : 0
                    onValueModified: {
                        if (!selectedElement || !selectedElement.isVisual) return
                        if (value === undefined || isNaN(value)) return
                        if (value !== Math.round(selectedElement.width)) {
                            selectedElement.width = value
                        }
                    }
                }
                
                ComboBox {
                    Layout.preferredWidth: 100
                    model: ["fixed", "relative", "fill", "fit content", "viewport"]
                    visible: selectedElement && (selectedElement.elementType === "Frame" || selectedElement.elementType === "FrameComponentInstance")
                    currentIndex: {
                        if (selectedElement && (selectedElement.elementType === "Frame" || selectedElement.elementType === "FrameComponentInstance")) {
                            switch (selectedElement.widthType) {
                                case 0: return 0
                                case 1: return 1
                                case 2: return 2
                                case 3: return 3
                                case 4: return 4
                                default: return 0
                            }
                        }
                        return 0
                    }
                    onActivated: function(index) {
                        if (selectedElement && (selectedElement.elementType === "Frame" || selectedElement.elementType === "FrameComponentInstance")) {
                            selectedElement.widthType = index
                        }
                    }
                }
            }
            
            Label { text: "Height:" }
            RowLayout {
                Layout.fillWidth: true
                spacing: 5
                
                SpinBox {
                    id: heightSpinBox
                    Layout.fillWidth: true
                    from: 1
                    to: 9999
                    value: selectedElement && selectedElement.isVisual ? Math.round(selectedElement.height) : 0
                    onValueModified: {
                        if (!selectedElement || !selectedElement.isVisual) return
                        if (value === undefined || isNaN(value)) return
                        if (value !== Math.round(selectedElement.height)) {
                            selectedElement.height = value
                        }
                    }
                }
                
                ComboBox {
                    Layout.preferredWidth: 100
                    model: ["fixed", "relative", "fill", "fit content", "viewport"]
                    visible: selectedElement && (selectedElement.elementType === "Frame" || selectedElement.elementType === "FrameComponentInstance")
                    currentIndex: {
                        if (selectedElement && (selectedElement.elementType === "Frame" || selectedElement.elementType === "FrameComponentInstance")) {
                            switch (selectedElement.heightType) {
                                case 0: return 0
                                case 1: return 1
                                case 2: return 2
                                case 3: return 3
                                case 4: return 4
                                default: return 0
                            }
                        }
                        return 0
                    }
                    onActivated: function(index) {
                        if (selectedElement && (selectedElement.elementType === "Frame" || selectedElement.elementType === "FrameComponentInstance")) {
                            selectedElement.heightType = index
                        }
                    }
                }
            }
        }
    }
    
    // Size section - for DesignElements with a parent
    GroupBox {
        Layout.fillWidth: true
        Layout.margins: 10
        title: "Size"
        visible: selectedElement && selectedElement.isDesignElement && selectedElement.parentId
        
        GridLayout {
            anchors.fill: parent
            columns: 2
            columnSpacing: 10
            rowSpacing: 5
            
            Label { text: "Width:" }
            RowLayout {
                Layout.fillWidth: true
                spacing: 5
                
                SpinBox {
                    id: widthSpinBoxSeparate
                    Layout.fillWidth: true
                    from: 1
                    to: 9999
                    value: selectedElement ? Math.round(selectedElement.width) : 0
                    onValueModified: function(value) {
                        if (selectedElement && value !== Math.round(selectedElement.width)) {
                            selectedElement.width = value
                        }
                    }
                }
                
                ComboBox {
                    Layout.preferredWidth: 100
                    model: ["fixed", "relative", "fill", "fit content", "viewport"]
                    visible: selectedElement && (selectedElement.elementType === "Frame" || selectedElement.elementType === "FrameComponentInstance")
                    currentIndex: {
                        if (selectedElement && (selectedElement.elementType === "Frame" || selectedElement.elementType === "FrameComponentInstance")) {
                            switch (selectedElement.widthType) {
                                case 0: return 0
                                case 1: return 1
                                case 2: return 2
                                case 3: return 3
                                case 4: return 4
                                default: return 0
                            }
                        }
                        return 0
                    }
                    onActivated: function(index) {
                        if (selectedElement && (selectedElement.elementType === "Frame" || selectedElement.elementType === "FrameComponentInstance")) {
                            selectedElement.widthType = index
                        }
                    }
                }
            }
            
            Label { text: "Height:" }
            RowLayout {
                Layout.fillWidth: true
                spacing: 5
                
                SpinBox {
                    id: heightSpinBoxSeparate
                    Layout.fillWidth: true
                    from: 1
                    to: 9999
                    value: selectedElement ? Math.round(selectedElement.height) : 0
                    onValueModified: function(value) {
                        if (selectedElement && value !== Math.round(selectedElement.height)) {
                            selectedElement.height = value
                        }
                    }
                }
                
                ComboBox {
                    Layout.preferredWidth: 100
                    model: ["fixed", "relative", "fill", "fit content", "viewport"]
                    visible: selectedElement && (selectedElement.elementType === "Frame" || selectedElement.elementType === "FrameComponentInstance")
                    currentIndex: {
                        if (selectedElement && (selectedElement.elementType === "Frame" || selectedElement.elementType === "FrameComponentInstance")) {
                            switch (selectedElement.heightType) {
                                case 0: return 0
                                case 1: return 1
                                case 2: return 2
                                case 3: return 3
                                case 4: return 4
                                default: return 0
                            }
                        }
                        return 0
                    }
                    onActivated: function(index) {
                        if (selectedElement && (selectedElement.elementType === "Frame" || selectedElement.elementType === "FrameComponentInstance")) {
                            selectedElement.heightType = index
                        }
                    }
                }
            }
        }
    }
}