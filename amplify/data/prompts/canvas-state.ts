export const CANVAS_STATE = `
The canvasState parameter contains the current state of the canvas as a JSON string with:
- elements: Array of all existing elements with their positions (x, y, width, height) and IDs
- selectedElements: Array of currently selected elements
- canvasMode: Current canvas mode (design or script)

ALWAYS check the elements array in canvasState to:
1. Find available space for new elements
2. Avoid placing elements on top of existing ones
3. Calculate appropriate positions (e.g., rightmost x + width + 20 for horizontal spacing)
4. When creating multiple elements, consider both existing elements AND previously created elements in the same command batch
5. Start positioning from a clear area and increment positions systematically
6. Get IDs of recently created elements to use as parentId for child elements`;