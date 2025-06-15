import QtQuick

pragma Singleton

QtObject {
    id: root
    
    // Mapping from port types to input configurations
    readonly property var typeInputMapping: {
        "String": {
            "inputType": "textInput",
            "defaultValue": "",
            "placeholder": "Enter text...",
            "validation": {
                "maxLength": 1000
            }
        },
        
        "Number": {
            "inputType": "numberInput",
            "defaultValue": 0,
            "placeholder": "0",
            "validation": {
                "min": -999999,
                "max": 999999,
                "decimals": 2
            }
        },
        
        "Boolean": {
            "inputType": "selectInput",
            "defaultValue": false,
            "options": [
                {"value": true, "label": "Yes"},
                {"value": false, "label": "No"}
            ]
        },
        
        "Flow": {
            "inputType": "none",  // Flow ports don't have inputs
            "defaultValue": null
        }
    }
    
    // Future data types can be added here:
    /*
    "Integer": {
        "inputType": "numberInput",
        "defaultValue": 0,
        "placeholder": "0",
        "validation": {
            "min": -999999,
            "max": 999999,
            "decimals": 0
        }
    },
    
    "Float": {
        "inputType": "numberInput",
        "defaultValue": 0.0,
        "placeholder": "0.0",
        "validation": {
            "min": -999999.99,
            "max": 999999.99,
            "decimals": 6
        }
    },
    
    "Color": {
        "inputType": "colorPicker",
        "defaultValue": "#000000"
    },
    
    "Enum": {
        "inputType": "selectInput",
        "defaultValue": "",
        "options": []  // Would be populated dynamically
    },
    
    "Array": {
        "inputType": "arrayInput",
        "defaultValue": [],
        "itemType": "String"  // Type of array elements
    },
    
    "Object": {
        "inputType": "objectInput",
        "defaultValue": {},
        "schema": {}  // Would define object structure
    },
    
    "File": {
        "inputType": "filePicker",
        "defaultValue": "",
        "filters": ["*.*"]
    },
    
    "Date": {
        "inputType": "datePicker",
        "defaultValue": new Date()
    }
    */
    
    // Helper function to get input config for a type
    function getInputConfig(portType) {
        return typeInputMapping[portType] || typeInputMapping["String"]  // Default to String
    }
    
    // Get the input type for a port type
    function getInputType(portType) {
        var config = getInputConfig(portType)
        return config.inputType
    }
    
    // Get the default value for a port type
    function getDefaultValue(portType) {
        var config = getInputConfig(portType)
        return config.defaultValue
    }
    
    // Check if a port type has an input control
    function hasInputControl(portType) {
        var inputType = getInputType(portType)
        return inputType !== "none"
    }
    
    // Validate a value for a given port type
    function validateValue(portType, value) {
        var config = getInputConfig(portType)
        
        switch (config.inputType) {
            case "textInput":
                if (config.validation && config.validation.maxLength) {
                    return value.length <= config.validation.maxLength
                }
                return true
                
            case "numberInput":
                var num = parseFloat(value)
                if (isNaN(num)) return false
                
                if (config.validation) {
                    if (config.validation.min !== undefined && num < config.validation.min) return false
                    if (config.validation.max !== undefined && num > config.validation.max) return false
                }
                return true
                
            case "selectInput":
                // Check if value is in options
                if (config.options) {
                    for (var i = 0; i < config.options.length; i++) {
                        if (config.options[i].value === value) return true
                    }
                    return false
                }
                return true
                
            default:
                return true
        }
    }
    
    // Format a value for display
    function formatValue(portType, value) {
        var config = getInputConfig(portType)
        
        switch (config.inputType) {
            case "numberInput":
                if (config.validation && config.validation.decimals !== undefined) {
                    return parseFloat(value).toFixed(config.validation.decimals)
                }
                return value.toString()
                
            case "selectInput":
                // Find label for value
                if (config.options) {
                    for (var i = 0; i < config.options.length; i++) {
                        if (config.options[i].value === value) {
                            return config.options[i].label
                        }
                    }
                }
                return value.toString()
                
            default:
                return value.toString()
        }
    }
}