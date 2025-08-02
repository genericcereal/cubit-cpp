#ifndef CONFIGOBJECT_H
#define CONFIGOBJECT_H

#include <QObject>
#include <QColor>
#include <QQmlEngine>
#include <QVariantList>
#include <QGuiApplication>
#include <QStyleHints>
#include "Config.h"

// Singleton to expose Config values to QML
class ConfigObject : public QObject
{
    Q_OBJECT
    QML_ELEMENT
    QML_SINGLETON
    
    // Dark mode (follows system setting)
    Q_PROPERTY(bool darkMode READ darkMode NOTIFY darkModeChanged)
    
    // Colors (now dynamic based on dark mode)
    Q_PROPERTY(QString selectionColor READ selectionColor NOTIFY darkModeChanged)
    Q_PROPERTY(QString hoverColor READ hoverColor NOTIFY darkModeChanged)
    Q_PROPERTY(QString canvasBackground READ canvasBackground NOTIFY darkModeChanged)
    Q_PROPERTY(QString panelBackground READ panelBackground NOTIFY darkModeChanged)
    Q_PROPERTY(QString panelHeaderBackground READ panelHeaderBackground NOTIFY darkModeChanged)
    Q_PROPERTY(QString elementBackgroundColor READ elementBackgroundColor NOTIFY darkModeChanged)
    Q_PROPERTY(QString nodeSelectionBoundsColor READ nodeSelectionBoundsColor NOTIFY darkModeChanged)
    Q_PROPERTY(QString textColor READ textColor NOTIFY darkModeChanged)
    Q_PROPERTY(QString actionsPanelBackground READ actionsPanelBackground NOTIFY darkModeChanged)
    Q_PROPERTY(QString listItemBackground READ listItemBackground NOTIFY darkModeChanged)
    Q_PROPERTY(QString listItemHover READ listItemHover NOTIFY darkModeChanged)
    Q_PROPERTY(QString listItemSelected READ listItemSelected NOTIFY darkModeChanged)
    Q_PROPERTY(QString expandBoxBackground READ expandBoxBackground NOTIFY darkModeChanged)
    Q_PROPERTY(QString expandBoxHover READ expandBoxHover NOTIFY darkModeChanged)
    Q_PROPERTY(QString expandBoxBorder READ expandBoxBorder NOTIFY darkModeChanged)
    Q_PROPERTY(QString expandBoxText READ expandBoxText NOTIFY darkModeChanged)
    Q_PROPERTY(QString secondaryTextColor READ secondaryTextColor NOTIFY darkModeChanged)
    Q_PROPERTY(QString dropIndicatorColor READ dropIndicatorColor NOTIFY darkModeChanged)
    
    // Project list colors
    Q_PROPERTY(QString projectCardBackground READ projectCardBackground NOTIFY darkModeChanged)
    Q_PROPERTY(QString projectCardBorder READ projectCardBorder NOTIFY darkModeChanged)
    Q_PROPERTY(QString projectCardHoverBorder READ projectCardHoverBorder NOTIFY darkModeChanged)
    Q_PROPERTY(QString projectThumbnailBackground READ projectThumbnailBackground NOTIFY darkModeChanged)
    
    // Button colors
    Q_PROPERTY(QString primaryButtonColor READ primaryButtonColor NOTIFY darkModeChanged)
    Q_PROPERTY(QString primaryButtonHover READ primaryButtonHover NOTIFY darkModeChanged)
    Q_PROPERTY(QString primaryButtonPressed READ primaryButtonPressed NOTIFY darkModeChanged)
    
    // Error colors
    Q_PROPERTY(QString errorBackground READ errorBackground NOTIFY darkModeChanged)
    Q_PROPERTY(QString errorBorder READ errorBorder NOTIFY darkModeChanged)
    Q_PROPERTY(QString errorText READ errorText NOTIFY darkModeChanged)
    
