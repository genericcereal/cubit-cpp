import { defineFunction } from '@aws-amplify/backend';

export const toolRegistry = defineFunction({
  name: 'toolRegistry',
  entry: './handler.ts'
});