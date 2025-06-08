#pragma once

// Application configuration constants
namespace Config {
    // Colors
    constexpr const char* SELECTION_COLOR = "#0066cc";
    constexpr const char* HOVER_COLOR = "#999999";
    constexpr const char* CANVAS_BACKGROUND = "#f5f5f5";
    constexpr const char* PANEL_BACKGROUND = "#ffffff";
    constexpr const char* PANEL_HEADER_BACKGROUND = "#f0f0f0";
    
    // Control colors
    constexpr const char* CONTROL_BAR_COLOR = "#0066cc";
    constexpr const char* CONTROL_ROTATION_JOINT_COLOR = "#0066cc";  // Blue rotation joints
    constexpr const char* CONTROL_RESIZE_JOINT_COLOR = "#ffcc00";    // Yellow resize joints
    constexpr const char* CONTROL_INNER_RECT_COLOR = "rgba(255, 204, 0, 0.05)";  // Yellow with 5% opacity
    
    // Sizes
    constexpr int DEFAULT_ELEMENT_WIDTH = 200;
    constexpr int DEFAULT_ELEMENT_HEIGHT = 150;
    constexpr int SELECTION_HANDLE_SIZE = 8;
    constexpr int CONTROL_MARGIN = 4;
    
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