    // Delete button colors
    Q_PROPERTY(QString deleteButtonColor READ deleteButtonColor NOTIFY darkModeChanged)
    Q_PROPERTY(QString deleteButtonHover READ deleteButtonHover NOTIFY darkModeChanged)
    Q_PROPERTY(QString deleteButtonPressed READ deleteButtonPressed NOTIFY darkModeChanged)
    
    // Disabled state colors
    Q_PROPERTY(QString disabledBackground READ disabledBackground NOTIFY darkModeChanged)
    Q_PROPERTY(QString disabledText READ disabledText NOTIFY darkModeChanged)
    
    // Main text color
    Q_PROPERTY(QString mainTextColor READ mainTextColor NOTIFY darkModeChanged)
    
    // Node colors
    Q_PROPERTY(QString nodeDefaultColor READ nodeDefaultColor CONSTANT)
    Q_PROPERTY(QString nodeStartColor READ nodeStartColor CONSTANT)
    Q_PROPERTY(QString nodeProcessColor READ nodeProcessColor CONSTANT)
    
    // Node header colors by type
    Q_PROPERTY(QString nodeHeaderEventColor READ nodeHeaderEventColor CONSTANT)
    Q_PROPERTY(QString nodeHeaderOperationColor READ nodeHeaderOperationColor CONSTANT)
    Q_PROPERTY(QString nodeHeaderParamColor READ nodeHeaderParamColor CONSTANT)
    Q_PROPERTY(QString nodeHeaderVariableColor READ nodeHeaderVariableColor CONSTANT)
    Q_PROPERTY(QString nodeHeaderTextColor READ nodeHeaderTextColor CONSTANT)
    
    // Edge colors
    Q_PROPERTY(QString edgeFlowColor READ edgeFlowColor CONSTANT)
    Q_PROPERTY(QString edgeFlowSelectedColor READ edgeFlowSelectedColor CONSTANT)
    Q_PROPERTY(QString edgeVariableColor READ edgeVariableColor CONSTANT)
    Q_PROPERTY(QString edgeVariableSelectedColor READ edgeVariableSelectedColor CONSTANT)
    Q_PROPERTY(QString edgePreviewColor READ edgePreviewColor CONSTANT)
    
    // Control colors
    Q_PROPERTY(QColor controlBarColor READ controlBarColor CONSTANT)
    Q_PROPERTY(QColor controlRotationJointColor READ controlRotationJointColor CONSTANT)
    Q_PROPERTY(QColor controlResizeJointColor READ controlResizeJointColor CONSTANT)
    Q_PROPERTY(QColor controlInnerRectColor READ controlInnerRectColor CONSTANT)
    Q_PROPERTY(QColor controlBarLineColor READ controlBarLineColor CONSTANT)
    Q_PROPERTY(QColor controlJointCircleFill READ controlJointCircleFill CONSTANT)
    Q_PROPERTY(QColor controlJointCircleBorder READ controlJointCircleBorder CONSTANT)
    Q_PROPERTY(QColor controlsBorderColor READ controlsBorderColor CONSTANT)
    Q_PROPERTY(QString controlsBorderColorString READ controlsBorderColorString CONSTANT)
    
    // Component instance colors (purple)
    Q_PROPERTY(QColor componentControlBarColor READ componentControlBarColor CONSTANT)
    Q_PROPERTY(QColor componentControlRotationJointColor READ componentControlRotationJointColor CONSTANT)
    Q_PROPERTY(QColor componentControlResizeJointColor READ componentControlResizeJointColor CONSTANT)
    Q_PROPERTY(QColor componentControlBarLineColor READ componentControlBarLineColor CONSTANT)
    Q_PROPERTY(QColor componentControlJointCircleBorder READ componentControlJointCircleBorder CONSTANT)
    Q_PROPERTY(QColor componentControlsBorderColor READ componentControlsBorderColor CONSTANT)
    Q_PROPERTY(QString componentControlsBorderColorString READ componentControlsBorderColorString CONSTANT)
    
