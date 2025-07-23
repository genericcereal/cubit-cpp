import { defineBackend } from "@aws-amplify/backend";
import { auth } from "./auth/resource";
import { data } from "./data/resource";
import { cubitAiStreamHandler } from "./functions/cubitAIStreamHandler/resource";
import { toolRegistry } from "./functions/toolRegistry/resource";
import * as lambda from "aws-cdk-lib/aws-lambda";

/**
 * @see https://docs.amplify.aws/react/build-a-backend/ to add storage, functions, and more
 */
const backend = defineBackend({
  auth,
  data,
  cubitAiStreamHandler,
  toolRegistry,
});

// ✅ unwrap the Lambda function construct
const toolRegistryLambda = backend.toolRegistry.resources.lambda;

const url = toolRegistryLambda.addFunctionUrl({
  authType: lambda.FunctionUrlAuthType.NONE,
  cors: {
    allowedOrigins: ['*'],
    allowedMethods: [lambda.HttpMethod.GET],
    allowedHeaders: ['*'],
  },
});

// ✅ Add to stack outputs
backend.addOutput({
  custom: {
    toolRegistryUrl: url.url,
  },
});
