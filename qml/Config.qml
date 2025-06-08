pragma Singleton
import QtQuick

QtObject {
    // Colors
    readonly property string selectionColor: "#0066cc"
    readonly property string hoverColor: "#999999"
    readonly property string canvasBackground: "#f5f5f5"
    readonly property string panelBackground: "#ffffff"
    readonly property string panelHeaderBackground: "#f0f0f0"
    
    // Control colors
    readonly property color controlBarColor: Qt.rgba(1, 0, 0, 0.1)           // Red with 10% opacity
    readonly property color controlRotationJointColor: Qt.rgba(1, 0, 0, 0.1) // Red with 10% opacity
    readonly property color controlResizeJointColor: Qt.rgba(0, 0, 1, 0.1)   // Blue with 10% opacity
    readonly property color controlInnerRectColor: Qt.rgba(1, 0.8, 0, 0.05)  // Yellow with 5% opacity
    
    // Sizes
    readonly property int defaultElementWidth: 200
    readonly property int defaultElementHeight: 150
    readonly property int selectionHandleSize: 8
    readonly property int controlMargin: 4
    
    // Control sizes
    readonly property int controlBarWidth: 10
    readonly property int controlBarHeight: 10
    readonly property int controlRotationJointSize: 20  // Blue rotation joints (not functional yet)
    readonly property int controlResizeJointSize: 10    // Yellow resize joints
    readonly property int controlLineWidth: 1            // Center lines in bars
    readonly property int controlJointOverlap: 10       // How much joints overlap with bars
    
    // Z-index layers
    readonly property int zBackground: 0
    readonly property int zElements: 10
    readonly property int zControls: 1000
    readonly property int zSelection: 2000
    readonly property int zPanels: 3000
    
    // Canvas
    readonly property int canvasWidth: 4000
    readonly property int canvasHeight: 4000
    readonly property real minZoom: 0.1
    readonly property real maxZoom: 5.0
    readonly property real zoomStep: 0.1
    
    // Animation
    readonly property int animationDuration: 150
    
    // Performance
    readonly property int fpsUpdateInterval: 1000 // milliseconds
}