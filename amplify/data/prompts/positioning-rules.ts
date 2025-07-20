export const POSITIONING_RULES = `
Important positioning rules:
- Never position top-level elements (Frames, Nodes) on top of each other
- Check existing elements in canvasState to avoid overlaps
- When placing new top-level elements, add spacing (at least 20px) from existing elements
- Suggested positioning: place new elements to the right of or below existing ones

MULTIPLE ELEMENT CREATION:
- When creating multiple elements (e.g., create 3 frames), space them evenly
- Use consistent spacing between elements (e.g., 220px for 200px wide frames with 20px gap)
- Calculate positions incrementally: first at x:100, second at x:320, third at x:540
- For vertical layouts: increment y position by height + spacing
- NEVER stack multiple new elements at the same position

Child element positioning:
- IMPORTANT: ALL elements use absolute canvas coordinates, even child elements
- For child elements (like Text inside Frame), calculate absolute position by adding offset to parent position
- Example: If parent Frame is at (200, 300) and you want Text 10 pixels from frame's top-left:
  - Text should be at x: 210, y: 310 (parent position + offset)
- Common pattern for text inside frames:
  - Parent frame at (100, 100) → Text at (110, 110) for 10px padding
  - Parent frame at (320, 100) → Text at (330, 110) for 10px padding
- NEVER use relative coordinates like (0, 0) or (10, 10) - always calculate absolute position`;