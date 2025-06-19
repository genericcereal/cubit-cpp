# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Application Architecture

The application follows a hierarchical architecture with the Application class as the central manager:

### Application Class
- Central singleton that manages the entire application state
- Maintains an array of Canvas instances (each with its own CanvasController, SelectionManager, and ElementModel)
- Tracks the active canvas and provides access to its components
- Manages canvas view modes (design/script) - script canvas is a different view of the same data, not a separate canvas type
- Contains a Panels instance for UI panel management

### Canvas Structure
Each canvas instance contains:
- **CanvasController**: Handles canvas interactions and element creation
- **SelectionManager**: Manages element selection state
- **ElementModel**: QAbstractListModel providing element data to QML views
- **View Mode**: Either "design" or "script" - determines how elements are displayed

### Panels Management
- **Panels Class**: Manages visibility of UI panels (DetailPanel, ActionsPanel, FPSMonitor)
- Accessible via `Application.panels` in QML
- Provides toggle methods and visibility properties for each panel
- Panel states can be controlled programmatically from anywhere in the application

### QML Integration
- Application is registered as a singleton: `Application`
- Access active canvas components: `Application.activeController`, `Application.activeSelectionManager`, `Application.activeElementModel`
- Control panels: `Application.panels.detailPanelVisible`, `Application.panels.toggleDetailPanel()`, etc.
- Switch canvases: `Application.switchToCanvas(canvasId)`
- Change view modes: `Application.activeCanvasViewMode = "script"`

## Terminology

- **Selection Box/Rectangle**: The blue rectangle that appears when dragging to select multiple elements

## Building and Running

### Qt Quick Application (cubit-quick)

The main application is now built using Qt Quick/QML. The project file is `cubit-quick.pro`.

#### Build Commands

```bash
# Clean the project
make clean

# Generate Makefile from project file
qmake6 -o Makefile cubit-quick.pro

# Build the project
make -j8

# Run the application on Mac
./cubit-quick.app/Contents/MacOS/cubit-quick
# or simply
open cubit-quick.app


# Run the application on WSL
./cubit-quick
```

#### Resource Files

All QML files must be added to `qml.qrc` to be included in the build. When adding new QML components:

1. Add the file path to `qml.qrc`
2. Run `make` to rebuild with the updated resources

#### Build Artifacts and Generated Files

**Important:** Qt's build system generates several files automatically. Do not manually duplicate or rename these files:

- `cubit-quick_metatypes.json` - Qt metatype information (auto-generated)
- `cubit-quick_qmltyperegistrations.cpp` - QML type registration code (auto-generated)
- `moc_*.cpp` and `moc_*.o` files - Qt's Meta-Object Compiler output
- `.o` object files in release/ and debug/ directories

If you see duplicate files with numbers appended (e.g., "file 2.o", "file 3.o"), these are build system artifacts that should not be committed to version control. Clean the build directory with `make clean` to remove them.

### CONTROLS

The controls system is designed to move, resize, and rotate the current selection. It should be implemented in the viewport overlay - the controls surface should scale with canvas zooms, but the individual control elements (bars, joints) should always stay a fixed size.

## Controls Implementation Summary

The controls system consists of:

**Visual Components:**

- **Surface**: Transparent yellow (0.05 opacity) bounding box for move operations
- **Bars**: 4 edge handles, 10px wide/tall, transparent red (0.2 opacity), overlap surface by 5px
- **Resize Joints**: 4 corner handles, 10x10px yellow squares (0.2 opacity)
- **Rotation Joints**: 4 corner handles, 30x30px blue squares (0.2 opacity), positioned behind resize joints

**Key Behaviors:**

- **Move**: Drag surface to move selection
- **Resize**: Drag bars/joints with dynamic transform origin (opposite edge becomes anchor)
- **Rotate**: Drag rotation joints to rotate around center
- **Click-to-select**: Click surface with multiple selections to select single element beneath cursor

**Technical Implementation:**

- Uses rotation-aware coordinate transformation (viewport → local control space)
- Index-based elements (0-3) instead of hardcoded positions
- Handles edge flipping for both single and multiple selections
- Maintains minimum 1x1 pixel size during resize
- Components scale with canvas zoom, but individual control elements maintain fixed size
- Implemented in Controls.qml within ViewportOverlay.qml

## Control Components

