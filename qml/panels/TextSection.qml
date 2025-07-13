import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import Cubit
import "../components"

PropertyGroup {
    id: root
    title: "Text"
    
    property var selectedElement
    
    visible: selectedElement && (selectedElement.elementType === "Text" || selectedElement.elementType === "TextVariant" || (selectedElement.elementType === "FrameComponentInstance" && selectedElement.hasOwnProperty("content")))
    
    property var textProps: [
        {
            name: "Content",
            type: "text",
            getter: () => selectedElement && selectedElement.content !== undefined ? selectedElement.content : "",
            setter: v => {
                if (selectedElement && selectedElement.content !== undefined) {
                    selectedElement.content = v
                }
            }
        },
        {
            name: "Size",
            type: "font_spinbox",
            from: 8,
            to: 144,
            getter: () => selectedElement && selectedElement.font ? selectedElement.font.pixelSize : 14,
            setter: v => {
                if (selectedElement && selectedElement.font !== undefined) {
                    var newFont = selectedElement.font
                    newFont.pixelSize = v
                    selectedElement.font = newFont
                }
            }
        }
    ]
    
    Component {
        id: textFieldComp
        TextField {
            Layout.fillWidth: true
        }
    }
    
    Component {
        id: fontSpinBoxComp
        SpinBox {
            Layout.fillWidth: true
        }
    }
    
    content: [
        Repeater {
            model: textProps
            
            delegate: LabeledField {
                label: modelData.name
                
                delegate: [
                    Loader {
                        anchors.left: parent.left
                        anchors.right: parent.right
                        sourceComponent: modelData.type === "text" ? textFieldComp
                                       : modelData.type === "font_spinbox" ? fontSpinBoxComp
                                       : null
                        
                        onLoaded: {
                            if (modelData.type === "text") {
                                item.text = Qt.binding(modelData.getter)
                                item.textChanged.connect(function() {
                                    modelData.setter(item.text)
                                })
                            } else if (modelData.type === "font_spinbox") {
                                item.from = modelData.from
                                item.to = modelData.to
                                item.value = Qt.binding(modelData.getter)
                                item.valueModified.connect(function() {
                                    modelData.setter(item.value)
                                })
                            }
                        }
                    }
                ]
            }
        }
    ]
}