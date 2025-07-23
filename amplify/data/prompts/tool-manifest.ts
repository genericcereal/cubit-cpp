export const toolManifest = {
  canvas: [
    {
      name: "set_canvas_position",
      description:
        "Set the position of the canvas.  This is usually used position the canvas so the user can see the elements you are creating.",
      params: [
        { name: "x", type: "number", description: "X position of the canvas" },
        { name: "y", type: "number", description: "Y position of the canvas" },
        {
          name: "zoom",
          type: "number",
          description: "Zoom level of the canvas",
        },
      ],
    },
    {
      name: "set_canvas_type",
      description:
        "Set the kind of canvas you are working with. Nodes and edges are only available on a script canvas.",
      params: [
        {
          name: "canvasType",
          type: "string",
          description: "Type of canvas (e.g., 'script', 'design')",
        },
      ],
    },
  ],
  ui: [
    {
      name: "create_frame",
      description: "Create a container frame",
      params: [
        { name: "x", type: "number", description: "X position of the frame" },
        { name: "y", type: "number", description: "Y position of the frame" },
        { name: "width", type: "number", description: "Width of the frame" },
        { name: "height", type: "number", description: "Height of the frame" },
      ],
    },
    {
      name: "add_text",
      description: "Add a text element",
      params: [
        { name: "x", type: "number", description: "X position of the text" },
        { name: "y", type: "number", description: "Y position of the text" },
        { name: "width", type: "number", description: "Width of the text" },
        { name: "height", type: "number", description: "Height of the text" },
        { name: "content", type: "string", description: "Text content to add" },
      ],
    },
  ],
  layout: [
    {
      name: "align_elements",
      description: "Align elements horizontally",
      params: [{ name: "alignment", type: "string" }],
    },
  ],
  logic: [{ name: "add_script_node" }],
};