Control components consist of bars, joints, and a surface. Controls should ONLY be displayed when there's an active selection or during frame creation.

- **Bars**: The rectangles around selected elements/frames that can be dragged to resize the selection. Bars should be 10px wide, 100% height for vertical bars, and 10px tall, 100% wide for horizontal bars. Should be transparent red (0.2 opacity). Bars should overlap with the selection by 5px (so that 5px overlaps with the Control Surface, and 5px overlaps on the canvas)
- **Joints**:
  - **Resize joints** Corner handles that appear at the intersection of bars. Should be a 10px square and yellow (0.2 opacity), fits at the intersection of a vertical, horizontal joint.
  - **Rotation joints** Rotate the selection. When a rotation operation occurs, the controls should follow the dragged rotation joint. Should appear behind resize joints, and should be larger - 30px square and blue (0.2 opacity)
- **Surface**: The bounding box of all selected elements. Should be transparent yellow (0.2 opacity).

## Move Behavior

- **General behavior**: When the user drags the control surface, the surface should follow the cursor and the selection moves with the control surface
- **Click to select**: When a user clicks on the control surface when multiple elements are selected, the item immediately below the users cursor should become single selected. We need to make a distinction between drag and click for this use case.

## Resize Behavior

- **Single Selection**: Direct manipulation of the element
- **Multiple Selection**: Proportional scaling of all selected elements
- **Edge Flipping**: When a bar is dragged past its opposite bar (or a resize joint is dragged past an opposide edge, corner):
  - Single selection: The element flips and continues resizing
  - Multiple selection: All elements maintain their relative positions but flip as a group
- **Transform Origin**: The transform origin should update to be the opposite of the active bar. For example, if the right bar is dragged, the transform origin of the controls should be on the left left. The same logic should apply to resize joints - if the top left corner is dragged, the bottom right corner should be the transform origin.
- **Bars and joints functionally depend on rotation**: Controls use rotation-aware logic where bars and joints adapt their behavior based on the current rotation angle. For example, if the controls are rotated 180 degrees, the top bar will functionally become the bottom bar. This is implemented using coordinate transformation matrices to convert mouse deltas to local control space.

## Rotation Behavior

1. **Rotation-Aware Implementation**: Controls use coordinate transformation to handle rotation properly:
   - Mouse deltas are transformed from viewport space to local control space using rotation matrices
   - Edge and corner indices are used instead of hardcoded "top/bottom/left/right" logic
   - Mouse cursors dynamically adjust based on the effective edge/corner after rotation
2. **Unified Behavior**: Single selection is treated as a special case of multiple selection for consistency
3. **Transformation Pipeline**: When resizing or rotating:
   - Mouse movements are captured in viewport/parent coordinates
   - Deltas are transformed to local control space: `localDelta = rotate(viewportDelta, -controlRotation)`
   - Resize operations are applied in local space
   - Position adjustments account for rotation to maintain proper anchor points

## Controls Implementation Notes

- Mouse interactions are handled in viewport coordinates and converted to canvas coordinates
- Minimum element size of 1x1 pixels is enforced during all resize operations (bars can overlap)
- Use QML transformations as much as possible (for example, use Item.Center for transformOrigin during rotation)

## Controls Implementation Strategy

### Architecture Overview

The controls system is implemented as a separate Controls.qml component that manages all control elements (surface, bars, and joints). This component uses rotation-aware logic where all resize and movement operations are transformed to local control space, ensuring that bars and joints behave correctly regardless of rotation angle. The implementation uses:

- **Index-based elements**: Bars and joints are created using Repeaters with indices (0-3) rather than named positions
- **Coordinate transformation**: Mouse deltas are transformed from viewport to local space using rotation matrices
- **Dynamic cursors**: Mouse cursors adjust based on the effective edge/corner after rotation
- **Unified resize logic**: All bars use the same resize logic pattern, with behavior determined by their index

### Component Structure

1. **ControlsContainer** (in ViewportOverlay.qml)

   - Main container that holds all control elements
   - Manages control state (position, size, rotation)
   - Handles coordinate transformations between viewport and canvas space
   - Implements pause/resume for position bindings during drag operations

2. **Control Surface**

   - Rectangle with transparent yellow (0.05 opacity) fill
   - Handles move operations when dragged
   - Implements click-to-select behavior for multiple selections
   - MouseArea covers entire surface area

