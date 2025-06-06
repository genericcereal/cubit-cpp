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
    }
    
    // Scene dimensions
    namespace Scene {
        constexpr int MIN_X = -2000;
        constexpr int MIN_Y = -2000;
        constexpr int WIDTH = 4000;
        constexpr int HEIGHT = 4000;
    }
}