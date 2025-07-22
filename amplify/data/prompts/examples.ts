export const EXAMPLES = `
Example of conversational approach with continuation:

User: 'Create a complex UI'

AI Response 1:
{
  \\"message\\": \\"I'll start by creating the main container\\",
  \\"commands\\": \\"[{\\\\\\"type\\\\\\":\\\\\\"createElement\\\\\\",\\\\\\"description\\\\\\":\\\\\\"Create container\\\\\\",\\\\\\"elementType\\\\\\":\\\\\\"frame\\\\\\",\\\\\\"x\\\\\\":100,\\\\\\"y\\\\\\":100,\\\\\\"width\\\\\\":400,\\\\\\"height\\\\\\":300}]\\",
  \\"shouldContinue\\": true,
  \\"continuationContext\\": \\"Next I'll add child elements\\"
}

AI Response 2:
{
  \\"message\\": \\"Now I'll add child elements\\",
  \\"commands\\": \\"[{\\\\\\"type\\\\\\":\\\\\\"createElement\\\\\\",\\\\\\"description\\\\\\":\\\\\\"Add text\\\\\\",\\\\\\"elementType\\\\\\":\\\\\\"text\\\\\\",\\\\\\"x\\\\\\":120,\\\\\\"y\\\\\\":120,\\\\\\"parentId\\\\\\":\\\\\\"frame1\\\\\\"}]\\",
  \\"shouldContinue\\": true,
  \\"continuationContext\\": \\"Next I'll add more details\\"
}

Remember: Use proper JSON syntax with double quotes in the actual commands array.

Group related commands with descriptive messages to show progress.

Example: Creating multiple elements with spacing:
- First element at position 100,100
- Second element with appropriate spacing from first
- Third element with consistent spacing pattern

Example: Creating parent-child relationships using tempIds:
1. Create parent element with tempId
2. Create child element with parentId referencing the tempId
3. Continue pattern for complex hierarchies

When creating complex UI:
- Start with the main container
- Create child elements in logical order
- Use consistent spacing and alignment
- Apply appropriate styling

For specific examples, see element documentation:
- frames.ts for frame-based UI examples
- text.ts for text element examples
- nodes.ts for node-based visual programming examples
- edges.ts for connection examples`;