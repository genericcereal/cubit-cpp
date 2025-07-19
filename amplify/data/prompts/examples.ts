export const EXAMPLES = `
Example: Creating a Frame with Text (in ONE batch):
- First command: Create Frame with tempId myFrame
- Second command: Create Text with parentId myFrame

Example: Creating multiple Frames (e.g., create 3 frames):
- First frame at position 100,100 size 200x150
- Second frame at position 320,100 size 200x150 (x position is 100 + 200 + 20)
- Third frame at position 540,100 size 200x150 (x position is 320 + 200 + 20)

Example: Creating a Login Screen (all in ONE batch using tempIds):
1. Main container frame at CANVAS position 100,100 size 400x500 (no parent, so uses canvas coordinates)
2. Title frame at CANVAS position 150,140 size 300x60 with tempId titleFrame (no parent specified)
3. Title text at RELATIVE position 0,0 with parentId titleFrame (inside frame, so relative to frame)
4. Email frame at CANVAS position 150,220 size 300x40 with tempId emailFrame (no parent specified)
5. Email text at RELATIVE position 10,10 with parentId emailFrame (inside frame, so relative)
6. Password frame at CANVAS position 150,280 size 300x40 with tempId passFrame (no parent specified)
7. Password text at RELATIVE position 10,10 with parentId passFrame (inside frame, so relative)
8. Button frame at CANVAS position 150,360 size 300x45 with tempId btnFrame (no parent specified)
9. Button text at RELATIVE position 0,12 with parentId btnFrame (inside frame, so relative)

When creating complex UI:
- Start with the main container
- Create child frames for each UI component
- Add text elements last (they need parent frames to exist first)
- Use consistent spacing and alignment
- Apply appropriate styling (colors, borders, etc.)`;