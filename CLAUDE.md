# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Terminology

- **Selection Box/Rectangle**: The blue rectangle that appears when dragging to select multiple elements
- **Controls**: The resize handles that appear on selected elements, consisting of:
  - **Bars**: The lines/borders around selected elements that can be dragged to resize
  - **Joints**: The corner handles (8x8 squares) that can be dragged to resize

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
qmake -o Makefile qt-hello.pro
```

## Architecture

This project is a Qt-based C++ application with a layered UI architecture:

### Controls System Architecture

The controls system is designed to support both current resize functionality and future rotation features:

#### Control Elements
- **Bars**: The lines/borders around selected elements that can be dragged to resize
- **Joints**: Corner handles that appear at the intersection of bars
  - **Resize joints** (yellow, smaller): Currently decorative, will handle corner resizing
  - **Rotation joints** (blue, larger): Currently decorative, will handle rotation

#### Resize Behavior
- **Single Selection**: Direct manipulation of the element
- **Multiple Selection**: Proportional scaling of all selected elements
- **Edge Flipping**: When a bar is dragged past its opposite bar:
  - Single selection: The element flips and continues resizing
  - Multiple selection: All elements maintain their relative positions but flip as a group

#### Design Principles for Future Rotation Support
1. **Edge-Agnostic Logic**: Controls should work based on "start/end" edges rather than "top/bottom/left/right"
2. **Unified Behavior**: Single selection is treated as a special case of multiple selection for consistency
3. **Transformation Pipeline**: When rotation is added, transformations will be applied in order:
   - Calculate changes in local coordinate space
   - Apply rotation transformation
   - Update element properties

#### Implementation Notes
- Controls always render the same way regardless of selection count
- The control overlay uses a bounding box approach for both single and multiple selections
- Mouse interactions are handled in viewport coordinates and converted to canvas coordinates
- Minimum element size of 1x1 pixels is enforced during all resize operations (bars can overlap)

#### Known Resize Behavior Issues
- **Left vs Right Bar Asymmetry**: The left bar resize calculates transformations differently than the right bar
  - Right bar correctly maintains element proportions during resize
  - Left bar has incorrect transform origin, causing unexpected scaling behavior
  - This affects both single and multiple selection
- **Solution**: Both bars should use the same scaling approach - calculate scale factor based on new size vs original size, then apply uniformly

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

### Migration Roadmap

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

### Key Technical Considerations

**Performance Optimizations:**

- Use `layer.enabled: true` for complex items
- Implement object pooling for elements
- Use `Loader` for on-demand component creation
- Batch property updates with `Qt.callLater()`

**Architecture Benefits:**

- Declarative UI reduces code complexity
- Better touch/mobile support
- Hardware-accelerated rendering
- Smoother animations and transitions
- Easier theming and styling

**Potential Challenges:**

- Learning curve for QML/JavaScript
- Different event handling model
- Custom painting requires more work
- WebEngineView integration complexity

### Development Tools & Resources

**Required Tools:**

- Qt Creator with QML designer
- QML profiler for performance
- Gammaray for debugging
- Qt Quick Controls 2 documentation

**Recommended Libraries:**

- Qt Quick Controls 2 for UI components
- Qt Quick Shapes for custom graphics
- Qt Quick Layouts for responsive design

### Migration Notes

_This section will be updated as the migration progresses with lessons learned, decisions made, and any deviations from the original plan._
