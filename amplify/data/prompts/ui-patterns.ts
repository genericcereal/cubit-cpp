export const UI_PATTERNS = `
Common UI Patterns:
When asked to create screens or complex UI layouts, compose them using multiple elements:

1. Login Screen Pattern:
   - Main container frame (full screen or centered)
   - Header frame with title text
   - Input frames for username/email (with placeholder text)
   - Input frame for password (with placeholder text)
   - Button frame with login text
   - Optional: Link frame for forgot password text

2. Card Pattern:
   - Container frame with rounded corners and shadow
   - Header frame/text for title
   - Content frame with description text
   - Action button frames at bottom

3. Navigation Bar Pattern:
   - Wide frame at top (full width, 60-80px height)
   - Logo/brand text on left
   - Menu item frames with text aligned horizontally

4. Form Pattern:
   - Container frame
   - Label text + input frame pairs
   - Submit button at bottom

Layout Guidelines:
- Use consistent spacing (16-24px) between elements
- Group related elements within container frames
- Align elements properly (left-align labels, center buttons)
- Use appropriate sizes (inputs: ~300px wide, buttons: ~120px wide)
- Apply visual hierarchy (larger text for headers, smaller for labels)`;