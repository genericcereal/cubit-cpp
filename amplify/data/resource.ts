import {
  type ClientSchema,
  a,
  defineData,
  defineFunction,
} from "@aws-amplify/backend";
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

  CubitAi: a
    .generation({
      aiModel: a.ai.model("Claude 3.5 Haiku"),
      systemPrompt: "You are a helpful AI assistant.",
    })
    .arguments({
      prompt: a.string().required(),
    })
    .returns(a.string())

    .authorization((allow) => allow.authenticated()),

  // Legacy cubitAi for backward compatibility (can be removed later)

  Team: a
    .model({
      name: a.string().required(),
      members: a.string().array(), // store user IDs
    })
    .authorization((allow) => allow.owner()),

  Project: a
    .model({
      name: a.string().required(),
      teamId: a.string().required(),
      canvasData: a.json().required(), // Store canvas state as JSON
    })
    .authorization((allow) => allow.owner()),
});

export type Schema = ClientSchema<typeof schema>;

export const data = defineData({
  schema,
  authorizationModes: {
    defaultAuthorizationMode: "userPool", // ✅ Must be userPool for owner()
  },
});