3. **Control Bars** (4 instances)

   - Rectangles positioned at edges of control surface
   - 10px wide/tall with transparent red (0.2 opacity) fill
   - Overlap control surface by 5px on each side
   - Each bar has its own MouseArea for drag handling
   - Dynamic transform origin based on which bar is being dragged

4. **Control Joints** (8 instances total)
   - **Resize Joints** (4): 10x10px yellow squares at corners
   - **Rotation Joints** (4): 30x30px blue squares behind resize joints
   - Positioned at corners of control surface
   - Resize joints handle proportional scaling
   - Rotation joints handle rotation around center

### Key Technical Considerations

1. **Coordinate Systems**

   - All mouse positions must be converted from viewport to canvas coordinates
   - Use viewportToCanvas() helper function consistently
   - Account for zoom level and scroll position

2. **State Management**

   - Use dragging flag to pause position bindings during operations
   - Maintain separate states for move, resize, and rotate operations
   - Ensure controls follow selection changes smoothly

3. **Performance**

   - Minimize property binding recalculations during drag
   - Use Item transformations instead of manual calculations where possible
   - Batch element updates to avoid multiple redraws

4. **Visual Hierarchy**
   - Z-ordering: Surface (bottom) → Rotation joints → Bars → Resize joints (top)
   - Ensure proper hit testing with overlapping components
   - Maintain consistent visual feedback across all operations

### Main Application Structure

- `main.cpp`: Entry point that sets up the main window with a splitter layout containing:
  - Left side: Canvas - The main drawing/interaction area
  - Right side: DetailPanel - A resizable panel with scrollable sections

### Canvas Element Architecture

The element system has been refactored to properly separate visual and non-visual elements:

#### Element Class Hierarchy:

```
Element (base class)
├── Properties: elementId, name, selected, parentId
├── No geometric properties
├── isVisual() method returns false by default
│
├── Variable (data element)
│   └── Properties: value
│   └── Non-visual: exists in the model but not displayed on canvas
│
└── CanvasElement (visual element base)
    ├── Properties: x, y, width, height
    ├── isVisual() returns true
    ├── containsPoint() method for hit testing
    ├── isDesignElement() returns false by default
    ├── isScriptElement() returns false by default
    │
    ├── DesignElement (design canvas elements)
    │   ├── isDesignElement() returns true
    │   │
    │   ├── Frame
    │   │   └── Properties: backgroundColor, borderColor, borderWidth, borderRadius, overflow
    │   │
    │   ├── Text
    │   │   └── Properties: text, font, color
    │   │
    │   └── Html
    │       └── Properties: html, url
    │
    └── ScriptElement (script canvas elements)
        ├── isScriptElement() returns true
        │
        ├── Node
        │   └── Properties: nodeTitle, nodeColor, inputPorts, outputPorts, etc.
        │
        └── Edge
            └── Properties: sourceNodeId, targetNodeId, sourcePoint, targetPoint, etc.
```

#### Key Design Principles:

1. **Element** is the base class containing only properties common to ALL elements
2. **CanvasElement** extends Element with geometric properties for visual elements
3. **DesignElement** extends CanvasElement for elements that belong on the design canvas (Frame, Text, Html)
4. **ScriptElement** extends CanvasElement for elements that belong on the script canvas (Node, Edge)
5. **Variable** inherits directly from Element since it has no visual representation
6. **Design elements** (Frame, Text, Html) inherit from DesignElement
7. **Script elements** (Node, Edge) inherit from ScriptElement

#### Element Creation and Management:

1. **Frames**: Visual frame elements that serve as containers
2. **Text**: Text display elements that are always contained within Frames
3. **Variables**: Non-visual elements that store data (created but not displayed on canvas)

#### Element Relationships:

- **Text elements are ALWAYS created within Frame containers**
- A Frame can contain multiple Text elements
- Text elements cannot exist independently on the Canvas
- Variables exist in the ElementModel but have no visual representation

#### Text Element Creation and Behavior:

- **Creation**: When using the Text tool, clicking and dragging creates a Frame with a Text element inside
- **Appearance**: Text appears at the top-left of its parent Frame with no padding
- **Sizing**: Text elements maintain their original size when the parent Frame is resized
- **Truncation**: When the Frame becomes too small, text is truncated (ellipsis feature planned but not yet working)
- **Selection**: The parent Frame is selected, not the Text element itself

#### Text Editing:

