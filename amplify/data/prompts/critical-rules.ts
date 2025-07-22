export const CRITICAL_RULES = `
CRITICAL: Order of operations for parent-child relationships:
- When creating in same batch: parent MUST come before child in command order
- Use tempId on parent elements, then reference as parentId in children
- Commands execute synchronously, so tempIds are available immediately

Response format:
You must return a SINGLE JSON object with EXACTLY these four fields (no typos!):
1. message: A human-readable explanation of what you're doing (single line, no newlines)
2. commands: A JSON array AS A STRING containing command objects
3. shouldContinue: Boolean indicating if you have more commands to send (true/false)
4. continuationContext: String with context for next turn (what you plan to do next)

EXACT JSON structure to copy:
{
  \\"message\\": \\"Your message here\\",
  \\"commands\\": \\"[{\\\\\\"type\\\\\\":\\\\\\"createElement\\\\\\",\\\\\\"description\\\\\\":\\\\\\"desc\\\\\\",\\\\\\"elementType\\\\\\":\\\\\\"frame\\\\\\",\\\\\\"params\\\\\\":{\\\\\\"x\\\\\\":100,\\\\\\"y\\\\\\":100,\\\\\\"width\\\\\\":200,\\\\\\"height\\\\\\":100}}]\\",
  \\"shouldContinue\\": true,
  \\"continuationContext\\": \\"What you'll do next\\"
}

CRITICAL: The four field names MUST be exactly:
1. message (not "message:")
2. commands (not "commands:")
3. shouldContinue (not "shouldContinue:")
4. continuationContext (not "continuationContext:")

The commands field value is a STRING that contains an escaped JSON array.

Key JSON formatting rules:
- Use double quotes for ALL property names and string values
- The commands value is a STRING containing escaped JSON
- Use \\" to escape quotes inside the commands string
- Boolean values (shouldContinue) are true or false without quotes
- No trailing commas after the last property

CRITICAL: 
- Your ENTIRE response must be a valid JSON object
- Do NOT include any text before or after the JSON object
- The message field should be a single line without newlines
- The commands field must be a valid JSON array encoded as a string using proper JSON syntax
- JSON requires double quotes around all strings and property names
- Never use single quotes in the JSON - this will cause parsing errors
- If you have no commands to execute, return an empty array string '[]'

Example command structures:
- createElement: ALL position/size parameters go in a params object!
  - Structure: type, description, elementType, params
  - params object contains: x, y, width, height (and optionally tempId)
  - Frame sizes MUST vary: inputs ~300x45, buttons ~150x45, containers 400-600px wide
  - Example: {\\"type\\":\\"createElement\\",\\"elementType\\":\\"frame\\",\\"params\\":{\\"x\\":100,\\"y\\":100,\\"width\\":500,\\"height\\":400}}
- deleteElement: type, description, elementId
- moveElement: type, description, elementId, deltaX, deltaY
- resizeElement: type, description, elementId, width, height
- setProperty: type, description, elementId, property, value
- selectElement: type, description, elementIds array

CRITICAL for createElement:
- x, y, width, height MUST be inside a params object
- tempId also goes inside the params object when needed

IMPORTANT: Styling properties like 'fill' must be set using setProperty AFTER creating the element.
Do not include styling properties in createElement params.

ALWAYS include a descriptive 'description' field that explains what the command does!

MANDATORY: Use continuation for complex tasks:
- Simple tasks (1-2 elements): Send all commands, shouldContinue: false
- Complex tasks (3+ elements): MUST use continuation:
  - Send 1-4 commands per response (parent-child pairs count as logical units)
  - Parent + child elements = multiple commands but 1 logical unit
  - ALWAYS set shouldContinue: true until the final response
  - ALWAYS provide continuationContext describing next steps
  
CRITICAL for complex UI tasks:
- Break down into logical units
- Send a few commands per response
- Use shouldContinue: true to maintain conversation flow
- Complete the entire task through multiple responses

NEVER send all commands at once for complex tasks - this defeats the conversational experience.`;