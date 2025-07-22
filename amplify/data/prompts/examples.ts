export const EXAMPLES = `
Example of conversational approach with continuation:

User: 'Create a login screen'
AI Response 1:
{
  message: 'I'll start by creating the main container for the login form',
  commands: '[{command to create container frame}]',
  shouldContinue: true,
  continuationContext: 'Next I'll add a title to the login form'
}

AI Response 2:
{
  message: 'Now I'll add the login title',
  commands: '[{command to create title text}]',
  shouldContinue: true,
  continuationContext: 'Next I'll create the input fields for email and password'
}

Remember: Use proper JSON syntax with double quotes in the actual commands array.

Group related commands with descriptive messages to show progress.

Example: Creating multiple Frames (e.g., create 3 frames):
- First frame at position 100,100 size 200x150
- Second frame at position 320,100 size 200x150 (x position is 100 + 200 + 20)
- Third frame at position 540,100 size 200x150 (x position is 320 + 200 + 20)

Example: Creating a Login Screen (all in ONE batch using tempIds):
1. Main container frame at position 100,100 size 400x500
2. Title frame at position 150,140 size 300x60 with tempId titleFrame
3. Title text at position 150,140 with parentId titleFrame (same as frame position for top-left alignment)
4. Email frame at position 150,220 size 300x40 with tempId emailFrame
5. Email text at position 160,230 with parentId emailFrame (frame position + 10 for padding)
6. Password frame at position 150,280 size 300x40 with tempId passFrame
7. Password text at position 160,290 with parentId passFrame (frame position + 10 for padding)
8. Button frame at position 150,360 size 300x45 with tempId btnFrame
9. Button text at position 150,372 with parentId btnFrame (frame position + 12 for vertical centering)

When creating complex UI:
- Start with the main container
- Create child frames for each UI component
- Add text elements last (they need parent frames to exist first)
- Use consistent spacing and alignment
- Apply appropriate styling (colors, borders, etc.)`;