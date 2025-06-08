import QtQuick 2.15

Item {
    id: root
    
    property var selectedElements: []
    
    // For now, just a placeholder
    // Will implement full controls in Phase 3
    
    Repeater {
        model: selectedElements
        
        Rectangle {
            x: modelData ? modelData.x - 4 : 0
            y: modelData ? modelData.y - 4 : 0
            width: modelData ? modelData.width + 8 : 0
            height: modelData ? modelData.height + 8 : 0
            
            color: "transparent"
            border.color: "#0066cc"
            border.width: 2
            
            // Corner handles
            Repeater {
                model: 4
                Rectangle {
                    width: 8
                    height: 8
                    color: "#0066cc"
                    
                    x: {
                        switch(index) {
                            case 0: return -4 // top-left
                            case 1: return parent.width - 4 // top-right
                            case 2: return parent.width - 4 // bottom-right
                            case 3: return -4 // bottom-left
                        }
                    }
                    
                    y: {
                        switch(index) {
                            case 0: return -4 // top-left
                            case 1: return -4 // top-right
                            case 2: return parent.height - 4 // bottom-right
                            case 3: return parent.height - 4 // bottom-left
                        }
                    }
                }
            }
        }
    }
}