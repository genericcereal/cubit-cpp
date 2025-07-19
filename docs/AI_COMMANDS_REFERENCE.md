# CubitAI Commands Reference

This document describes the commands available to CubitAI for creating and manipulating elements on the canvas.

## Response Format

CubitAI should return responses in the following JSON format:

```json
{
  "message": "Human-readable explanation of what I'm doing",
  "commands": [
    // Array of command objects
  ]
}
```

## Available Commands

### 1. createElement

Creates a new element on the canvas.

```json
{
  "type": "createElement",
  "elementType": "Frame|Text|Node|Edge",
  "params": {
    "x": 100,
    "y": 100,
    "width": 200,
    "height": 150,
    "parentId": "optional-parent-id",  // Required for Text elements
    "text": "Text content",             // For Text elements
    "properties": {                     // Optional properties
      "backgroundColor": "#ffffff",
      "borderColor": "#000000",
      "borderWidth": 1
    }
  }
}
```

#### Element Types:
- **Frame**: Container element that can hold other elements
- **Text**: Text element (must be created inside a Frame)
- **Node**: Script canvas node for visual programming
- **Edge**: Connection between nodes

### 2. deleteElement

Removes an element from the canvas.

```json
{
  "type": "deleteElement",
  "elementId": "element-id-to-delete"
}
```

### 3. moveElement

Moves an element by a specified delta.

```json
{
  "type": "moveElement",
  "elementId": "element-id",
  "deltaX": 50,
  "deltaY": -30
}
```

### 4. resizeElement

Changes the size of an element.

```json
{
  "type": "resizeElement",
  "elementId": "element-id",
  "width": 300,
  "height": 200
}
```

### 5. setProperty

Sets a property value on an element.

```json
{
  "type": "setProperty",
  "elementId": "element-id",
  "property": "backgroundColor",
  "value": "#ff0000"
}
```

Common properties:
- Frame: backgroundColor, borderColor, borderWidth, borderRadius
- Text: content, font, color
- Node: nodeTitle, nodeColor

### 6. selectElement

Changes the current selection.

```json
{
  "type": "selectElement",
  "elementIds": ["id1", "id2", "id3"]
}
```

## Canvas State

CubitAI receives the current canvas state with each request:

```json
{
  "canvasMode": "design|script",
  "selectedElements": [
    {
      "id": "element-id",
      "name": "Element Name",
      "type": "Frame|Text|Node|Edge"
    }
  ],
  "elements": [
    {
      "id": "element-id",
      "name": "Element Name",
      "type": "Frame",
      "x": 100,
      "y": 100,
      "width": 200,
      "height": 150
    }
  ]
}
```

## Example Usage

### Creating a Frame with Text

```json
{
  "message": "I'll create a frame with a title inside",
  "commands": [
    {
      "type": "createElement",
      "elementType": "Frame",
      "params": {
        "x": 100,
        "y": 100,
        "width": 300,
        "height": 200,
        "properties": {
          "backgroundColor": "#f0f0f0"
        }
      }
    },
    {
      "type": "createElement",
      "elementType": "Text",
      "params": {
        "parentId": "{{frame-id-from-previous-command}}",
        "text": "Hello World",
        "x": 0,
        "y": 0,
        "width": 300,
        "height": 30
      }
    }
  ]
}
```

### Moving and Styling Elements

```json
{
  "message": "I'll move the selected element and change its color",
  "commands": [
    {
      "type": "moveElement",
      "elementId": "selected-element-id",
      "deltaX": 100,
      "deltaY": 0
    },
    {
      "type": "setProperty",
      "elementId": "selected-element-id",
      "property": "backgroundColor",
      "value": "#3498db"
    }
  ]
}
```

## Important Notes

1. Text elements must always be created inside a Frame
2. When creating elements, the AI should check the canvas state to avoid overlapping
3. Element IDs are generated automatically - the AI should use the IDs from the canvas state
4. All coordinates are in canvas space
5. Commands are executed sequentially in the order provided