export const TEXT_PROMPTS = `
TEXT ELEMENT DOCUMENTATION:

Definition:
- Text element that must be inside a Frame
- Used for displaying text content within containers
- Element type: text
- Is container: false

Properties:
- content: The text content to display (required)
  - Type: string
  - Example: Hello World
- fontFamily: Font family for the text
  - Type: string
  - Example: Arial, Helvetica, sans-serif
- fontSize: Size of the text in pixels
  - Type: number
  - Example: 16
- Additional properties like fontWeight, textAlign, etc. can be added as needed

Creation:
- Command: createElement
- Required fields: elementType, x, y, parentId
- Text elements MUST have a parentId (typically a Frame)
- Text position is absolute on the canvas, not relative to parent
- Calculate text position by adding desired offset to parent position

Example creation command:
type: createElement
elementType: text
x: 110  (Parent frame x 100 + padding 10)
y: 110  (Parent frame y 100 + padding 10)
parentId: frame1  (or use tempId reference)

JSON Command Examples:

Basic text creation:
{\\"type\\":\\"createElement\\",\\"description\\":\\"Add login title\\",\\"elementType\\":\\"text\\",\\"params\\":{\\"x\\":160,\\"y\\":155,\\"parentId\\":\\"6602168831179409\\"}}

Text with content in params (alternative):
{\\"type\\":\\"createElement\\",\\"description\\":\\"Add button text\\",\\"elementType\\":\\"text\\",\\"params\\":{\\"x\\":190,\\"y\\":315,\\"parentId\\":\\"btnFrame\\",\\"content\\":\\"Submit\\"}}

Setting properties after creation:
{\\"type\\":\\"setProperty\\",\\"description\\":\\"Set text content\\",\\"elementId\\":\\"[text-element-id]\\",\\"property\\":\\"content\\",\\"value\\":\\"Login\\"}
{\\"type\\":\\"setProperty\\",\\"description\\":\\"Set font size\\",\\"elementId\\":\\"[text-element-id]\\",\\"property\\":\\"fontSize\\",\\"value\\":24}
{\\"type\\":\\"setProperty\\",\\"description\\":\\"Set font family\\",\\"elementId\\":\\"[text-element-id]\\",\\"property\\":\\"fontFamily\\",\\"value\\":\\"Inter\\"}

Note: Text content and styling are set using setProperty AFTER creating the text element

Positioning Rules:
- Use absolute canvas coordinates for positioning
- For padding from parent frame edge, add offset to parent's position
- Example: Frame at (100,100) with 10px padding → Text at (110,110)
- Text does not have width/height in creation - it auto-sizes to content

Common Positioning Patterns:
- Top-left aligned: Position text at parent position + padding
- Centered: Calculate center point of parent frame for text position
- Right aligned: Position based on parent's right edge minus text width

Usage Patterns:

1. Labels:
   - Text used as labels for form inputs or UI elements
   - Small font size (12-14px), positioned above or beside input fields

2. Headers:
   - Text used as titles or section headers
   - Larger font size (18-24px), often centered in parent frame

3. Buttons:
   - Text inside button frames
   - Centered both horizontally and vertically in button frame

4. Content:
   - Body text or descriptions
   - Regular font size (14-16px), left-aligned with proper line spacing

Visual Hierarchy Guidelines:
- Use larger text (18-24px) for headers and titles
- Use medium text (14-16px) for body content and descriptions
- Use smaller text (12-14px) for labels and secondary information
- Maintain consistent font sizes across similar UI elements

Example - Creating Text Inside a Frame:
First create the parent frame:
type: createElement
elementType: frame
tempId: myFrame
x: 100
y: 100
width: 200
height: 50

Then create text with parentId:
type: createElement
elementType: text
parentId: myFrame
x: 110  (100 + 10 padding)
y: 120  (100 + 20 for vertical centering)

Complete JSON example with tempId:
Frame: {\\"type\\":\\"createElement\\",\\"description\\":\\"Create button frame\\",\\"elementType\\":\\"frame\\",\\"params\\":{\\"x\\":150,\\"y\\":300,\\"width\\":120,\\"height\\":45,\\"tempId\\":\\"btnFrame\\"}}
Text: {\\"type\\":\\"createElement\\",\\"description\\":\\"Add button text\\",\\"elementType\\":\\"text\\",\\"params\\":{\\"x\\":190,\\"y\\":315,\\"parentId\\":\\"btnFrame\\",\\"tempId\\":\\"btnText\\"}}
Content: {\\"type\\":\\"setProperty\\",\\"description\\":\\"Set button text\\",\\"elementId\\":\\"btnText\\",\\"property\\":\\"content\\",\\"value\\":\\"Submit\\"}

Best Practices:
- Always create the parent frame before creating text
- Use consistent padding from frame edges (typically 10-20px)
- Consider text alignment when positioning (left, center, right)
- Group related text elements within the same parent frame
- Use semantic text sizes to establish visual hierarchy

Limitations:
- Text cannot exist without a parent Frame
- Text elements cannot contain other elements
- Width and height are determined by content, not explicitly set`;