    // Element creation preview
    Q_PROPERTY(QColor elementCreationPreviewColor READ elementCreationPreviewColor CONSTANT)
    Q_PROPERTY(QString elementCreationPreviewBorderColor READ elementCreationPreviewBorderColor CONSTANT)
    Q_PROPERTY(int elementCreationPreviewBorderWidth READ elementCreationPreviewBorderWidth CONSTANT)
    
    // Hover badge colors
    Q_PROPERTY(QColor hoverBadgeBackgroundColor READ hoverBadgeBackgroundColor CONSTANT)
    Q_PROPERTY(QColor hoverBadgeBorderColor READ hoverBadgeBorderColor CONSTANT)
    Q_PROPERTY(QColor hoverBadgeTextColor READ hoverBadgeTextColor CONSTANT)
    Q_PROPERTY(QColor componentHoverBadgeBackgroundColor READ componentHoverBadgeBackgroundColor CONSTANT)
    Q_PROPERTY(QColor componentHoverBadgeBorderColor READ componentHoverBadgeBorderColor CONSTANT)
    
    // Sizes
    Q_PROPERTY(int defaultElementWidth READ defaultElementWidth CONSTANT)
    Q_PROPERTY(int defaultElementHeight READ defaultElementHeight CONSTANT)
    Q_PROPERTY(int selectionHandleSize READ selectionHandleSize CONSTANT)
    Q_PROPERTY(int controlMargin READ controlMargin CONSTANT)
    
    // Node sizes
    Q_PROPERTY(int nodeMinHeight READ nodeMinHeight CONSTANT)
    Q_PROPERTY(int nodeBottomMargin READ nodeBottomMargin CONSTANT)
    Q_PROPERTY(int nodeHeaderHeight READ nodeHeaderHeight CONSTANT)
    Q_PROPERTY(int nodeHeaderTextSize READ nodeHeaderTextSize CONSTANT)
    Q_PROPERTY(int nodeHeaderPadding READ nodeHeaderPadding CONSTANT)
    
    // Edge sizes
    Q_PROPERTY(int edgeFlowWidth READ edgeFlowWidth CONSTANT)
    Q_PROPERTY(int edgeFlowSelectedWidth READ edgeFlowSelectedWidth CONSTANT)
    Q_PROPERTY(int edgeVariableWidth READ edgeVariableWidth CONSTANT)
    Q_PROPERTY(int edgeVariableSelectedWidth READ edgeVariableSelectedWidth CONSTANT)
    Q_PROPERTY(int edgePreviewWidth READ edgePreviewWidth CONSTANT)
    
    // Control sizes
    Q_PROPERTY(int controlBarWidth READ controlBarWidth CONSTANT)
    Q_PROPERTY(int controlBarHeight READ controlBarHeight CONSTANT)
    Q_PROPERTY(int controlRotationJointSize READ controlRotationJointSize CONSTANT)
    Q_PROPERTY(int controlResizeJointSize READ controlResizeJointSize CONSTANT)
    Q_PROPERTY(int controlLineWidth READ controlLineWidth CONSTANT)
    Q_PROPERTY(int controlJointOverlap READ controlJointOverlap CONSTANT)
    
    // Z-index layers
    Q_PROPERTY(int zBackground READ zBackground CONSTANT)
    Q_PROPERTY(int zElements READ zElements CONSTANT)
    Q_PROPERTY(int zControls READ zControls CONSTANT)
    Q_PROPERTY(int zSelection READ zSelection CONSTANT)
    Q_PROPERTY(int zPanels READ zPanels CONSTANT)
    Q_PROPERTY(int zHoverBadge READ zHoverBadge CONSTANT)
    
