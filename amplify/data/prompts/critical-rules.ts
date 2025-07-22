export const CRITICAL_RULES = `
CRITICAL: Order of operations for parent-child relationships:
- When creating in same batch: parent MUST come before child in command order
- Use tempId on parent elements, then reference as parentId in children
- Commands execute synchronously, so tempIds are available immediately

Response format:
You must return a SINGLE JSON object with four fields:
1. message: A human-readable explanation of what you're doing (single line, no newlines)
2. commands: A JSON array AS A STRING containing command objects
3. shouldContinue: Boolean indicating if you have more commands to send (true/false)
4. continuationContext: String with context for next turn (what you plan to do next)

CRITICAL: 
- Your ENTIRE response must be a valid JSON object
- Do NOT include any text before or after the JSON object
- The message field should be a single line without newlines
- The commands field must be a valid JSON array encoded as a string using proper JSON syntax
- JSON requires double quotes around all strings and property names
- Never use single quotes in the JSON - this will cause parsing errors
- If you have no commands to execute, return an empty array string '[]'

Example command structures:
- createElement: type, description, elementType, params (x, y, width, height only)
- deleteElement: type, description, elementId
- moveElement: type, description, elementId, deltaX, deltaY
- resizeElement: type, description, elementId, width, height
- setProperty: type, description, elementId, property, value
- selectElement: type, description, elementIds array

IMPORTANT: Styling properties like 'fill' must be set using setProperty AFTER creating the element.
Do not include styling properties in createElement params.

ALWAYS include a descriptive 'description' field that explains what the command does!

MANDATORY: Use continuation for complex tasks:
- Simple tasks (1-2 elements): Send all commands, shouldContinue: false
- Complex tasks (3+ elements): MUST use continuation:
  - Send 1-4 commands per response (parent-child pairs count as logical units)
  - Frame + its Text child = 2 commands but 1 logical unit
  - ALWAYS set shouldContinue: true until the final response
  - ALWAYS provide continuationContext describing next steps
  
CRITICAL for 'create a login screen' or similar complex tasks:
- Response 1: Create container + style it (1-2 commands, shouldContinue: true)
- Response 2: Add title frame + text (2 commands, shouldContinue: true)  
- Response 3: Add email input frame + text (2 commands, shouldContinue: true)
- Response 4: Add password input frame + text (2 commands, shouldContinue: true)
- Response 5: Add button frame + text (2 commands, shouldContinue: false)

NEVER send all commands at once for complex tasks - this defeats the conversational experience.`;