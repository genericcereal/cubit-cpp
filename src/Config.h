#pragma once

// Application configuration constants
namespace Config {
    // Light mode colors
    constexpr const char* SELECTION_COLOR = "#0066cc";
    constexpr const char* HOVER_COLOR = "#999999";
    constexpr const char* CANVAS_BACKGROUND = "#f5f5f5";
    constexpr const char* PANEL_BACKGROUND = "#ffffff";
    constexpr const char* PANEL_HEADER_BACKGROUND = "#f0f0f0";
    constexpr const char* ELEMENT_BACKGROUND_COLOR = "#E6F3FF";  // Light blue for frames and text
    constexpr const char* NODE_SELECTION_BOUNDS_COLOR = "#0066cc";  // Blue for node multi-select bounding box
    constexpr const char* TEXT_COLOR = "#333333";  // Dark gray for text
    
    // Dark mode colors
    constexpr const char* DARK_SELECTION_COLOR = "#4C9AFF";
    constexpr const char* DARK_HOVER_COLOR = "#666666";
    constexpr const char* DARK_CANVAS_BACKGROUND = "#1e1e1e";
    constexpr const char* DARK_PANEL_BACKGROUND = "#252526";
    constexpr const char* DARK_PANEL_HEADER_BACKGROUND = "#2d2d30";
    constexpr const char* DARK_ELEMENT_BACKGROUND_COLOR = "#264F78";  // Dark blue for frames and text
    constexpr const char* DARK_NODE_SELECTION_BOUNDS_COLOR = "#4C9AFF";  // Light blue for node multi-select bounding box
    constexpr const char* DARK_TEXT_COLOR = "#CCCCCC";  // Light gray for text
    
    // Actions panel colors
    constexpr const char* ACTIONS_PANEL_BACKGROUND = "rgba(0, 0, 0, 0.8)";  // Semi-transparent black
    constexpr const char* DARK_ACTIONS_PANEL_BACKGROUND = "rgba(32, 32, 32, 0.9)";  // Semi-transparent dark gray
    
    // List item colors
    constexpr const char* LIST_ITEM_BACKGROUND = "#ffffff";
    constexpr const char* LIST_ITEM_HOVER = "#f5f5f5";
    constexpr const char* LIST_ITEM_SELECTED = "#e3f2fd";
    constexpr const char* DARK_LIST_ITEM_BACKGROUND = "#2d2d30";
    constexpr const char* DARK_LIST_ITEM_HOVER = "#3e3e42";
    constexpr const char* DARK_LIST_ITEM_SELECTED = "#094771";
    
    // Expand box colors
    constexpr const char* EXPAND_BOX_BACKGROUND = "#f5f5f5";
    constexpr const char* EXPAND_BOX_HOVER = "#e0e0e0";
    constexpr const char* EXPAND_BOX_BORDER = "#cccccc";
    constexpr const char* EXPAND_BOX_TEXT = "#444444";
    constexpr const char* DARK_EXPAND_BOX_BACKGROUND = "#3e3e42";
    constexpr const char* DARK_EXPAND_BOX_HOVER = "#4e4e52";
    constexpr const char* DARK_EXPAND_BOX_BORDER = "#555555";
    constexpr const char* DARK_EXPAND_BOX_TEXT = "#cccccc";
    
    // Secondary text color
    constexpr const char* SECONDARY_TEXT_COLOR = "#666666";
    constexpr const char* DARK_SECONDARY_TEXT_COLOR = "#999999";
    
    // Drop indicator color
    constexpr const char* DROP_INDICATOR_COLOR = "#2196F3";
    constexpr const char* DARK_DROP_INDICATOR_COLOR = "#4C9AFF";
    
    // Project list colors
    constexpr const char* PROJECT_CARD_BACKGROUND = "#ffffff";
    constexpr const char* PROJECT_CARD_BORDER = "#E0E0E0";
    constexpr const char* PROJECT_CARD_HOVER_BORDER = "#1976FF";
    constexpr const char* PROJECT_THUMBNAIL_BACKGROUND = "#F0F0F0";
    constexpr const char* DARK_PROJECT_CARD_BACKGROUND = "#2d2d30";
    constexpr const char* DARK_PROJECT_CARD_BORDER = "#444444";
    constexpr const char* DARK_PROJECT_CARD_HOVER_BORDER = "#4C9AFF";
    constexpr const char* DARK_PROJECT_THUMBNAIL_BACKGROUND = "#1e1e1e";
    
