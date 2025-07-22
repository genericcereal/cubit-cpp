export const COMMANDS = `Available commands to return as JSON array:
1. createElement - creates a new element with specified type and parameters
   - ALL elements use absolute canvas coordinates for x and y position on the canvas
   - For child elements like Text, calculate absolute position by adding parent position plus desired offset
   - Example: Frame at 100,100 with Text inside should have Text at 110,110 for 10px padding
   - Optional: include tempId in params to reference this element later in the batch
2. deleteElement - removes an element by its ID
3. moveElement - moves an element by deltaX and deltaY
4. resizeElement - changes width and height of an element
5. setProperty - modifies a property of an element
   - For Frame elements: fill property accepts color as hex string like #FF0000 or #3498db
   - For Text elements: content property is required - the text to display
   - Example for Frame: type is setProperty, elementId is frame1, property is fill, value is #3498db
6. selectElement - changes the current selection

Command structure:
- Each command has a type field indicating the command name
- Each command should have a description field explaining what it does
- Additional fields depend on the command type
- Commands should be returned as a JSON array string

Example command format:
type, description, elementType, and params fields

CRITICAL JSON FORMATTING RULES:
1. The commands field MUST contain a properly escaped JSON string
2. NEVER split element IDs, property names, or values across JSON boundaries
3. Always validate your JSON structure before responding
4. Use double backslashes for escaping quotes inside the commands string
5. Keep all JSON on a single line - no newlines within the commands array string
6. Double-check that all parentIds and elementIds are complete 16-digit strings

Response format MUST follow this exact structure:
- message: Brief description of what you are doing
- commands: A string containing an escaped JSON array of commands
- shouldContinue: Boolean indicating whether to continue
- continuationContext: What you will do next if shouldContinue is true

IMPORTANT: The commands value is a STRING containing a JSON array, not a direct JSON array.
The commands string should contain escaped JSON with double backslashes before quotes.
Never use brackets or braces directly in the prompt text outside of the commands string.`;
