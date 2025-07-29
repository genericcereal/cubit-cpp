import { Amplify } from 'aws-amplify';
import { generateClient } from 'aws-amplify/data';
import type { Schema } from '../../data/resource';

// Initialize the Amplify Data client
const client = generateClient<Schema>();

export const handler = async (event: any) => {
  console.log('CubitAI Handler event:', JSON.stringify(event, null, 2));
  
  try {
    // Extract prompt from the event
    const prompt = event.prompt || event.body?.prompt || event.arguments?.prompt;
    
    if (!prompt) {
      return {
        statusCode: 400,
        headers: {
          'Content-Type': 'application/json',
          'Access-Control-Allow-Origin': '*'
        },
        body: JSON.stringify({
          error: 'Prompt is required'
        })
      };
    }
    
    // Call the CubitAi generation
    const response = await client.generations.CubitAi({
      prompt: prompt
    });
    
    return {
      statusCode: 200,
      headers: {
        'Content-Type': 'application/json',
        'Access-Control-Allow-Origin': '*'
      },
      body: JSON.stringify({
        response: response.data,
        prompt: prompt
      })
    };
  } catch (error) {
    console.error('Error calling CubitAi:', error);
    
    return {
      statusCode: 500,
      headers: {
        'Content-Type': 'application/json',
        'Access-Control-Allow-Origin': '*'
      },
      body: JSON.stringify({
        error: 'Failed to generate AI response',
        details: error instanceof Error ? error.message : 'Unknown error'
      })
    };
  }
};