    // Canvas
    Q_PROPERTY(int canvasWidth READ canvasWidth CONSTANT)
    Q_PROPERTY(int canvasHeight READ canvasHeight CONSTANT)
    Q_PROPERTY(qreal minZoom READ minZoom CONSTANT)
    Q_PROPERTY(qreal maxZoom READ maxZoom CONSTANT)
    Q_PROPERTY(qreal zoomStep READ zoomStep CONSTANT)
    
    // Animation
    Q_PROPERTY(int animationDuration READ animationDuration CONSTANT)
    
    // Performance
    Q_PROPERTY(int targetFps READ targetFps CONSTANT)
    Q_PROPERTY(int throttleInterval READ throttleInterval CONSTANT)
    Q_PROPERTY(int minFps READ minFps CONSTANT)
    Q_PROPERTY(int maxFps READ maxFps CONSTANT)
    Q_PROPERTY(bool adaptiveThrottlingDefault READ adaptiveThrottlingDefault CONSTANT)
    Q_PROPERTY(int zoomThrottleInterval READ zoomThrottleInterval CONSTANT)
    
    // Platform options
    Q_PROPERTY(QVariantList platformOptions READ platformOptions CONSTANT)
    
public:
    ConfigObject(QObject *parent = nullptr) : QObject(parent) {
        // Connect to system theme changes
        if (QGuiApplication::styleHints()) {
            connect(QGuiApplication::styleHints(), &QStyleHints::colorSchemeChanged,
                    this, &ConfigObject::darkModeChanged);
        }
    }
    
    // Dark mode getter (follows system setting)
    bool darkMode() const { 
        if (QGuiApplication::styleHints()) {
            return QGuiApplication::styleHints()->colorScheme() == Qt::ColorScheme::Dark;
        }
        return false;
    }
    
    // Color getters (now conditional on dark mode)
    QString selectionColor() const { 
        return darkMode() ? Config::DARK_SELECTION_COLOR : Config::SELECTION_COLOR; 
    }
    QString hoverColor() const { 
        return darkMode() ? Config::DARK_HOVER_COLOR : Config::HOVER_COLOR; 
    }
    QString canvasBackground() const { 
        return darkMode() ? Config::DARK_CANVAS_BACKGROUND : Config::CANVAS_BACKGROUND; 
    }
    QString panelBackground() const { 
        return darkMode() ? Config::DARK_PANEL_BACKGROUND : Config::PANEL_BACKGROUND; 
    }
    QString panelHeaderBackground() const { 
        return darkMode() ? Config::DARK_PANEL_HEADER_BACKGROUND : Config::PANEL_HEADER_BACKGROUND; 
    }
    QString elementBackgroundColor() const { 
        return darkMode() ? Config::DARK_ELEMENT_BACKGROUND_COLOR : Config::ELEMENT_BACKGROUND_COLOR; 
    }
    QString nodeSelectionBoundsColor() const { 
        return darkMode() ? Config::DARK_NODE_SELECTION_BOUNDS_COLOR : Config::NODE_SELECTION_BOUNDS_COLOR; 
    }
    QString textColor() const { 
        return darkMode() ? Config::DARK_TEXT_COLOR : Config::TEXT_COLOR; 
    }
    QString actionsPanelBackground() const { 
        return darkMode() ? Config::DARK_ACTIONS_PANEL_BACKGROUND : Config::ACTIONS_PANEL_BACKGROUND; 
    }
    QString listItemBackground() const { 
        return darkMode() ? Config::DARK_LIST_ITEM_BACKGROUND : Config::LIST_ITEM_BACKGROUND; 
    }
    QString listItemHover() const { 
        return darkMode() ? Config::DARK_LIST_ITEM_HOVER : Config::LIST_ITEM_HOVER; 
    }
    QString listItemSelected() const { 
        return darkMode() ? Config::DARK_LIST_ITEM_SELECTED : Config::LIST_ITEM_SELECTED; 
    }
    QString expandBoxBackground() const { 
        return darkMode() ? Config::DARK_EXPAND_BOX_BACKGROUND : Config::EXPAND_BOX_BACKGROUND; 
    }
    QString expandBoxHover() const { 
        return darkMode() ? Config::DARK_EXPAND_BOX_HOVER : Config::EXPAND_BOX_HOVER; 
    }
    QString expandBoxBorder() const { 
        return darkMode() ? Config::DARK_EXPAND_BOX_BORDER : Config::EXPAND_BOX_BORDER; 
    }
    QString expandBoxText() const { 
        return darkMode() ? Config::DARK_EXPAND_BOX_TEXT : Config::EXPAND_BOX_TEXT; 
    }
    QString secondaryTextColor() const { 
        return darkMode() ? Config::DARK_SECONDARY_TEXT_COLOR : Config::SECONDARY_TEXT_COLOR; 
    }
    QString dropIndicatorColor() const { 
        return darkMode() ? Config::DARK_DROP_INDICATOR_COLOR : Config::DROP_INDICATOR_COLOR; 
    }
    
