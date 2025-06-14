#pragma once

// Application configuration constants
namespace Config {
    // Colors
    constexpr const char* SELECTION_COLOR = "#0066cc";
    constexpr const char* HOVER_COLOR = "#999999";
    constexpr const char* CANVAS_BACKGROUND = "#f5f5f5";
    constexpr const char* PANEL_BACKGROUND = "#ffffff";
    constexpr const char* PANEL_HEADER_BACKGROUND = "#f0f0f0";
    constexpr const char* ELEMENT_BACKGROUND_COLOR = "#E6F3FF";  // Light blue for frames and text
    constexpr const char* NODE_SELECTION_BOUNDS_COLOR = "#0066cc";  // Blue for node multi-select bounding box
    
    // Node colors
    constexpr const char* NODE_DEFAULT_COLOR = "#E6F3FF";       // Light blue (same as element background)
    constexpr const char* NODE_START_COLOR = "#E6F3FF";         // Light blue for start node
    constexpr const char* NODE_PROCESS_COLOR = "#E6F3FF";       // Light blue for process node
    
    // Edge colors
    constexpr const char* EDGE_FLOW_COLOR = "#666666";          // Dark gray for flow edges
    constexpr const char* EDGE_FLOW_SELECTED_COLOR = "#2196F3"; // Blue for selected flow edges
    constexpr const char* EDGE_VARIABLE_COLOR = "#FF9800";      // Orange for variable edges
    constexpr const char* EDGE_VARIABLE_SELECTED_COLOR = "#FF5722"; // Darker orange for selected variable edges
    constexpr const char* EDGE_PREVIEW_COLOR = "#999999";       // Light gray for edge previews
    
    // Control colors
    constexpr const char* CONTROL_BAR_COLOR = "rgba(255, 0, 0, 0.1)";           // Red with 10% opacity
    constexpr const char* CONTROL_ROTATION_JOINT_COLOR = "rgba(255, 0, 0, 0.1)"; // Red with 10% opacity
    constexpr const char* CONTROL_RESIZE_JOINT_COLOR = "rgba(0, 0, 255, 0.1)";  // Blue with 10% opacity
    constexpr const char* CONTROL_INNER_RECT_COLOR = "rgba(255, 204, 0, 0.05)"; // Yellow with 5% opacity
    constexpr const char* CONTROL_BAR_LINE_COLOR = "rgba(0, 100, 255, 1.0)";    // Blue with 100% opacity
    constexpr const char* CONTROL_JOINT_CIRCLE_FILL = "rgba(255, 255, 255, 1.0)"; // White fill
    constexpr const char* CONTROL_JOINT_CIRCLE_BORDER = "rgba(0, 100, 255, 1.0)"; // Blue border
    
    // Hover badge colors
    constexpr const char* HOVER_BADGE_BACKGROUND_COLOR = "#2196F3";  // Blue
    constexpr const char* HOVER_BADGE_BORDER_COLOR = "#1976D2";      // Darker blue
    constexpr const char* HOVER_BADGE_TEXT_COLOR = "#FFFFFF";        // White
    
    // Sizes
    constexpr int DEFAULT_ELEMENT_WIDTH = 200;
    constexpr int DEFAULT_ELEMENT_HEIGHT = 150;
    constexpr int SELECTION_HANDLE_SIZE = 8;
    constexpr int CONTROL_MARGIN = 4;
    
    // Node sizes
    constexpr int NODE_MIN_HEIGHT = 100;
    constexpr int NODE_BOTTOM_MARGIN = 10;
    
    // Edge sizes
    constexpr int EDGE_FLOW_WIDTH = 3;              // Width for flow edges
    constexpr int EDGE_FLOW_SELECTED_WIDTH = 4;     // Width for selected flow edges
    constexpr int EDGE_VARIABLE_WIDTH = 2;          // Width for variable edges
    constexpr int EDGE_VARIABLE_SELECTED_WIDTH = 3; // Width for selected variable edges
    constexpr int EDGE_PREVIEW_WIDTH = 2;           // Width for edge previews
    
    // Control sizes
    constexpr int CONTROL_BAR_WIDTH = 10;
    constexpr int CONTROL_BAR_HEIGHT = 10;
    constexpr int CONTROL_ROTATION_JOINT_SIZE = 20;  // Blue rotation joints (not functional yet)
    constexpr int CONTROL_RESIZE_JOINT_SIZE = 10;    // Yellow resize joints
    constexpr int CONTROL_LINE_WIDTH = 1;            // Center lines in bars
    constexpr int CONTROL_JOINT_OVERLAP = 10;        // How much joints overlap with bars
    
    // Z-index layers
    constexpr int Z_BACKGROUND = 0;
    constexpr int Z_ELEMENTS = 10;
    constexpr int Z_CONTROLS = 1000;
    constexpr int Z_SELECTION = 2000;
    constexpr int Z_PANELS = 3000;
    constexpr int Z_HOVER_BADGE = 4000;
    
    // Canvas
    constexpr int CANVAS_WIDTH = 4000;
    constexpr int CANVAS_HEIGHT = 4000;
    constexpr qreal MIN_ZOOM = 0.1;
    constexpr qreal MAX_ZOOM = 5.0;
    constexpr qreal ZOOM_STEP = 0.1;
    
    // Animation
    constexpr int ANIMATION_DURATION = 150;
    
    // Performance
    constexpr int FPS_UPDATE_INTERVAL = 1000; // milliseconds
}