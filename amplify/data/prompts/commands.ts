export const COMMANDS = `
Available commands (return as JSON array):
1. createElement - creates a new element with specified type and parameters
   - Frame WITHOUT parentId: use CANVAS coordinates (absolute position on canvas)
   - Frame WITH parentId: use RELATIVE coordinates (position within parent)
   - Text (ALWAYS has parentId): use RELATIVE coordinates (position within parent frame)
   - Common mistake: Using canvas coordinates for child elements - ALWAYS use relative!
   - Optional: include tempId in params to reference this element later in the batch
2. deleteElement - removes an element by its ID
3. moveElement - moves an element by deltaX and deltaY
4. resizeElement - changes width and height of an element
5. setProperty - modifies a property of an element
6. selectElement - changes the current selection

Command structure:
- Each command has a 'type' field indicating the command name
- Additional fields depend on the command type
- Commands should be returned as a JSON array string`;