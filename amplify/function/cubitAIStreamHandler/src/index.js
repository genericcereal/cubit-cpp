const AWS = require('aws-sdk');
const https = require('https');
const { v4: uuidv4 } = require('uuid');

// NEW: Bedrock runtime client for Claude via AWS
const {
  BedrockRuntimeClient,
  InvokeModelWithResponseStreamCommand,
} = require('@aws-sdk/client-bedrock-runtime');

// Configure AWS SDK for AppSync signing
const appsync = new AWS.AppSync();
const signer = new AWS.Signers.V4(
  new AWS.Credentials(
    process.env.AWS_ACCESS_KEY_ID,
    process.env.AWS_SECRET_ACCESS_KEY,
    process.env.AWS_SESSION_TOKEN
  ),
  'appsync'
);

// AppSync Configuration
const APPSYNC_ENDPOINT = process.env.APPSYNC_ENDPOINT;
const REGION = process.env.AWS_REGION;

// Bedrock client for Claude
const bedrock = new BedrockRuntimeClient({ region: REGION });

// Helper: sign and execute an AppSync GraphQL mutation
async function executeGraphQLMutation(mutation, variables) {
  const endpoint = new URL(APPSYNC_ENDPOINT).hostname;
  const req = {
    host: endpoint,
    method: 'POST',
    path: '/graphql',
    headers: {
      'Content-Type': 'application/json',
      host: endpoint,
    },
    body: JSON.stringify({
      query: mutation,
      variables,
    }),
  };

  const signed = signer.sign(req);

  return new Promise((resolve, reject) => {
    const request = https.request(
      {
        hostname: endpoint,
        path: '/graphql',
        method: 'POST',
        headers: signed.headers,
      },
      (res) => {
        let data = '';
        res.on('data', (chunk) => (data += chunk));
        res.on('end', () => resolve(JSON.parse(data)));
      }
    );

    request.on('error', reject);
    request.write(req.body);
    request.end();
  });
}

// NEW: stream from Claude via Bedrock
async function streamFromClaude(messages, onChunk) {
  // Convert to Bedrock-compatible format
  const bedrockMessages = messages.map((m) => ({
    role: m.role,
    content: [{ type: 'text', text: m.content }],
  }));

  const payload = {
    anthropic_version: 'bedrock-2023-05-31',
    max_tokens: 4000,
    temperature: 0.7,
    messages: bedrockMessages,
  };

  const command = new InvokeModelWithResponseStreamCommand({
    modelId: 'anthropic.claude-3-5-haiku-20240620-v1:0', // Claude 3.5 Haiku
    contentType: 'application/json',
    accept: 'application/json',
    body: JSON.stringify(payload),
  });

  const response = await bedrock.send(command);

  // Stream chunks from Bedrock
  for await (const event of response.body) {
    const chunkJson = JSON.parse(new TextDecoder().decode(event.chunk.bytes));

    // Bedrock streaming returns text in different ways:
    const textDelta =
      chunkJson.delta?.text ||
      chunkJson.completion ||
      chunkJson?.output?.message?.content?.[0]?.text;

    if (textDelta) {
      await onChunk(textDelta);
    }
  }
}

const CREATE_ASSISTANT_RESPONSE_STREAM_MUTATION = `
  mutation CreateAssistantResponseStreamCubitChat($input: CreateConversationMessageCubitChatAssistantStreamingInput!) {
    createAssistantResponseStreamCubitChat(input: $input) {
      id
      conversationId
      associatedUserMessageId
      contentBlockText
      contentBlockIndex
      contentBlockDoneAtIndex
      contentBlockDeltaIndex
      stopReason
      errors {
        errorType
        message
      }
    }
  }
`;

exports.handler = async (event) => {
  console.log('Event:', JSON.stringify(event, null, 2));

  for (const record of event.Records) {
    if (record.eventName === 'INSERT' && record.dynamodb?.NewImage) {
      const newImage = AWS.DynamoDB.Converter.unmarshall(
        record.dynamodb.NewImage
      );

      if (
        newImage.__typename === 'ConversationMessageCubitChat' &&
        newImage.role === 'user' &&
        !newImage.isAIProcessed
      ) {
        const conversationId = newImage.conversationId;
        const userMessageId = newImage.id;
        
        // Extract text content from the content array
        let messageContent = '';
        if (Array.isArray(newImage.content)) {
          messageContent = newImage.content
            .filter(block => block.text)
            .map(block => block.text)
            .join('\n');
        } else if (typeof newImage.content === 'string') {
          messageContent = newImage.content;
        }

        console.log('Processing message:', {
          conversationId,
          userMessageId,
          messageContent,
        });

        try {
          // Build conversation history
          const messages = [
            { role: 'system', content: 'You are a helpful AI assistant.' },
            { role: 'user', content: messageContent },
          ];

          let contentBlockIndex = 0;
          let fullResponse = '';

          // Stream Bedrock Claude response
          await streamFromClaude(messages, async (chunk) => {
            fullResponse += chunk;

            const variables = {
              input: {
                conversationId: conversationId,
                associatedUserMessageId: userMessageId,
                contentBlockText: chunk,
                contentBlockIndex: contentBlockIndex,
                contentBlockDeltaIndex: 0,
              },
            };

            try {
              await executeGraphQLMutation(
                CREATE_ASSISTANT_RESPONSE_STREAM_MUTATION,
                variables
              );
            } catch (error) {
              console.error('Error sending chunk:', error);
            }
          });

          // Send completion signal
          const completionVariables = {
            input: {
              conversationId: conversationId,
              associatedUserMessageId: userMessageId,
              contentBlockDoneAtIndex: contentBlockIndex,
              stopReason: 'end_turn',
            },
          };

          await executeGraphQLMutation(
            CREATE_ASSISTANT_RESPONSE_STREAM_MUTATION,
            completionVariables
          );
        } catch (error) {
          console.error('Error processing AI response:', error);

          const errorVariables = {
            input: {
              conversationId: conversationId,
              associatedUserMessageId: userMessageId,
              errors: [
                {
                  errorType: 'ProcessingError',
                  message: error.message || 'Failed to generate response',
                },
              ],
            },
          };

          await executeGraphQLMutation(
            CREATE_ASSISTANT_RESPONSE_STREAM_MUTATION,
            errorVariables
          );
        }
      }
    }
  }

  return { statusCode: 200, body: 'Processed successfully' };
};
