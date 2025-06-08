#pragma once

// Application configuration constants
namespace Config {
    // Colors
    constexpr const char* SELECTION_COLOR = "#0066cc";
    constexpr const char* HOVER_COLOR = "#999999";
    constexpr const char* CANVAS_BACKGROUND = "#f5f5f5";
    constexpr const char* PANEL_BACKGROUND = "#ffffff";
    constexpr const char* PANEL_HEADER_BACKGROUND = "#f0f0f0";
    
    // Sizes
    constexpr int DEFAULT_ELEMENT_WIDTH = 200;
    constexpr int DEFAULT_ELEMENT_HEIGHT = 150;
    constexpr int SELECTION_HANDLE_SIZE = 8;
    constexpr int CONTROL_MARGIN = 4;
    
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