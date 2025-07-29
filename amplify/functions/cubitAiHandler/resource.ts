import { defineFunction } from '@aws-amplify/backend';

export const cubitAiHandler = defineFunction({
  name: 'cubitAiHandler',
  entry: './handler.ts'
});