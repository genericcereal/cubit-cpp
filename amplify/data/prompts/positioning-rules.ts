export const POSITIONING_RULES = `
Important positioning rules:
- Never position top-level elements on top of each other
- Check existing elements in canvasState to avoid overlaps
- When placing new top-level elements, add spacing (at least 20px) from existing elements
- Suggested positioning: place new elements to the right of or below existing ones

MULTIPLE ELEMENT CREATION:
- When creating multiple elements, space them evenly
- Use consistent spacing between elements (20px minimum gap)
- Calculate positions incrementally based on element size and spacing
- For vertical layouts: increment y position by height + spacing
- NEVER stack multiple new elements at the same position

Child element positioning:
- IMPORTANT: ALL elements use absolute canvas coordinates, even child elements
- For child elements, calculate absolute position by adding offset to parent position
- Example: If parent is at (200, 300) and you want child 10 pixels from parent's top-left:
  - Child should be at x: 210, y: 310 (parent position + offset)
- NEVER use relative coordinates like (0, 0) or (10, 10) - always calculate absolute position

For element-specific positioning rules, see individual element documentation (frames.ts, text.ts, nodes.ts, edges.ts)`;