#pragma once

namespace Config {
    // Z-Index values for layering UI elements
    // Higher values appear on top of lower values
    namespace ZIndex {
        // Background layer
        constexpr int BACKGROUND = 0;
        
        // Canvas elements
        constexpr int FRAME = 10;          // Frame widgets
        constexpr int CLIENT_RECT = 11;    // ClientRect overlays (slightly above frames)
        constexpr int TEXT = 12;           // Text elements within frames
        
        // Interactive overlays
        constexpr int CONTROLS = 1000;     // Selection controls
        constexpr int SELECTION_BOX = 2000; // Drag selection box
        
        // Top-level UI panels
        constexpr int ACTIONS_PANEL = 3000; // Bottom action panel
        constexpr int FPS_WIDGET = 3000;    // FPS display widget
    }
    
    // Element sizing defaults
    namespace ElementDefaults {
        constexpr int INITIAL_SIZE = 5;     // Initial size when creating elements
        constexpr int MIN_WIDTH = 20;       // Minimum element width
        constexpr int MIN_HEIGHT = 20;      // Minimum element height
    }
    
    // Colors
    namespace Colors {
        // Frame colors
        constexpr int FRAME_R = 200;
        constexpr int FRAME_G = 200;
        constexpr int FRAME_B = 200;
        
        // Canvas background
        constexpr int CANVAS_BG_R = 242;
        constexpr int CANVAS_BG_G = 242;
        constexpr int CANVAS_BG_B = 242;
        
        // Selection box colors
        constexpr int SELECTION_BOX_R = 0;
        constexpr int SELECTION_BOX_G = 120;
        constexpr int SELECTION_BOX_B = 215;
        constexpr int SELECTION_BOX_ALPHA = 50;  // Transparency (0-255)
        constexpr int SELECTION_BOX_BORDER_ALPHA = 100;  // Border transparency
        
        // Hover indicator colors
        constexpr int HOVER_R = 0;
        constexpr int HOVER_G = 0;
        constexpr int HOVER_B = 255;  // Blue color for hover
        
        // Control colors
        constexpr int CONTROL_INNER_RECT_R = 255;
        constexpr int CONTROL_INNER_RECT_G = 255;
        constexpr int CONTROL_INNER_RECT_B = 0;
        constexpr int CONTROL_INNER_RECT_ALPHA = 13;  // 5% opacity
        
        constexpr int CONTROL_BAR_R = 255;
        constexpr int CONTROL_BAR_G = 0;
        constexpr int CONTROL_BAR_B = 0;
        constexpr int CONTROL_BAR_ALPHA = 128;  // 50% opacity
        
        constexpr int CONTROL_LINE_R = 0;
        constexpr int CONTROL_LINE_G = 0;
        constexpr int CONTROL_LINE_B = 0;
        constexpr int CONTROL_LINE_ALPHA = 128;  // 50% opacity
        
        constexpr int CONTROL_ROTATION_JOINT_R = 0;
        constexpr int CONTROL_ROTATION_JOINT_G = 0;
        constexpr int CONTROL_ROTATION_JOINT_B = 255;
        constexpr int CONTROL_ROTATION_JOINT_ALPHA = 128;  // 50% opacity
        
        constexpr int CONTROL_RESIZE_JOINT_R = 255;
        constexpr int CONTROL_RESIZE_JOINT_G = 255;
        constexpr int CONTROL_RESIZE_JOINT_B = 0;
        constexpr int CONTROL_RESIZE_JOINT_ALPHA = 128;  // 50% opacity
    }
    
    // Scene dimensions
    namespace Scene {
        constexpr int MIN_X = -2000;
        constexpr int MIN_Y = -2000;
        constexpr int WIDTH = 4000;
        constexpr int HEIGHT = 4000;
    }
}