    // Button colors
    constexpr const char* PRIMARY_BUTTON_COLOR = "#1976FF";
    constexpr const char* PRIMARY_BUTTON_HOVER = "#3385FF";
    constexpr const char* PRIMARY_BUTTON_PRESSED = "#0066FF";
    constexpr const char* DARK_PRIMARY_BUTTON_COLOR = "#4C9AFF";
    constexpr const char* DARK_PRIMARY_BUTTON_HOVER = "#5DA8FF";
    constexpr const char* DARK_PRIMARY_BUTTON_PRESSED = "#3D8AFF";
    
    // Error colors
    constexpr const char* ERROR_BACKGROUND = "#FFEBEE";
    constexpr const char* ERROR_BORDER = "#F44336";
    constexpr const char* ERROR_TEXT = "#D32F2F";
    constexpr const char* DARK_ERROR_BACKGROUND = "#3D1F1F";
    constexpr const char* DARK_ERROR_BORDER = "#CF6679";
    constexpr const char* DARK_ERROR_TEXT = "#CF6679";
    
    // Delete button colors
    constexpr const char* DELETE_BUTTON_COLOR = "#FF5555";
    constexpr const char* DELETE_BUTTON_HOVER = "#FF3333";
    constexpr const char* DELETE_BUTTON_PRESSED = "#CC0000";
    constexpr const char* DARK_DELETE_BUTTON_COLOR = "#CF6679";
    constexpr const char* DARK_DELETE_BUTTON_HOVER = "#E57373";
    constexpr const char* DARK_DELETE_BUTTON_PRESSED = "#D32F2F";
    
    // Disabled state colors
    constexpr const char* DISABLED_BACKGROUND = "#CCCCCC";
    constexpr const char* DISABLED_TEXT = "#666666";
    constexpr const char* DARK_DISABLED_BACKGROUND = "#3e3e42";
    constexpr const char* DARK_DISABLED_TEXT = "#999999";
    
    // Main text color (different from general text color)
    constexpr const char* MAIN_TEXT_COLOR = "#1A1A1A";
    constexpr const char* DARK_MAIN_TEXT_COLOR = "#E0E0E0";
    
    // Node colors
    constexpr const char* NODE_DEFAULT_COLOR = "#E6F3FF";       // Light blue (same as element background)
    constexpr const char* NODE_START_COLOR = "#E6F3FF";         // Light blue for start node
    constexpr const char* NODE_PROCESS_COLOR = "#E6F3FF";       // Light blue for process node
    
    // Node header colors by type
    constexpr const char* NODE_HEADER_EVENT_COLOR = "#FF0000";      // Red for Event nodes
    constexpr const char* NODE_HEADER_OPERATION_COLOR = "#0066FF";  // Blue for Operation nodes
    constexpr const char* NODE_HEADER_PARAM_COLOR = "#000000";      // Black for Param nodes
    constexpr const char* NODE_HEADER_VARIABLE_COLOR = "#9C27B0";   // Purple for Variable nodes
    constexpr const char* NODE_HEADER_TEXT_COLOR = "#FFFFFF";       // White text on headers
    
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
    constexpr const char* CONTROL_INNER_RECT_COLOR = "rgba(255, 204, 0, 0.0)"; // Yellow with 0% opacity
    constexpr const char* CONTROL_BAR_LINE_COLOR = "rgba(0, 100, 255, 1.0)";    // Blue with 100% opacity
    constexpr const char* CONTROL_JOINT_CIRCLE_FILL = "rgba(255, 255, 255, 1.0)"; // White fill
    constexpr const char* CONTROL_JOINT_CIRCLE_BORDER = "rgba(0, 100, 255, 1.0)"; // Blue border
    constexpr const char* CONTROLS_BORDER_COLOR = "#0064FF";      // Blue for selection bounding box
    
