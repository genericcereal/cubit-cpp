pragma Singleton
import QtQuick

QtObject {
    // Colors
    readonly property string selectionColor: "#0066cc"
    readonly property string hoverColor: "#999999"
    readonly property string canvasBackground: "#f5f5f5"
    readonly property string panelBackground: "#ffffff"
    readonly property string panelHeaderBackground: "#f0f0f0"
    readonly property string elementBackgroundColor: "#E6F3FF"  // Light blue for frames and text
    readonly property string nodeSelectionBoundsColor: "#0066cc"  // Blue for node multi-select bounding box
    
    // Node colors
    readonly property string nodeDefaultColor: "#E6F3FF"       // Light blue (same as element background)
    readonly property string nodeStartColor: "#E6F3FF"         // Light blue for start node
    readonly property string nodeProcessColor: "#E6F3FF"       // Light blue for process node
    
    // Node header colors by type
    readonly property string nodeHeaderEventColor: "#FF0000"      // Red for Event nodes
    readonly property string nodeHeaderOperationColor: "#0066FF"  // Blue for Operation nodes
    readonly property string nodeHeaderParamColor: "#000000"      // Black for Param nodes
    readonly property string nodeHeaderVariableColor: "#9C27B0"   // Purple for Variable nodes
    readonly property string nodeHeaderTextColor: "#FFFFFF"       // White text on headers
    
    // Edge colors
    readonly property string edgeFlowColor: "#666666"          // Dark gray for flow edges
    readonly property string edgeFlowSelectedColor: "#2196F3"  // Blue for selected flow edges
    readonly property string edgeVariableColor: "#FF9800"      // Orange for variable edges
    readonly property string edgeVariableSelectedColor: "#FF5722" // Darker orange for selected variable edges
    readonly property string edgePreviewColor: "#999999"       // Light gray for edge previews
    
    // Control colors
    readonly property color controlBarColor: Qt.rgba(1, 0, 0, 0.1)           // Red with 10% opacity
    readonly property color controlRotationJointColor: Qt.rgba(1, 0, 0, 0.1) // Red with 10% opacity
    readonly property color controlResizeJointColor: Qt.rgba(0, 0, 1, 0.1)   // Blue with 10% opacity
    readonly property color controlInnerRectColor: Qt.rgba(1, 0.8, 0, 0.05)  // Yellow with 5% opacity
    readonly property color controlBarLineColor: Qt.rgba(0, 0.4, 1, 1.0)     // Blue with 100% opacity
    readonly property color controlJointCircleFill: Qt.rgba(1, 1, 1, 1.0)    // White fill
    readonly property color controlJointCircleBorder: Qt.rgba(0, 0.4, 1, 1.0) // Blue border
    
    // Component instance colors (purple)
    readonly property color componentControlBarColor: Qt.rgba(0.5, 0, 0.5, 0.1)           // Purple with 10% opacity
    readonly property color componentControlRotationJointColor: Qt.rgba(0.5, 0, 0.5, 0.1) // Purple with 10% opacity
    readonly property color componentControlResizeJointColor: Qt.rgba(0.5, 0, 0.5, 0.1)   // Purple with 10% opacity
    readonly property color componentControlBarLineColor: Qt.rgba(0.5, 0, 0.5, 1.0)       // Purple with 100% opacity
    readonly property color componentControlJointCircleBorder: Qt.rgba(0.5, 0, 0.5, 1.0)  // Purple border
    
    // Element creation preview
    readonly property color elementCreationPreviewColor: Qt.rgba(0, 0.4, 1, 0.1)  // Blue with 10% opacity
    readonly property string elementCreationPreviewBorderColor: "#0066cc"          // Blue border
    readonly property int elementCreationPreviewBorderWidth: 2
    
    // Hover badge colors
    readonly property color hoverBadgeBackgroundColor: "#2196F3"  // Blue
    readonly property color hoverBadgeBorderColor: "#1976D2"      // Darker blue
    readonly property color hoverBadgeTextColor: "#FFFFFF"        // White
    readonly property string componentHoverBadgeBackgroundColor: "#7B1FA2"  // Purple
    readonly property string componentHoverBadgeBorderColor: "#6A1B9A"      // Darker purple
    
    // Sizes
    readonly property int defaultElementWidth: 200
    readonly property int defaultElementHeight: 150
    readonly property int selectionHandleSize: 8
    readonly property int controlMargin: 4
    
    // Node sizes
    readonly property int nodeMinHeight: 100
    readonly property int nodeBottomMargin: 10
    readonly property int nodeHeaderHeight: 30           // Height of node header
    readonly property int nodeHeaderTextSize: 12        // Font size for node header text
    readonly property int nodeHeaderPadding: 8           // Padding inside node header
    
    // Edge sizes
    readonly property int edgeFlowWidth: 3              // Width for flow edges
    readonly property int edgeFlowSelectedWidth: 4     // Width for selected flow edges
    readonly property int edgeVariableWidth: 2          // Width for variable edges
    readonly property int edgeVariableSelectedWidth: 3 // Width for selected variable edges
    readonly property int edgePreviewWidth: 2           // Width for edge previews
    
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
    readonly property int zHoverBadge: 4000
    
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