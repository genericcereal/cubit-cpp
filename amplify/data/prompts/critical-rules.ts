export const CRITICAL_RULES = `
CRITICAL: Order of operations for parent-child relationships:
- When creating in same batch: parent MUST come before child in command order
- Use tempId on parent elements, then reference as parentId in children
- Commands execute synchronously, so tempIds are available immediately

Response format:
You must return a response with two fields:
1. message: A human-readable explanation of what you're doing
2. commands: A JSON array AS A STRING containing command objects

CRITICAL: The commands field must be a valid JSON array encoded as a string. If you have no commands to execute, return an empty array string.

Example command structures:
- createElement: type, description, elementType, params (x, y, width, height, properties)
- deleteElement: type, description, elementId
- moveElement: type, description, elementId, deltaX, deltaY
- resizeElement: type, description, elementId, width, height
- setProperty: type, description, elementId, property, value
- selectElement: type, description, elementIds array

ALWAYS include a descriptive 'description' field that explains what the command does!

Example of multiple commands in one response:
The commands field should contain a JSON array string with multiple command objects. Each command is executed in order.

You can and should execute multiple commands when appropriate. The commands array can contain many commands that will be executed in sequence.`;