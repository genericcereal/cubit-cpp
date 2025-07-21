import { type ClientSchema, a, defineData } from "@aws-amplify/backend";
import { CUBITAI_SYSTEM_PROMPT } from "./cubitai-system-prompt";

/*== STEP 1 ===============================================================
The section below creates the schema for the AI conversation system with
streaming support. It uses the Amplify Conversation API to enable real-time
AI responses via WebSocket subscriptions.
=========================================================================*/
const schema = a.schema({
  // Conversation API - this creates the conversation system
  CubitChat: a
    .conversation({
      aiModel: a.ai.model("Claude 3.5 Haiku"),
      systemPrompt: CUBITAI_SYSTEM_PROMPT,
      inferenceConfiguration: {
        maxTokens: 4000,
        temperature: 0.7,
      },
    })
    .authorization((allow) => allow.owner()),

  // Legacy cubitAi for backward compatibility (can be removed later)
  cubitAi: a
    .generation({
      aiModel: a.ai.model("Claude 3 Haiku"),
      systemPrompt: CUBITAI_SYSTEM_PROMPT,
    })
    .arguments({
      description: a.string(),
      canvasState: a.string(), // Current canvas state as JSON string
    })
    .returns(
      a.customType({
        message: a.string(), // Human-readable response
        commands: a.string(), // Array of command objects as JSON string
      })
    )
    .authorization((allow) => allow.authenticated()),
});

export type Schema = ClientSchema<typeof schema>;

export const data = defineData({
  schema,
  authorizationModes: {
    defaultAuthorizationMode: "userPool", // ✅ Must be userPool for owner()
  },
});