    // Project list colors
    QString projectCardBackground() const { 
        return darkMode() ? Config::DARK_PROJECT_CARD_BACKGROUND : Config::PROJECT_CARD_BACKGROUND; 
    }
    QString projectCardBorder() const { 
        return darkMode() ? Config::DARK_PROJECT_CARD_BORDER : Config::PROJECT_CARD_BORDER; 
    }
    QString projectCardHoverBorder() const { 
        return darkMode() ? Config::DARK_PROJECT_CARD_HOVER_BORDER : Config::PROJECT_CARD_HOVER_BORDER; 
    }
    QString projectThumbnailBackground() const { 
        return darkMode() ? Config::DARK_PROJECT_THUMBNAIL_BACKGROUND : Config::PROJECT_THUMBNAIL_BACKGROUND; 
    }
    
    // Button colors
    QString primaryButtonColor() const { 
        return darkMode() ? Config::DARK_PRIMARY_BUTTON_COLOR : Config::PRIMARY_BUTTON_COLOR; 
    }
    QString primaryButtonHover() const { 
        return darkMode() ? Config::DARK_PRIMARY_BUTTON_HOVER : Config::PRIMARY_BUTTON_HOVER; 
    }
    QString primaryButtonPressed() const { 
        return darkMode() ? Config::DARK_PRIMARY_BUTTON_PRESSED : Config::PRIMARY_BUTTON_PRESSED; 
    }
    
    // Error colors
    QString errorBackground() const { 
        return darkMode() ? Config::DARK_ERROR_BACKGROUND : Config::ERROR_BACKGROUND; 
    }
    QString errorBorder() const { 
        return darkMode() ? Config::DARK_ERROR_BORDER : Config::ERROR_BORDER; 
    }
    QString errorText() const { 
        return darkMode() ? Config::DARK_ERROR_TEXT : Config::ERROR_TEXT; 
    }
    
    // Delete button colors
    QString deleteButtonColor() const { 
        return darkMode() ? Config::DARK_DELETE_BUTTON_COLOR : Config::DELETE_BUTTON_COLOR; 
    }
    QString deleteButtonHover() const { 
        return darkMode() ? Config::DARK_DELETE_BUTTON_HOVER : Config::DELETE_BUTTON_HOVER; 
    }
    QString deleteButtonPressed() const { 
        return darkMode() ? Config::DARK_DELETE_BUTTON_PRESSED : Config::DELETE_BUTTON_PRESSED; 
    }
    
    // Disabled state colors
    QString disabledBackground() const { 
        return darkMode() ? Config::DARK_DISABLED_BACKGROUND : Config::DISABLED_BACKGROUND; 
    }
    QString disabledText() const { 
        return darkMode() ? Config::DARK_DISABLED_TEXT : Config::DISABLED_TEXT; 
    }
    
    // Main text color
    QString mainTextColor() const { 
        return darkMode() ? Config::DARK_MAIN_TEXT_COLOR : Config::MAIN_TEXT_COLOR; 
    }
    
