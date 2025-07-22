export const COMMAND_BATCHES = `
CRITICAL RULES for Command Batches:
- Commands are executed SYNCHRONOUSLY in order
- You CAN create parent-child relationships using temporary IDs
- Use tempId in params when creating an element you'll reference later
- Reference the tempId as parentId in subsequent commands

Example: Creating parent-child relationships in ONE batch:
- First command: createElement for parent element, include tempId in params
- Second command: createElement for child element, use parentId referencing the tempId
- The system will replace tempId with the actual ID when executing

For creating elements with children:
- Use tempId when creating parent elements that you'll reference later
- Reference the tempId as parentId when creating child elements
- All can be done in ONE command batch

For specific element parent-child rules, see element documentation (frames.ts, text.ts, etc.)

The system will automatically replace tempIds with actual element IDs during execution.`;