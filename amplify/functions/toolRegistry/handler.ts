import { toolManifest } from '../../data/prompts/tool-manifest';

interface Tool {
  name: string;
  description?: string;
  category?: string;
}

export const handler = async (event: any) => {
  const category = event.queryStringParameters?.category;
  
  let tools: Tool[] = [];
  
  if (!category) {
    // Return all tools if no category specified
    tools = Object.entries(toolManifest).flatMap(([cat, toolList]) => 
      toolList.map(tool => ({ ...tool, category: cat }))
    );
  } else if (toolManifest[category as keyof typeof toolManifest]) {
    // Return tools for specific category
    tools = toolManifest[category as keyof typeof toolManifest].map(tool => 
      ({ ...tool, category })
    );
  }
  
  return {
    statusCode: 200,
    headers: {
      'Content-Type': 'application/json',
      'Access-Control-Allow-Origin': '*'
    },
    body: JSON.stringify({
      tools,
      category: category || 'all'
    })
  };
};