    // Node colors
    QString nodeDefaultColor() const { return Config::NODE_DEFAULT_COLOR; }
    QString nodeStartColor() const { return Config::NODE_START_COLOR; }
    QString nodeProcessColor() const { return Config::NODE_PROCESS_COLOR; }
    
    // Node header colors by type
    QString nodeHeaderEventColor() const { return Config::NODE_HEADER_EVENT_COLOR; }
    QString nodeHeaderOperationColor() const { return Config::NODE_HEADER_OPERATION_COLOR; }
    QString nodeHeaderParamColor() const { return Config::NODE_HEADER_PARAM_COLOR; }
    QString nodeHeaderVariableColor() const { return Config::NODE_HEADER_VARIABLE_COLOR; }
    QString nodeHeaderTextColor() const { return Config::NODE_HEADER_TEXT_COLOR; }
    
    // Edge colors
    QString edgeFlowColor() const { return Config::EDGE_FLOW_COLOR; }
    QString edgeFlowSelectedColor() const { return Config::EDGE_FLOW_SELECTED_COLOR; }
    QString edgeVariableColor() const { return Config::EDGE_VARIABLE_COLOR; }
    QString edgeVariableSelectedColor() const { return Config::EDGE_VARIABLE_SELECTED_COLOR; }
    QString edgePreviewColor() const { return Config::EDGE_PREVIEW_COLOR; }
    
    // Control colors (return QColor for rgba values)
    QColor controlBarColor() const { return QColor(255, 0, 0, 0); }  // Completely transparent
    QColor controlRotationJointColor() const { return QColor(255, 0, 0, 0); }  // Completely transparent
    QColor controlResizeJointColor() const { return QColor(0, 0, 255, 0); }  // Completely transparent
    QColor controlInnerRectColor() const { return Qt::transparent; }
    QColor controlBarLineColor() const { return QColor(0, 100, 255, 255); }
    QColor controlJointCircleFill() const { return QColor(255, 255, 255, 255); }
    QColor controlJointCircleBorder() const { return QColor(0, 100, 255, 255); }
    QColor controlsBorderColor() const { return QColor(Config::CONTROLS_BORDER_COLOR); }
    QString controlsBorderColorString() const { return Config::CONTROLS_BORDER_COLOR; }
    
    // Component instance colors (purple)
    QColor componentControlBarColor() const { return QColor(128, 0, 128, 0); }  // Completely transparent
    QColor componentControlRotationJointColor() const { return QColor(128, 0, 128, 0); }  // Completely transparent
    QColor componentControlResizeJointColor() const { return QColor(128, 0, 128, 0); }  // Completely transparent
    QColor componentControlBarLineColor() const { return QColor(128, 0, 128, 255); }
    QColor componentControlJointCircleBorder() const { return QColor(128, 0, 128, 255); }
    QColor componentControlsBorderColor() const { return QColor(Config::COMPONENT_CONTROLS_BORDER_COLOR); }
    QString componentControlsBorderColorString() const { return Config::COMPONENT_CONTROLS_BORDER_COLOR; }
    
    // Element creation preview
    QColor elementCreationPreviewColor() const { return QColor(0, 100, 255, 25); }
    QString elementCreationPreviewBorderColor() const { return Config::ELEMENT_CREATION_PREVIEW_BORDER_COLOR; }
    int elementCreationPreviewBorderWidth() const { return Config::ELEMENT_CREATION_PREVIEW_BORDER_WIDTH; }
    
    // Hover badge colors
    QColor hoverBadgeBackgroundColor() const { return QColor(33, 150, 243, 255); }  // #2196F3
    QColor hoverBadgeBorderColor() const { return QColor(25, 118, 210, 255); }  // #1976D2
    QColor hoverBadgeTextColor() const { return QColor(255, 255, 255, 255); }
    QColor componentHoverBadgeBackgroundColor() const { return QColor(128, 0, 128, 204); }  // 80% opacity = 204/255
    QColor componentHoverBadgeBorderColor() const { return QColor(255, 255, 255, 51); }  // 20% opacity = 51/255
    
