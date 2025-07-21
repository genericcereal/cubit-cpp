import { defineBackend } from "@aws-amplify/backend";
import { auth } from "./auth/resource";
import { data } from "./data/resource";
import { cubitAiStreamHandler } from "./function/cubitAIStreamHandler/resource";

/**
 * @see https://docs.amplify.aws/react/build-a-backend/ to add storage, functions, and more
 */
defineBackend({
  auth,
  data,
  cubitAiStreamHandler,
});
