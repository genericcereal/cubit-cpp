import { defineFunction } from "@aws-amplify/backend";

export const cubitAiStreamHandler = defineFunction({
  // ✅ NO `name` here – Amplify infers the name from folder
  entry: "./src/index.js", // relative to this resource.ts
  runtime: 20, // ✅ Node 20.x (typed as NodeVersion)
  timeoutSeconds: 300,
  memoryMB: 512,
  environment: {
    APPSYNC_ENDPOINT:
      "https://enrxjqdgdvc7pkragdib6abhyy.appsync-api.us-west-2.amazonaws.com/graphql",
  },
});