- **Activation**: Double-click on the controls when a single Frame containing Text is selected
- **Edit Mode**: A QLineEdit appears over the text for in-place editing
- **Save**: Press Enter to save changes
- **Cancel**: Press Escape to cancel editing
- **Auto-save**: Clicking outside the edit field saves changes

#### Important Mouse Event Handling Rules:

- **Frames and Text elements do NOT handle mouse events directly**
- Each Frame has a corresponding **ClientRect** that handles all mouse interactions for the Frame and its contents
- ClientRects are transparent overlays positioned exactly over their associated Frame
- Text elements inherit mouse event handling from their parent Frame's ClientRect
- This separation ensures clean event handling and consistent interaction behavior

### UI Components

- **Canvas**: Main interactive area that manages elements and their interactions
- **DetailPanel**: Resizable side panel with multiple scrollable sections
- **ActionsPanel**: A fixed-size, semi-transparent panel positioned at the bottom of the main window
- **FPSWidget**: Performance monitoring widget positioned at the top-right of the main window
- **Controls**: Resizable control handles that appear around selected Frame elements
- **ClientRect**: Transparent overlay widgets that handle mouse events for Frame and Text elements

### Component Relationships

```
+------------------+     +------------------+
| Main Window      |     |                  |
|  +-------------+ |     | Detail Panel     |
|  | Canvas      | |     | (QWidget)        |
|  | (QWidget)   | |     |                  |
|  |             | |     | - Element List   |
|  | - Frames    | |     | - Properties     |
|  | - Text      | |     | - Scrollable     |
|  | - ClientRects| |     |   sections       |
|  | - Controls  | |     |                  |
|  |             | |     |                  |
|  +-------------+ |     |                  |
|  | ActionsPanel| |     |                  |
|  | FPSWidget   | |     |                  |
+------------------+     +------------------+
```

## Development Workflow

The codebase follows a standard Qt development pattern:

- Qt5 Widgets for the UI components
- QMake for project configuration
- C++11 for the implementation
- Standard Q_OBJECT macro usage for Qt's meta-object system
- Qt's signal/slot mechanism for component communication

When creating new UI components:

1. Define a new header (.h) file with the component class
2. Implement the component in a .cpp file
3. Add the files to qt-hello.pro
4. Run qmake to regenerate the Makefile
5. Build with make. The make file is in /src, so if you're in the root directory you'll need to change directories.
6. Define style constants in Config.h so that style updates can be made centrally. Do not keep styles inline.

## Other instructions

