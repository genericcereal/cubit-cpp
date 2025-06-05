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

### Canvas Layer Architecture

The Canvas implements a Z-indexed layer system with four distinct layers:

1. **ElementLayer** (bottom, z-index 1): Base layer for primary content elements (Frames, Text)
2. **ClientRectLayer** (z-index 2): Layer for client rectangles that appear with frames
3. **ControlLayer** (z-index 3): Layer for interaction controls
4. **PanelLayer** (top, z-index 4): Layer for UI panels, contains the ActionsPanel

### UI Components

- **Canvas**: Main interactive area with the layered architecture
- **DetailPanel**: Resizable side panel with multiple scrollable sections
- **ActionsPanel**: A fixed-size, semi-transparent panel positioned at the bottom of the Canvas
- **Layer classes**: Specialized QWidget-based containers for organizing UI elements by function

### Component Relationships

```
+------------------+     +------------------+
| Main Window      |     |                  |
|  +-------------+ |     | Detail Panel     |
|  | Canvas      | |     | (QWidget)        |
|  | (QWidget)   | |     |                  |
|  |             | |     | - Scrollable     |
|  | [Layers:    | |     |   sections       |
|  |  -Element   | |     |                  |
|  |  -Control   | |     | - Vertically     |
|  |  -Panel]    | |     |   adjustable     |
|  |             | |     |   splitters      |
|  +-------------+ |     |                  |
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
5. Build with make

## Other instructions

- Never create "mock" implementations. Don't create files for testing purposes with the intention of throwing them away later.
- Eliminate all unused/stale methods, variables, etc. when you come across them. Don't leave unused code in the code base.
