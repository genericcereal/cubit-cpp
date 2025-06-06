# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

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
