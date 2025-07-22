export const FRAMES_PROMPTS = `
FRAME ELEMENT DOCUMENTATION:

Definition:
- Frame is a container element with background, border, and can contain other elements
- Text elements must be inside a Frame
- Element type: frame
- Is container: true
- Can contain: text elements

Properties:
- fill: Background color of the frame
  - Type: string
  - Format: hex color like #FF0000 or #3498db
  - Example setProperty command: type is setProperty, elementId is frame1, property is fill, value is #3498db
- Additional properties like border, borderRadius, etc. can be added as needed

Creation:
- Command: createElement
- Required fields: elementType, x, y, width, height
- Optional: include tempId in params to reference this element later in the batch
- IMPORTANT: Frame sizes should vary based on their purpose and content

Frame Size Guidelines:
- Container frames: Typically larger (400-600px wide, variable height)
- Input frames: 250-350px wide, 40-50px tall
- Button frames: 100-200px wide, 40-50px tall
- Title/header frames: Match container width, 60-80px tall
- Card frames: 300-400px wide, height based on content
- DO NOT use the same size for all frames - vary sizes appropriately

Example creation command (container frame):
type: createElement
elementType: frame
x: 100
y: 100
width: 500
height: 400

Example with tempId (input frame):
type: createElement
elementType: frame
tempId: emailInput
x: 150
y: 220
width: 300
height: 45

JSON Command Examples:

Container frame creation and styling:
Create: {\\"type\\":\\"createElement\\",\\"description\\":\\"Create main container\\",\\"elementType\\":\\"frame\\",\\"params\\":{\\"x\\":100,\\"y\\":100,\\"width\\":500,\\"height\\":400}}
Fill: {\\"type\\":\\"setProperty\\",\\"description\\":\\"Set container background\\",\\"elementId\\":\\"[frame-id]\\",\\"property\\":\\"fill\\",\\"value\\":\\"#f5f5f5\\"}

Input frame with background:
Create: {\\"type\\":\\"createElement\\",\\"description\\":\\"Create email input frame\\",\\"elementType\\":\\"frame\\",\\"params\\":{\\"x\\":150,\\"y\\":220,\\"width\\":300,\\"height\\":45,\\"tempId\\":\\"emailFrame\\"}}
Fill: {\\"type\\":\\"setProperty\\",\\"description\\":\\"Set input background\\",\\"elementId\\":\\"emailFrame\\",\\"property\\":\\"fill\\",\\"value\\":\\"#ffffff\\"}

Button frame with color:
Create: {\\"type\\":\\"createElement\\",\\"description\\":\\"Create login button\\",\\"elementType\\":\\"frame\\",\\"params\\":{\\"x\\":150,\\"y\\":360,\\"width\\":150,\\"height\\":45}}
Fill: {\\"type\\":\\"setProperty\\",\\"description\\":\\"Set button color\\",\\"elementId\\":\\"[frame-id]\\",\\"property\\":\\"fill\\",\\"value\\":\\"#3498db\\"}

Note: Fill and other styling properties MUST be set with setProperty AFTER creating the frame

Positioning Rules:
- Frames are top-level elements
- Never position frames on top of each other
- Add at least 20px spacing between frames
- When creating multiple frames, space them evenly (e.g., 220px apart for 200px wide frames)
- Child elements inside frames use absolute canvas coordinates

UI Patterns:
Frames are used as containers in all UI patterns:

1. Login Screen Pattern:
   - Main container frame (400x500)
   - Header frame for title
   - Input frames for form fields
   - Button frame for actions

2. Card Pattern:
   - Container frame with rounded corners for card-like UI

3. Navigation Bar Pattern:
   - Wide frame positioned at top of canvas

4. Form Pattern:
   - Container frame with label/input frame pairs

Guideline: Group related elements within container frames

Example - Login Screen Layout:
- Main container: 400x500 at position 100,100 (overall container for the login form)
- Title frame: 300x60 at position 150,140 (contains the login title text)
- Email frame: 300x40 at position 150,220 (contains email input field)
- Password frame: 300x40 at position 150,280 (contains password input field)
- Button frame: 300x45 at position 150,360 (contains login button)

Pattern: Start with main container, create child frames for each UI component section

Critical Rules:
- Frame + Text child counts as 2 commands but represents 1 logical UI unit
- Complex UIs are built incrementally: create container frame first, add child frames for sections, add text elements last
- Frames are the primary building block for UI layouts

Best Practices:
- Always create the parent frame before its children
- Use frames to group related UI elements
- Consider frame hierarchy when building complex layouts
- Frames can be nested to create sophisticated layouts`;