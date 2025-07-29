import QtQuick
import QtQuick.Controls
import Cubit 1.0

Item {
    id: root
    
    // Required properties
    required property string portType
    required property int portIndex
    required property bool hasIncomingEdge
    required property bool isHovered
    required property var canvas
    
    // Optional properties
    property var value: PortTypeConfigObject.getDefaultValue(portType)
    property real inputWidth: 80
    property real inputHeight: 24
    
    // Signals
    signal portValueChanged(var newValue)
    
    width: inputWidth
    height: inputHeight
    
    // Explicitly bind visibility with logging
    visible: {
        var shouldBeVisible = portType !== "Flow" && !hasIncomingEdge
        console.log("PortInput visibility calculation:", 
                   "portType:", portType,
                   "hasIncomingEdge:", hasIncomingEdge,
                   "=> should be visible:", shouldBeVisible)
        return shouldBeVisible
    }
    
    onVisibleChanged: {
        console.log("PortInput visibility changed to:", visible,
                   "portType:", portType,
                   "hasIncomingEdge:", hasIncomingEdge,
                   "parent.visible:", parent ? parent.visible : "no parent")
    }
    
    onHasIncomingEdgeChanged: {
        console.log("PortInput hasIncomingEdge property changed to:", hasIncomingEdge,
                   "portType:", portType,
                   "portIndex:", portIndex,
                   "=> visible should be:", portType !== "Flow" && !hasIncomingEdge)
    }
    
    // Function to handle clicks from parent
    function handleClick() {
        console.log("PortInput.handleClick called for port:", portIndex, "type:", portType)
        console.log("  inputConfig:", JSON.stringify(inputConfig))
        console.log("  textField.visible:", textField.visible)
        console.log("  numberField.visible:", numberField.visible)
        console.log("  comboBox.visible:", comboBox.visible)
        console.log("  Component visibility:", visible)
        console.log("  Parent visibility:", parent ? parent.visible : "no parent")
        
        if (inputConfigObject.inputType === "textInput" && textField.visible) {
            console.log("  Focusing text field")
            textField.forceActiveFocus()
            console.log("  TextField now has focus:", textField.activeFocus)
        } else if (inputConfigObject.inputType === "numberInput" && numberField.visible) {
            console.log("  Focusing number field")
            numberField.forceActiveFocus()
            console.log("  NumberField now has focus:", numberField.activeFocus)
        } else if (inputConfigObject.inputType === "selectInput" && comboBox.visible) {
            console.log("  Opening combo box")
            comboBox.popup.open()
        }
    }
    
    // Function to blur/close any active inputs
    function blur() {
        if (inputConfigObject.inputType === "selectInput" && comboBox.visible && comboBox.popup.opened) {
            comboBox.popup.close()
        }
        // TextFields and NumberFields will lose focus when parent takes focus
    }
    
    // Get the input configuration
    property var inputConfig: PortTypeConfigObject.getInputConfig(portType)
    
    // Track if any input has focus
    property bool hasInputFocus: (textField.visible && textField.activeFocus) || 
                                 (numberField.visible && numberField.activeFocus) ||
                                 (comboBox.visible && comboBox.popup.opened)
    
    // Direct component based on type instead of using Loader
    TextField {
        id: textField
        visible: inputConfigObject.inputType === "textInput"
        anchors.fill: parent
        text: root.value || ""
        placeholderText: inputConfigObject.placeholder || ""
        font.pixelSize: 11
        selectByMouse: true
        
        
        onTextChanged: {
            if (inputConfigObject.inputType === "textInput" && PortTypeConfigObject.validateValue(root.portType, text)) {
                root.value = text
                root.portValueChanged(text)
            }
        }
        
        onActiveFocusChanged: {
        }
    }
    
    TextField {
        id: numberField
        visible: inputConfigObject.inputType === "numberInput"
        anchors.fill: parent
        text: root.value !== undefined ? root.value.toString() : "0"
        placeholderText: inputConfigObject.placeholder || "0"
        font.pixelSize: 11
        selectByMouse: true
        validator: DoubleValidator {
            bottom: inputConfigObject.validation ? inputConfigObject.validation.min : -999999
            top: inputConfigObject.validation ? inputConfigObject.validation.max : 999999
            decimals: inputConfigObject.validation ? inputConfigObject.validation.decimals : 2
        }
        
        
        onTextChanged: {
            if (inputConfigObject.inputType === "numberInput") {
                var num = parseFloat(text)
                if (!isNaN(num) && PortTypeConfigObject.validateValue(root.portType, num)) {
                    root.value = num
                    root.portValueChanged(num)
                }
            }
        }
    }
    
    ComboBox {
        id: comboBox
        visible: inputConfigObject.inputType === "selectInput"
        anchors.fill: parent
        currentIndex: {
            // Find index of current value
            if (inputConfigObject.options && inputConfigObject.inputType === "selectInput") {
                for (var i = 0; i < inputConfigObject.options.length; i++) {
                    if (inputConfigObject.options[i].value === root.value) {
                        return i
                    }
                }
            }
            return 0
        }
        model: {
            if (inputConfigObject.options && inputConfigObject.inputType === "selectInput") {
                return inputConfigObject.options.map(function(opt) { return opt.label })
            }
            return []
        }
        font.pixelSize: 11
        
        
        // Custom popup that also scales
        popup: Popup {
            y: comboBox.height
            width: comboBox.width
            implicitHeight: contentItem.implicitHeight
            padding: 1
            
            // Apply zoom scale to match the canvas
            scale: root.canvas ? root.canvas.zoomLevel : 1.0
            transformOrigin: Item.TopLeft
            
            contentItem: ListView {
                clip: true
                implicitHeight: contentHeight
                model: comboBox.popup.visible ? comboBox.delegateModel : null
                currentIndex: comboBox.highlightedIndex
                
                ScrollIndicator.vertical: ScrollIndicator { }
            }
            
            background: Rectangle {
                border.color: "#CCCCCC"
                radius: 2
            }
        }
        
        onCurrentIndexChanged: {
            if (inputConfigObject.inputType === "selectInput" && inputConfigObject.options && currentIndex >= 0 && currentIndex < inputConfigObject.options.length) {
                var newValue = inputConfigObject.options[currentIndex].value
                root.value = newValue
                root.portValueChanged(newValue)
                console.log("Port", root.portIndex, "value changed to:", newValue)
            }
        }
    }
}