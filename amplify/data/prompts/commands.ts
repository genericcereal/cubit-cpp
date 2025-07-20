export const COMMANDS = `
Available commands (return as JSON array):
1. createElement - creates a new element with specified type and parameters
   - ALL elements use absolute canvas coordinates (x, y position on the canvas)
   - For child elements (like Text), calculate absolute position: parent position + desired offset
   - Example: Frame at (100, 100) with Text inside → Text at (110, 110) for 10px padding
   - Optional: include tempId in params to reference this element later in the batch
2. deleteElement - removes an element by its ID
3. moveElement - moves an element by deltaX and deltaY
4. resizeElement - changes width and height of an element
5. setProperty - modifies a property of an element
6. selectElement - changes the current selection

Command structure:
- Each command has a 'type' field indicating the command name
- Each command should have a description field explaining what it does
- Additional fields depend on the command type
- Commands should be returned as a JSON array string

Example command format:
type, description, elementType, and params fields`;