- Never create "mock" implementations. Don't create files for testing purposes with the intention of throwing them away later.
- Eliminate all unused/stale methods, variables, etc. when you come across them. Don't leave unused code in the code base.
- Always look for Qts helpers before implementing something bespoke. For example, use QWidget::mouseDoubleClickEvent instead of creating double clicks with timers.
- DRY (Don't Repeat Yourself) code is incredibly important to this project. Where possible, modularize and re-use code. Point out redundancy and duplication when you come across it.
- Always use Qt::UniqueConnection when connecting signals to prevent duplicate connections. Example:
  ```cpp
  connect(element, &Element::geometryChanged,
          this, &SelectionManager::onElementGeometryChanged,
          Qt::UniqueConnection);
  ```

## Qt Quick Migration Plan

### Current Status

**Phase**: Phase 2 - Canvas Implementation
**Started**: January 2025
**Target Completion**: TBD
**Progress**: Phase 2 completed - Canvas interactions fully implemented

### Widget to Qt Quick Component Mapping

| Qt Widget Component   | Qt Quick Equivalent                                  | Key Considerations                                           |
| --------------------- | ---------------------------------------------------- | ------------------------------------------------------------ |
| QGraphicsView/Scene   | `Canvas` or custom `Item` with `MultiPointTouchArea` | Qt Quick has built-in scene graph, no need for QGraphicsView |
| QGraphicsProxyWidget  | Native QML components                                | Direct QML rendering, better performance                     |
| QWidget containers    | `Item`, `Rectangle`, `Flickable`                     | Declarative layout system                                    |
| QVBoxLayout/QSplitter | `ColumnLayout`, `SplitView`                          | QML layouts are more flexible                                |
| Custom painting       | `Canvas` item or custom `QQuickPaintedItem`          | For selection boxes and overlays                             |
| QWebEngineView        | `WebEngineView`                                      | Direct QML equivalent available                              |
| Signal/Slot           | QML signals and JavaScript                           | More concise syntax                                          |

### Proposed QML Architecture

```qml
ApplicationWindow {
    SplitView {
        // Left Panel - Canvas Area
        Item {
            CanvasView {
                // Main editing canvas
                ElementsLayer {}
                ControlsLayer {}
                SelectionLayer {}
            }

            // Overlays
            ActionsPanel {}
            FPSMonitor {}
        }

        // Right Panel
        DetailPanel {
            ElementList {}
            PropertiesPanel {}
        }
    }
}
```

### C++ Backend Strategy

**Core C++ Classes:**

- `Element` - Base class for all elements (properties: id, name, selected, parentId)
- `CanvasElement` - Base class for visual elements (adds: x, y, width, height)
- `DesignElement` - Base class for design canvas elements (inherits from CanvasElement)
- `ScriptElement` - Base class for script canvas elements (inherits from CanvasElement)
- `Variable` - Non-visual data element
- `Frame`, `Text`, `Html` - Visual design elements inheriting from DesignElement
- `Node`, `Edge` - Visual script elements inheriting from ScriptElement

**Controller Classes:**

- `ElementModel` - QAbstractListModel for QML ListView
- `CanvasController` - Handles interaction logic and element creation
- `SelectionManager` - Selection state management
- `ViewportCache` - Optimizes viewport calculations

**Application Architecture:**

- `Application` - Central singleton managing multiple canvases and application state
  - Manages array of Canvas instances
  - Tracks active canvas ID
  - Provides access to active canvas components (controller, selection manager, element model)
  - Manages canvas view modes (design/script)
  - Contains Panels instance for panel management
- `Panels` - Manages UI panel visibility states
  - Controls DetailPanel, ActionsPanel, and FPSMonitor visibility
  - Provides toggle methods for each panel
  - Accessible via Application.panels in QML

**QML Registration:**

```cpp
// Base classes (uncreatable)
qmlRegisterUncreatableType<Element>("Cubit", 1, 0, "Element", "Element is an abstract base class");
qmlRegisterUncreatableType<CanvasElement>("Cubit", 1, 0, "CanvasElement", "CanvasElement is an abstract base class");
qmlRegisterUncreatableType<DesignElement>("Cubit", 1, 0, "DesignElement", "DesignElement is an abstract base class");
qmlRegisterUncreatableType<ScriptElement>("Cubit", 1, 0, "ScriptElement", "ScriptElement is an abstract base class");

// Concrete element types
qmlRegisterType<Frame>("Cubit", 1, 0, "Frame");
qmlRegisterType<Text>("Cubit", 1, 0, "TextElement");
qmlRegisterType<Html>("Cubit", 1, 0, "Html");
qmlRegisterType<Variable>("Cubit", 1, 0, "Variable");
qmlRegisterType<Node>("Cubit", 1, 0, "Node");
qmlRegisterType<Edge>("Cubit", 1, 0, "Edge");

// Controllers
qmlRegisterType<CanvasController>("Cubit", 1, 0, "CanvasController");
qmlRegisterType<ElementModel>("Cubit", 1, 0, "ElementModel");
qmlRegisterType<SelectionManager>("Cubit", 1, 0, "SelectionManager");
qmlRegisterType<ViewportCache>("Cubit", 1, 0, "ViewportCache");
qmlRegisterType<Panels>("Cubit", 1, 0, "Panels");

// Application singleton
Application* appInstance = new Application(&app);
qmlRegisterSingletonType<Application>("Cubit", 1, 0, "Application",
    [appInstance](QQmlEngine *engine, QJSEngine *scriptEngine) -> QObject * {
        Q_UNUSED(engine)
        Q_UNUSED(scriptEngine)
        return appInstance;
    });
```

### Other Notes

- Only use the legacy folder when the user tells you to reference a previous implementation. Do not make edits to anything in the legacy folder.

## Performance Optimizations

### ViewportCache C++ Class

To reduce JavaScript calculations in QML, we've implemented a `ViewportCache` class that handles expensive viewport calculations in C++:

- **Cached viewport bounds**: Calculates viewport bounds once per frame update instead of per-element
- **Efficient coordinate transformations**: Provides optimized methods for converting between viewport and canvas coordinates
- **Element visibility management**: Determines which elements are visible in the viewport with a single calculation per frame
- **Reduced property bindings**: QML components simply use cached values instead of recalculating

### Component Architecture Refactoring

**CanvasView.qml has been split into smaller, focused components:**

1. **ElementLayer.qml**: Handles element rendering

   - Uses `Instantiator` instead of `Repeater + Loader` for better performance
   - Direct object creation without Loader overhead
   - Implements efficient visibility culling using ViewportCache
   - Tracks element lifecycle with `onObjectAdded`/`onObjectRemoved`

2. **InputController.qml**: Centralizes all mouse/keyboard interactions

   - Single MouseArea for all canvas interactions
   - Uses cached coordinate transformations from ViewportCache
   - Handles panning, zooming, selection, and element manipulation
   - **Throttled event handling**: Mouse move and wheel events are throttled to 60fps (16ms intervals) to reduce excessive calculations
   - Panning is not throttled to maintain smooth user experience

3. **OverlayLayers.qml**: Manages overlay elements
   - Creation preview rectangles
   - Other viewport-based overlays

### Touch Gesture Support

We've implemented native touch gesture handling using Qt's built-in handlers:

**PinchHandler:**

- Provides smooth pinch-to-zoom functionality for touch devices
- Maintains the pinch center point during zooming
- Integrates with ViewportCache for efficient coordinate transformations
- Automatically disables Flickable interaction during pinch gestures

**WheelHandler:**

- Replaces manual wheel event handling with Qt's native handler
- Supports both mouse wheels and touchpad gestures
- Throttled to 60fps for smooth performance
- Ctrl+Wheel for zooming with stable zoom point

**Native Panning:**

- Removed custom middle-mouse button panning code
- Flickable now handles both touch and mouse panning natively
- Better integration with touch devices and smoother performance

### Event Throttling

To improve performance during interactions, we've implemented event throttling:

**Throttled Events:**

- **Mouse Position Changed**: Limited to 60fps (16ms intervals) in InputController
- **Wheel Events**: Throttled to 60fps in WheelHandler to prevent excessive zoom calculations

**Implementation Details:**

- Uses QML Timer components with 16ms intervals
- Batches pending events and processes them at the next timer tick
- Reduces calls to `ViewportCache.viewportToCanvas()` from potentially hundreds per second to maximum 60fps

### Instantiator vs Repeater + Loader

We've replaced the `Repeater + Loader` pattern with `Instantiator` in ElementLayer.qml:

**Benefits:**

- No asynchronous loading delays
- Direct component instantiation
- Better memory management with explicit cleanup
- More control over object lifecycle
- Reduced overhead for large numbers of elements

**Implementation:**

- Uses `onObjectAdded`/`onObjectRemoved` for model-to-view wiring
- Maintains an `elementItems` map for quick element lookup by ID
- Proper cleanup in `Component.onDestruction`
- Dynamic property bindings for position, size, and visibility updates

## Canvas Architecture Improvements

### Coordinate Transformation Utilities

All coordinate math has been centralized in `CanvasUtils.js` as a JavaScript library:

```javascript
.pragma library

function viewportToCanvas(pt, contentX, contentY, zoom, minX, minY) {
    return Qt.point(
        (contentX + pt.x) / zoom + minX,
        (contentY + pt.y) / zoom + minY
    );
}

function canvasToViewport(pt, minX, minY, zoom) {
    return Qt.point(
        (pt.x - minX) * zoom,
        (pt.y - minY) * zoom
    );
}
```

### CanvasInputHandler Component

A dedicated input handling component (`CanvasInputHandler.qml`) provides a clean signal-based interface for all canvas interactions:

**Signals:**

- `clicked(point canvasPoint)` - Single click with no drag
- `dragStarted(point canvasPoint)` - Drag operation started
- `dragMoved(point canvasPoint)` - Mouse moved during drag
- `dragEnded(point canvasPoint)` - Drag operation ended
- `hovered(point canvasPoint)` - Mouse hover over canvas
- `panned(real dx, real dy)` - Middle mouse button pan
- `zoomed(point canvasPoint, real scaleFactor)` - Ctrl+wheel zoom

**Features:**

- Automatic coordinate transformation from viewport to canvas space
- Built-in distinction between clicks and drags
- Separate handling for panning vs selection operations
- Clean separation of concerns

### Simplified BaseCanvas API

`BaseCanvas.qml` now provides a simplified API that subclasses can easily override:

```qml
BaseCanvas {
    // Properties
    controller: /* canvas controller */
    selectionManager: /* selection manager */
    elementModel: /* element model */

    // Add content via default property
    contentData: [
        // Your canvas elements here
    ]

    // Override these handler functions
    function handleClick(pt) { }
    function handleDragStart(pt) { }
    function handleDragMove(pt) { }
    function handleDragEnd(pt) { }
    function handleHover(pt) { }
    function handleSelectionRect(rect) { }
}
```

**Key Improvements:**

- No need to handle raw mouse events
- Automatic coordinate transformation
- Built-in selection box handling
- Clean separation between base functionality and specific implementations
- Support for both DesignCanvas and ScriptCanvas with minimal code

### Implementation Pattern

Both `DesignCanvas.qml` and `ScriptCanvas.qml` follow this pattern:

1. Use `contentData` property to add canvas elements
2. Override handler functions for specific behavior
3. Let BaseCanvas handle all the complex input management
4. Focus on business logic rather than input handling

This architecture significantly reduces code duplication and makes the canvas implementations much more maintainable.

## Node System

### Node Types

The script canvas supports three types of nodes, each with distinct visual and functional characteristics:

1. **Event Nodes** (Red header)
   - Trigger actions in response to events
   - Examples: User Input, On Load
   - Typically have output flow ports to initiate execution chains

2. **Operation Nodes** (Blue header)
   - Perform actions or transformations
   - Examples: HTTP Request, Math Operation, Function, Display Output
   - Usually have both input and output ports for data flow

3. **Param Nodes** (Black header)
   - Control flow or store/manage data
   - Examples: Variable, Condition, Loop
   - May have special behavior like conditional branching or iteration

### Node Header Component

All nodes display a colored header (`NodeHeader.qml`) that:
- Shows the node name with left-aligned text
- Uses a background color based on node type (red/blue/black)
- Has a fixed height of 30px with 12px text
- White text color for contrast on all header types

### Node Type Property

- The `nodeType` property is read-only and set during node creation
- Node types cannot be changed after creation
- The type is specified in the NodeCatalog and passed through the creation pipeline
- Default type is "Operation" if not specified

## Scripts Class

### Overview

The `Scripts` class manages collections of nodes and edges for visual programming functionality. It's used as a property in both Canvas instances and DesignElement objects.

### Features

- **Node Management**: Add, remove, clear, and query nodes
- **Edge Management**: Add, remove, clear, and query edges
- **Relationship Queries**: Find edges connected to specific nodes (incoming, outgoing, or both)
- **QML Integration**: Exposed as QML list properties for easy binding
- **Automatic Cleanup**: Removes connected edges when nodes are deleted

### Default Initialization

The Scripts class automatically initializes with default nodes when created:
- **onLoad Node**: An event node positioned at (400, 200) with a single "Done" output port
- Additional default nodes can be added in the `loadInitialNodes()` method

### Usage

**In C++:**
```cpp
Scripts* scripts = new Scripts(parent);
Node* node = new Node(id, scripts);
scripts->addNode(node);
```

**In QML:**
```qml
// Access scripts from a Canvas
Application.activeCanvas.scripts.nodes
Application.activeCanvas.scripts.edges

// Access scripts from a DesignElement
frameElement.scripts.nodes
frameElement.scripts.edges
```

## Console Messages

### ConsoleMessageRepository

All console messages must be added to the centralized `ConsoleMessageRepository` singleton instead of being created inline. This ensures consistent message handling and logging across the application.

**Usage in C++:**
```cpp
#include "ConsoleMessageRepository.h"

// Add different types of messages
ConsoleMessageRepository::instance()->addOutput("Operation completed");
ConsoleMessageRepository::instance()->addError("Failed to load file");
ConsoleMessageRepository::instance()->addWarning("Configuration not found");
ConsoleMessageRepository::instance()->addInfo("Application started");
ConsoleMessageRepository::instance()->addInput("user command");
```

**Usage in QML:**
```qml
import Cubit

// Access the singleton
ConsoleMessageRepository.addOutput("Message from QML")
ConsoleMessageRepository.addError("Error from QML")
```

**Message Types:**
- `Input`: User commands entered in the console
- `Output`: General output messages
- `Error`: Error messages (also logged to qCritical)
- `Warning`: Warning messages (also logged to qWarning)
- `Info`: Informational messages

## Command System (Undo/Redo)

### Overview

The application implements the Command Pattern for comprehensive undo/redo support. Every user action that modifies the canvas state (create, delete, move, resize, etc.) is wrapped in a command object that knows how to execute and undo itself.

### Architecture

**Base Classes:**

- `Command` - Abstract base class for all commands
  - `execute()` - Performs the action
  - `undo()` - Reverses the action
  - `redo()` - Re-executes the action (default implementation calls execute())
  - `description()` - Returns a human-readable description of the action

- `CommandHistory` - Manages undo/redo stacks
  - Maintains undo and redo stacks using `std::stack<std::unique_ptr<Command>>`
  - Tracks command execution state
  - Provides `canUndo()`/`canRedo()` state for UI
  - Emits signals for UI updates
  - Configurable maximum undo count (default: 100)

### Implemented Commands

1. **CreateFrameCommand**
   - Creates a new Frame element
   - Stores the frame for undo/redo operations
   - Manages selection state

2. **DeleteElementsCommand**
   - Deletes one or more elements
   - Preserves element data for restoration
   - Handles parent-child relationships (future support)
   - Restores selection on undo

3. **MoveElementsCommand**
   - Moves selected elements by a delta
   - Supports merging consecutive moves (drag operations)
   - Efficient handling of multi-element moves

4. **ResizeElementCommand**
   - Resizes a single element
   - Supports merging consecutive resizes
   - Preserves original dimensions

5. **SetPropertyCommand**
   - Generic property changes using Qt's property system
   - Supports merging consecutive property changes
   - Works with any QObject property

### Integration

**CanvasController:**
- Exposes `undo()` and `redo()` methods
- Provides `canUndo` and `canRedo` properties for QML binding
- Creates and executes commands for all canvas operations
- Example: `m_commandHistory->execute(std::make_unique<DeleteElementsCommand>(...));`

**DragManager:**
- Tracks total delta during drag operations
- Emits `dragEnded` signal with movement information
- CanvasController creates MoveElementsCommand on drag completion

**QML Integration:**
- Keyboard shortcuts in `InputController.qml`:
  - Cmd/Ctrl+Z for Undo
  - Cmd/Ctrl+Shift+Z for Redo
- Properties exposed for UI state: `controller.canUndo`, `controller.canRedo`

### Usage Example

```cpp
// Creating a frame
auto command = std::make_unique<CreateFrameCommand>(
    m_elementModel, m_selectionManager, QRectF(x, y, width, height));
m_commandHistory->execute(std::move(command));

// Moving elements
auto command = std::make_unique<MoveElementsCommand>(
    selectedElements, QPointF(deltaX, deltaY));
m_commandHistory->execute(std::move(command));

// Undo/Redo
m_commandHistory->undo();
m_commandHistory->redo();
```

### Command Implementation Guidelines

When creating new commands:

1. Inherit from `Command` base class
2. Store all necessary state for undo/redo
3. Implement `execute()` and `undo()` (redo() has default implementation)
4. Consider implementing merge functionality for consecutive operations
5. Set a descriptive string using `setDescription()`
6. Handle edge cases (null pointers, invalid states)
7. Manage object ownership carefully (who owns elements during undo/redo)

### Technical Notes

- Commands use move semantics (`std::unique_ptr`) for efficient memory management
- The `setExecuted()` method is private and only accessible to `CommandHistory`
- Commands should be stateless regarding their execution - they should work correctly whether called via execute() or redo()
- MoveElementsCommand handles the first execution specially since elements are already moved during drag

## Other instructions

- Never create "mock" implementations. Don't create files for testing purposes with the intention of throwing them away later.
- Eliminate all unused/stale methods, variables, etc. when you come across them. Don't leave unused code in the code base.
- Always look for Qts helpers before implementing something bespoke. For example, use QWidget::mouseDoubleClickEvent instead of creating double clicks with timers.
- DRY (Don't Repeat Yourself) code is incredibly important to this project. Where possible, modularize and re-use code. Point out redundancy and duplication when you come across it.
- Always use Qt::UniqueConnection when connecting signals to prevent duplicate connections. Example:
  ```cpp
  connect(element, &Element::geometryChanged,
          this, &SelectionManager::onElementGeometryChanged,
          Qt::UniqueConnection);
  ```
