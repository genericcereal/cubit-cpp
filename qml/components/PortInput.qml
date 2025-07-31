import QtQuick
import QtQuick.Controls
import Cubit 1.0
import "."

Item {
    id: root
    
    // Required properties
    required property string portType
    required property int portIndex
    required property bool hasIncomingEdge
    required property bool isHovered
    required property var canvas
    
    // Optional properties
    property var value: PortTypeConfig.getDefaultValue(portType)
    property real inputWidth: 80
    property real inputHeight: 24
    
    // Signals
    signal portValueChanged(var newValue)
    
    width: inputWidth
    height: inputHeight
    
    // Explicitly bind visibility with logging
    visible: portType !== "Flow" && !hasIncomingEdge
    
    
    // Function to handle clicks from parent
    function handleClick() {
        
        if (inputConfig.inputType === "textInput" && textField.visible) {
            textField.forceActiveFocus()
        } else if (inputConfig.inputType === "numberInput" && numberField.visible) {
            numberField.forceActiveFocus()
        } else if (inputConfig.inputType === "selectInput" && comboBox.visible) {
            comboBox.popup.open()
        }
    }
    
    // Function to blur/close any active inputs
    function blur() {
        if (inputConfig.inputType === "selectInput" && comboBox.visible && comboBox.popup.opened) {
            comboBox.popup.close()
        }
        // TextFields and NumberFields will lose focus when parent takes focus
    }
    
    // Get the input configuration
    property var inputConfig: PortTypeConfig.getInputConfig(portType)
    
    // Track if any input has focus
    property bool hasInputFocus: (textField.visible && textField.activeFocus) || 
                                 (numberField.visible && numberField.activeFocus) ||
                                 (comboBox.visible && comboBox.popup.opened)
    
    // Direct component based on type instead of using Loader
    TextField {
        id: textField
        visible: inputConfig.inputType === "textInput"
        anchors.fill: parent
        text: root.value || ""
        placeholderText: inputConfig.placeholder || ""
        font.pixelSize: 11
        selectByMouse: true
        
        
        onTextChanged: {
            if (inputConfig.inputType === "textInput" && text !== root.value && PortTypeConfig.validateValue(root.portType, text)) {
                root.value = text
                root.portValueChanged(text)
            }
        }
        
        onActiveFocusChanged: {
        }
    }
    
    TextField {
        id: numberField
        visible: inputConfig.inputType === "numberInput"
        anchors.fill: parent
        placeholderText: inputConfig.placeholder || "0"
        font.pixelSize: 11
        selectByMouse: true
        validator: DoubleValidator {
            bottom: inputConfig && inputConfig.validation && inputConfig.validation.min !== undefined ? inputConfig.validation.min : -999999
            top: inputConfig && inputConfig.validation && inputConfig.validation.max !== undefined ? inputConfig.validation.max : 999999
            decimals: inputConfig && inputConfig.validation && inputConfig.validation.decimals !== undefined ? inputConfig.validation.decimals : 2
        }
        
        // Use Component.onCompleted to set initial text without triggering binding loop
        Component.onCompleted: {
            text = root.value !== undefined ? root.value.toString() : "0"
        }
        
        // Update text when root.value changes externally
        Connections {
            target: root
            function onValueChanged() {
                if (numberField.text !== root.value.toString()) {
                    numberField.text = root.value.toString()
                }
            }
        }
        
        onTextChanged: {
            if (inputConfig.inputType === "numberInput") {
                var num = parseFloat(text)
                if (!isNaN(num) && num !== root.value && PortTypeConfig.validateValue(root.portType, num)) {
                    root.value = num
                    root.portValueChanged(num)
                }
            }
        }
    }
    
    ComboBox {
        id: comboBox
        visible: inputConfig.inputType === "selectInput"
        anchors.fill: parent
        currentIndex: {
            // Find index of current value
            if (inputConfig.options && inputConfig.inputType === "selectInput") {
                for (var i = 0; i < inputConfig.options.length; i++) {
                    if (inputConfig.options[i].value === root.value) {
                        return i
                    }
                }
            }
            return 0
        }
        model: {
            if (inputConfig.options && inputConfig.inputType === "selectInput") {
                return inputConfig.options.map(function(opt) { return opt.label })
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
            if (inputConfig.inputType === "selectInput" && inputConfig.options && currentIndex >= 0 && currentIndex < inputConfig.options.length) {
                var newValue = inputConfig.options[currentIndex].value
                root.value = newValue
                root.portValueChanged(newValue)
            }
        }
    }
}