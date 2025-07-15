import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

Item {
    id: root
    
    // Properties
    property color currentColor: Qt.hsla(hue / 360, saturation, lightness, alphaValue)
    property real hue: 200
    property real saturation: 1.0
    property real lightness: 0.5
    property real alphaValue: 1.0
    property int colorFormat: 1 // 0: RGB, 1: HEX, 2: HSL
    property int fillType: 0 // 0: Solid, 1: Linear, 2: Radial, 3: Conic, 4: Image
    
    // Internal flag to prevent feedback loops during external updates
    property bool _updatingFromExternal: false
    
    // Signals
    signal colorChanged(color newColor)
    signal eyeButtonClicked()
    
    implicitHeight: colorTypeRow.y + colorTypeRow.height + 10
    implicitWidth: 230
    
    // Fill type selector
    ComboBox {
        id: fillTypeCombo
        anchors.horizontalCenter: parent.horizontalCenter
        anchors.top: parent.top
        anchors.topMargin: 10
        width: parent.width - 20
        model: ["Solid", "Linear", "Radial", "Conic", "Image"]
        currentIndex: root.fillType
        
        onCurrentIndexChanged: {
            if (currentIndex !== root.fillType) {
                root.fillType = currentIndex
            }
        }
    }
    
    // Shade selector
    ShadeSelector {
        id: shadeSelector
        anchors.horizontalCenter: parent.horizontalCenter
        anchors.top: fillTypeCombo.bottom
        anchors.topMargin: 10
        width: parent.width - 20
        height: 100
        
        hue: root.hue
        saturation: root.saturation
        lightness: root.lightness
        
        onShadeUpdated: function(newSaturation, newLightness) {
            root.saturation = newSaturation
            root.lightness = newLightness
            updateColor()
        }
    }
    
    // Hue slider
    HueSlider {
        id: hueSlider
        anchors.horizontalCenter: parent.horizontalCenter
        anchors.top: shadeSelector.bottom
        anchors.topMargin: 10
        width: parent.width - 20
        
        hue: root.hue
        saturation: 1.0 // Always show full saturation for hue gradient
        lightness: 0.5  // Medium lightness for hue gradient
        
        onHueUpdated: function(newHue) {
            root.hue = newHue
            shadeSelector.hue = newHue
            updateColor()
        }
    }
    
    // Opacity slider
    OpacitySlider {
        id: opacitySlider
        anchors.horizontalCenter: parent.horizontalCenter
        anchors.top: hueSlider.bottom
        anchors.topMargin: 10
        width: parent.width - 20
        
        color: Qt.hsla(root.hue / 360, root.saturation, root.lightness, 1.0)
        alpha: root.alphaValue
        
        onAlphaUpdated: function(newAlpha) {
            root.alphaValue = newAlpha
            updateColor()
        }
    }
    
    // Row with color value and opacity percentage inputs
    Row {
        id: valueRow
        anchors.horizontalCenter: parent.horizontalCenter
        anchors.top: opacitySlider.bottom
        anchors.topMargin: 10
        width: parent.width - 20
        spacing: 10
        
        // Color value input
        TextField {
            id: colorInput
            width: (parent.width - parent.spacing) * 0.65
            text: formatColorString()
            
            onAccepted: parseColorInput(text)
        }
        
        // Opacity percentage input
        TextField {
            id: opacityInput
            width: (parent.width - parent.spacing) * 0.35
            text: Math.round(root.alphaValue * 100) + "%"
            
            onAccepted: {
                var value = text.trim()
                if (value.endsWith("%")) {
                    value = value.substring(0, value.length - 1)
                }
                var percentage = parseFloat(value)
                if (!isNaN(percentage)) {
                    root.alphaValue = Math.max(0, Math.min(100, percentage)) / 100
                    opacitySlider.alpha = root.alphaValue
                    updateColor()
                }
            }
        }
    }
    
    // Row with color type selector and eye button
    Row {
        id: colorTypeRow
        anchors.horizontalCenter: parent.horizontalCenter
        anchors.top: valueRow.bottom
        anchors.topMargin: 10
        width: parent.width - 20
        spacing: 10
        
        // Color type selector
        ComboBox {
            id: colorTypeCombo
            width: (parent.width - parent.spacing) * 0.65
            model: ["RGB", "HEX", "HSL"]
            currentIndex: root.colorFormat
            
            onCurrentIndexChanged: {
                if (currentIndex !== root.colorFormat) {
                    root.colorFormat = currentIndex
                    colorInput.text = formatColorString()
                }
            }
        }
        
        // Eye button
        Button {
            width: (parent.width - parent.spacing) * 0.35
            text: "👁"
            font.pixelSize: 18
            
            onClicked: root.eyeButtonClicked()
        }
    }
    
    // Helper functions
    function updateColor() {
        var newColor = Qt.hsla(root.hue / 360, root.saturation, root.lightness, root.alphaValue)
        root.currentColor = newColor
        
        // Only emit colorChanged if we're not updating from external binding changes
        if (!root._updatingFromExternal) {
            root.colorChanged(newColor)
        }
        
        colorInput.text = formatColorString()
        opacityInput.text = Math.round(root.alphaValue * 100) + "%"
    }
    
    function formatColorString() {
        var color = root.currentColor
        
        switch (root.colorFormat) {
            case 0: // RGB
                var r = Math.round(color.r * 255)
                var g = Math.round(color.g * 255)
                var b = Math.round(color.b * 255)
                return "rgb(" + r + ", " + g + ", " + b + ")"
                
            case 1: // HEX
                var rh = Math.round(color.r * 255).toString(16).padStart(2, '0')
                var gh = Math.round(color.g * 255).toString(16).padStart(2, '0')
                var bh = Math.round(color.b * 255).toString(16).padStart(2, '0')
                return "#" + rh + gh + bh
                
            case 2: // HSL
                var h = Math.round(root.hue)
                var s = Math.round(root.saturation * 100)
                var l = Math.round(root.lightness * 100)
                return "hsl(" + h + ", " + s + "%, " + l + "%)"
                
            default:
                return "#000000"
        }
    }
    
    function parseColorInput(text) {
        text = text.trim()
        
        if (root.colorFormat === 0) { // RGB
            var rgbMatch = text.match(/rgb\s*\(\s*(\d+)\s*,\s*(\d+)\s*,\s*(\d+)\s*\)/)
            if (rgbMatch) {
                var r = Math.max(0, Math.min(255, parseInt(rgbMatch[1]))) / 255
                var g = Math.max(0, Math.min(255, parseInt(rgbMatch[2]))) / 255
                var b = Math.max(0, Math.min(255, parseInt(rgbMatch[3]))) / 255
                
                var color = Qt.rgba(r, g, b, 1)
                root.hue = color.hslHue * 360
                root.saturation = color.hslSaturation
                root.lightness = color.hslLightness
                
                shadeSelector.hue = root.hue
                shadeSelector.saturation = root.saturation
                shadeSelector.lightness = root.lightness
                hueSlider.hue = root.hue
                
                updateColor()
            }
        } else if (root.colorFormat === 1) { // HEX
            var hex = text
            if (hex.startsWith("#")) {
                hex = hex.substring(1)
            }
            if (hex.length === 6) {
                var r = parseInt(hex.substring(0, 2), 16) / 255
                var g = parseInt(hex.substring(2, 4), 16) / 255
                var b = parseInt(hex.substring(4, 6), 16) / 255
                
                var color = Qt.rgba(r, g, b, 1)
                root.hue = color.hslHue * 360
                root.saturation = color.hslSaturation
                root.lightness = color.hslLightness
                
                shadeSelector.hue = root.hue
                shadeSelector.saturation = root.saturation
                shadeSelector.lightness = root.lightness
                hueSlider.hue = root.hue
                
                updateColor()
            }
        } else if (root.colorFormat === 2) { // HSL
            var hslMatch = text.match(/hsl\s*\(\s*(\d+)\s*,\s*(\d+)%?\s*,\s*(\d+)%?\s*\)/)
            if (hslMatch) {
                root.hue = Math.max(0, Math.min(360, parseInt(hslMatch[1])))
                root.saturation = Math.max(0, Math.min(100, parseInt(hslMatch[2]))) / 100
                root.lightness = Math.max(0, Math.min(100, parseInt(hslMatch[3]))) / 100
                
                shadeSelector.hue = root.hue
                shadeSelector.saturation = root.saturation
                shadeSelector.lightness = root.lightness
                hueSlider.hue = root.hue
                
                updateColor()
            }
        }
    }
    
    // Update child components when properties change
    onHueChanged: {
        _updatingFromExternal = true
        shadeSelector.hue = hue
        hueSlider.hue = hue
        updateColor()
        _updatingFromExternal = false
    }
    
    onSaturationChanged: {
        _updatingFromExternal = true
        shadeSelector.saturation = saturation
        updateColor()
        _updatingFromExternal = false
    }
    
    onLightnessChanged: {
        _updatingFromExternal = true
        shadeSelector.lightness = lightness
        updateColor()
        _updatingFromExternal = false
    }
    
    onAlphaValueChanged: {
        _updatingFromExternal = true
        opacitySlider.alpha = alphaValue
        updateColor()
        _updatingFromExternal = false
    }
    
    onColorFormatChanged: {
        colorTypeCombo.currentIndex = colorFormat
    }
    
    onFillTypeChanged: {
        fillTypeCombo.currentIndex = fillType
    }
}