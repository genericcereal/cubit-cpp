import QtQuick
import QtQuick.Controls
import Cubit 1.0
import "../.."

// Shared visual component for control joints used by both DesignControls and ShapeControls
Rectangle {
    id: root
    
    // Visual properties
    property real size: 10
    property color baseColor: ConfigObject.controlResizeJointColor
    property color hoverColor: Qt.lighter(baseColor, 1.2)
    property color activeColor: Qt.lighter(baseColor, 1.4)
    property bool showInnerCircle: true
    property color innerCircleColor: ConfigObject.controlJointCircleFill
    property color innerCircleBorderColor: ConfigObject.controlJointCircleBorder
    
    // State properties
    property bool isHovered: false
    property bool isActive: false
    property bool isSelected: false  // For ShapeControls - shows which joint is being dragged
    
    width: size
    height: size
    radius: showInnerCircle ? 0 : size / 2
    color: isActive ? activeColor : (isHovered ? hoverColor : baseColor)
    antialiasing: true
    
    // Inner decorative circle (used by DesignControls)
    Rectangle {
        visible: parent.showInnerCircle
        anchors.centerIn: parent
        width: 10  // Fixed size to match DesignControls
        height: 10
        radius: 5
        // When selected (ShapeControls only), swap colors: blue fill with white border
        color: parent.isSelected ? parent.innerCircleBorderColor : parent.innerCircleColor
        border.color: parent.isSelected ? parent.innerCircleColor : parent.innerCircleBorderColor
        border.width: 1
        antialiasing: true
    }
}