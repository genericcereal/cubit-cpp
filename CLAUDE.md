# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Terminology

- **Selection Box/Rectangle**: The blue rectangle that appears when dragging to select multiple elements

## Building and Running

### Build Commands

```bash
# Clean the project
make clean

# Build the project
make

# Run the application
./qt-hello
```

### Regenerating Makefile from Qt Project

```bash
qmake6 -o Makefile qt-hello.pro
```

### CONTROLS

The controls system is designed to move, resize, and rotate the current selection. It should be implemented in the viewport overlay - the controls surface should scale with canvas zooms, but the individual control elements (bars, joints) should always stay a fixed size.

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

### Implementation Steps

1. **Phase 1: Basic Structure**
   - Create control surface with move functionality
   - Implement coordinate transformation helpers
   - Add visual feedback (colors, opacity)
   - Ensure controls only appear when selection exists

2. **Phase 2: Resize Bars**
   - Implement individual bar components
   - Add drag handlers for each edge
   - Calculate new dimensions based on drag distance
   - Update selected elements proportionally
   - Handle edge flipping behavior

3. **Phase 3: Resize Joints**
   - Add corner resize joints
   - Implement proportional corner resizing
   - Maintain aspect ratio for single selection
   - Scale all elements proportionally for multiple selection

4. **Phase 4: Rotation Joints**
   - Add rotation joints behind resize joints
   - Calculate rotation angle from drag position
   - Apply rotation to controls container
   - Update element rotations accordingly

5. **Phase 5: Advanced Behaviors**
   - Implement click-to-select on surface
   - Add proper state management for drag operations
   - Ensure minimum size constraints
   - Handle edge cases (zero size, negative dimensions)

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

The Canvas renders multiple types of elements:

1. **Frames**: Visual frame elements (QFrame-based widgets) that serve as containers
2. **Text**: Text display elements that are always contained within Frames
3. **Variables**: Non-visual elements that store data (created but not displayed on canvas)

#### Element Hierarchy:

- **Text elements are ALWAYS created within Frame containers**
- A Frame can contain multiple Text elements
- Text elements cannot exist independently on the Canvas

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

**Core C++ Classes to Retain:**

- `Element` hierarchy (as Q_OBJECT with Q_PROPERTY)
- Business logic and data management
- Performance-critical algorithms

**New C++ Classes:**

- `ElementModel` - QAbstractListModel for QML ListView
- `CanvasController` - Handles interaction logic
- `SelectionManager` - Selection state management
- `ElementFactory` - Creates QML components dynamically

**QML Exposure:**

```cpp
qmlRegisterType<Element>("Cubit", 1, 0, "Element");
qmlRegisterType<Frame>("Cubit", 1, 0, "Frame");
qmlRegisterType<CanvasController>("Cubit", 1, 0, "CanvasController");
```

#### Phase 1: Foundation (2-3 weeks) - COMPLETED

- [x] Set up Qt Quick project structure
- [x] Create basic QML window with SplitView
- [x] Implement C++ backend classes with Q_PROPERTY
- [x] Register C++ types with QML
- [x] Create basic Element QML components

**Phase 1 Summary:**

- Created parallel Qt Quick project in `/qtquick` directory
- Implemented base Element class with Q_PROPERTY bindings
- Created Frame, Text, Html, and Variable element classes
- Built CanvasController, ElementModel, and SelectionManager
- Set up main QML window with SplitView layout
- Created basic QML components for all UI elements
- Project structure allows side-by-side development with original Qt Widgets version

#### Phase 2: Canvas Implementation (3-4 weeks) - COMPLETED

- [x] Implement custom QQuickItem for canvas
- [x] Port mouse/touch interaction handling
- [x] Implement pan/zoom with PinchHandler and WheelHandler
- [x] Create selection system with MultiPointTouchArea
- [x] Port coordinate system management

**Phase 2 Summary:**

- Implemented complete canvas interaction system in QML
- Added zoom with Ctrl+mouse wheel (maintains point under cursor)
- Middle mouse button panning
- Selection box with real-time element selection
- Element creation preview when dragging in creation modes
- Coordinate system properly converts between view and canvas space
- Keyboard shortcuts (Delete, Ctrl+A, Escape)
- Grid overlay that appears when zoomed in
- Scroll indicators for better navigation

#### Phase 3: Element System (2-3 weeks) - IN PROGRESS

- [x] Create QML components for each element type
- [x] Implement element rendering in QML
- [ ] Port parent-child relationships
- [x] Implement clipping with `clip` property
- [ ] Create element manipulation controls

**Phase 3 Progress:**

- Created base ElementItem component with common element functionality
- Implemented FrameElement, TextElement, and HtmlElement inheriting from ElementItem
- Added consistent selection visual feedback across all element types
- Fixed Qt 6 compatibility issues with imports and signal handlers
- Implemented proper property bindings between QML and C++ objects
- Added clipping support for Frame elements
- Attempted hierarchical rendering for parent-child relationships (needs more work)

**Known Issues:**

- Element dragging in select mode not working (mouse handler integration needed)
- Visual feedback during element creation needs to be restored
- Parent-child element relationships need proper implementation
- Resize handles not yet implemented

#### Phase 4: UI Panels (2 weeks) - COMPLETED

- [x] Port DetailPanel to QML
- [x] Create ElementList with ListView
- [x] Implement Properties panel with dynamic forms
- [x] Port ActionsPanel with ToolBar
- [x] Create FPS monitor component

All UI panels were already implemented in the qtquick directory. Updated all panel QML files to use Qt6 unversioned imports.

#### Phase 5: Advanced Features (2-3 weeks) - NOT STARTED

- [ ] Implement "foreground" (i.e. viewport based) implementation for controls, selection boxes, hover indicators, etc. These should not scale when the canvas zooms.
- [ ] Optimize performance with batching
- [ ] Implement undo/redo system

#### Phase 6: Testing & Polish (2 weeks) - NOT STARTED

- [ ] Performance testing and optimization
- [ ] Memory leak detection
- [ ] Cross-platform testing
- [ ] UI/UX refinements
- [ ] Documentation

### Other Notes

- Only use the legacy folder when the user tells you to reference a previous implementation. Do not make edits to anything in the legacy folder.