    // Component instance colors (purple)
    constexpr const char* COMPONENT_CONTROL_BAR_COLOR = "rgba(128, 0, 128, 0.1)";           // Purple with 10% opacity
    constexpr const char* COMPONENT_CONTROL_ROTATION_JOINT_COLOR = "rgba(128, 0, 128, 0.1)"; // Purple with 10% opacity
    constexpr const char* COMPONENT_CONTROL_RESIZE_JOINT_COLOR = "rgba(128, 0, 128, 0.1)";   // Purple with 10% opacity
    constexpr const char* COMPONENT_CONTROL_BAR_LINE_COLOR = "rgba(128, 0, 128, 1.0)";       // Purple with 100% opacity
    constexpr const char* COMPONENT_CONTROL_JOINT_CIRCLE_BORDER = "rgba(128, 0, 128, 1.0)";  // Purple border
    constexpr const char* COMPONENT_CONTROLS_BORDER_COLOR = "#800080";      // Purple for component selection bounding box
    
    // Element creation preview
    constexpr const char* ELEMENT_CREATION_PREVIEW_COLOR = "rgba(0, 100, 255, 0.1)";  // Blue with 10% opacity
    constexpr const char* ELEMENT_CREATION_PREVIEW_BORDER_COLOR = "#0066cc";          // Blue border
    constexpr int ELEMENT_CREATION_PREVIEW_BORDER_WIDTH = 2;
    
    // Hover badge colors
    constexpr const char* HOVER_BADGE_BACKGROUND_COLOR = "#2196F3";  // Blue
    constexpr const char* HOVER_BADGE_BORDER_COLOR = "#1976D2";      // Darker blue
    constexpr const char* HOVER_BADGE_TEXT_COLOR = "#FFFFFF";        // White
    constexpr const char* COMPONENT_HOVER_BADGE_BACKGROUND_COLOR = "#7B1FA2";  // Purple
    constexpr const char* COMPONENT_HOVER_BADGE_BORDER_COLOR = "#6A1B9A";      // Darker purple
    
    // Sizes
    constexpr int DEFAULT_ELEMENT_WIDTH = 200;
    constexpr int DEFAULT_ELEMENT_HEIGHT = 150;
    constexpr int SELECTION_HANDLE_SIZE = 8;
    constexpr int CONTROL_MARGIN = 4;
    
    // Node sizes
    constexpr int NODE_MIN_HEIGHT = 100;
    constexpr int NODE_BOTTOM_MARGIN = 10;
    constexpr int NODE_HEADER_HEIGHT = 30;           // Height of node header
    constexpr int NODE_HEADER_TEXT_SIZE = 12;        // Font size for node header text
    constexpr int NODE_HEADER_PADDING = 8;           // Padding inside node header
    
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
    constexpr int TARGET_FPS = 65;         // Target frame rate for adaptive throttling
    constexpr int THROTTLE_INTERVAL = 1000 / TARGET_FPS;  // Calculate interval from target FPS
    constexpr int MIN_FPS = 30;            // Minimum acceptable FPS
    constexpr int MAX_FPS = 120;           // Maximum FPS cap
    constexpr bool ADAPTIVE_THROTTLING_DEFAULT = true;  // Enable adaptive throttling by default
    
    // API URLs
    constexpr const char* TOOL_REGISTRY_URL = "https://k72mo3oun7sefawjhvilq2ne5a0ybfgr.lambda-url.us-west-2.on.aws/";
    constexpr const char* GRAPHQL_ENDPOINT = "https://enrxjqdgdvc7pkragdib6abhyy.appsync-api.us-west-2.amazonaws.com/graphql";
    
    // Authentication
    constexpr const char* COGNITO_DOMAIN = "https://us-west-2jequuy6sn.auth.us-west-2.amazoncognito.com";
    constexpr const char* COGNITO_CLIENT_ID = "2l40nv8rncp41ur8ql5httl8ni";
    constexpr const char* COGNITO_REDIRECT_URI = "cubitapp://callback";
    constexpr const char* COGNITO_SCOPE = "email openid phone";
    
    // Platform options
    constexpr const char* PLATFORM_OPTIONS[] = {"web", "ios", "android"};
    constexpr int PLATFORM_OPTIONS_COUNT = 3;
    
    // Additional UI configuration
    constexpr int ZOOM_THROTTLE_INTERVAL = 8;  // 120fps throttling for zoom updates (smoother feel)
}