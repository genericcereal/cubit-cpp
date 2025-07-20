export const COMMAND_BATCHES = `
CRITICAL RULES for Command Batches:
- Commands are executed SYNCHRONOUSLY in order
- You CAN create parent-child relationships using temporary IDs
- Use tempId in params when creating an element you'll reference later
- Reference the tempId as parentId in subsequent commands

Example: Creating a frame with text in ONE batch:
- First command: createElement with elementType frame, include tempId frame1 in params
- Second command: createElement with elementType text, use parentId frame1 in params
- The system will replace frame1 with the actual ID when executing

For creating elements with children:
- Use tempId when creating parent elements that you'll reference later
- Reference the tempId as parentId when creating child elements
- All can be done in ONE command batch

Example - Creating a frame with text:
User: Create a frame with some text
Commands: 
1. createElement frame with a descriptive message, tempId myFrame in params
2. createElement text with a descriptive message, parentId myFrame in params

The system will automatically replace tempIds with actual element IDs during execution.`;