    // Sizes
    int defaultElementWidth() const { return Config::DEFAULT_ELEMENT_WIDTH; }
    int defaultElementHeight() const { return Config::DEFAULT_ELEMENT_HEIGHT; }
    int selectionHandleSize() const { return Config::SELECTION_HANDLE_SIZE; }
    int controlMargin() const { return Config::CONTROL_MARGIN; }
    
    // Node sizes
    int nodeMinHeight() const { return Config::NODE_MIN_HEIGHT; }
    int nodeBottomMargin() const { return Config::NODE_BOTTOM_MARGIN; }
    int nodeHeaderHeight() const { return Config::NODE_HEADER_HEIGHT; }
    int nodeHeaderTextSize() const { return Config::NODE_HEADER_TEXT_SIZE; }
    int nodeHeaderPadding() const { return Config::NODE_HEADER_PADDING; }
    
    // Edge sizes
    int edgeFlowWidth() const { return Config::EDGE_FLOW_WIDTH; }
    int edgeFlowSelectedWidth() const { return Config::EDGE_FLOW_SELECTED_WIDTH; }
    int edgeVariableWidth() const { return Config::EDGE_VARIABLE_WIDTH; }
    int edgeVariableSelectedWidth() const { return Config::EDGE_VARIABLE_SELECTED_WIDTH; }
    int edgePreviewWidth() const { return Config::EDGE_PREVIEW_WIDTH; }
    
    // Control sizes
    int controlBarWidth() const { return Config::CONTROL_BAR_WIDTH; }
    int controlBarHeight() const { return Config::CONTROL_BAR_HEIGHT; }
    int controlRotationJointSize() const { return Config::CONTROL_ROTATION_JOINT_SIZE; }
    int controlResizeJointSize() const { return Config::CONTROL_RESIZE_JOINT_SIZE; }
    int controlLineWidth() const { return Config::CONTROL_LINE_WIDTH; }
    int controlJointOverlap() const { return Config::CONTROL_JOINT_OVERLAP; }
    
    // Z-index layers
    int zBackground() const { return Config::Z_BACKGROUND; }
    int zElements() const { return Config::Z_ELEMENTS; }
    int zControls() const { return Config::Z_CONTROLS; }
    int zSelection() const { return Config::Z_SELECTION; }
    int zPanels() const { return Config::Z_PANELS; }
    int zHoverBadge() const { return Config::Z_HOVER_BADGE; }
    
    // Canvas
    int canvasWidth() const { return Config::CANVAS_WIDTH; }
    int canvasHeight() const { return Config::CANVAS_HEIGHT; }
    qreal minZoom() const { return Config::MIN_ZOOM; }
    qreal maxZoom() const { return Config::MAX_ZOOM; }
    qreal zoomStep() const { return Config::ZOOM_STEP; }
    
    // Animation
    int animationDuration() const { return Config::ANIMATION_DURATION; }
    
    // Performance
    int targetFps() const { return Config::TARGET_FPS; }
    int throttleInterval() const { return Config::THROTTLE_INTERVAL; }
    int minFps() const { return Config::MIN_FPS; }
    int maxFps() const { return Config::MAX_FPS; }
    bool adaptiveThrottlingDefault() const { return Config::ADAPTIVE_THROTTLING_DEFAULT; }
    int zoomThrottleInterval() const { return Config::ZOOM_THROTTLE_INTERVAL; }
    
    // Platform options
    QVariantList platformOptions() const {
        QVariantList options;
        for (int i = 0; i < Config::PLATFORM_OPTIONS_COUNT; ++i) {
            options.append(Config::PLATFORM_OPTIONS[i]);
        }
        return options;
    }

signals:
    void darkModeChanged();
};

#endif // CONFIGOBJECT_H