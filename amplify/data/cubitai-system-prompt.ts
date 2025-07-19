import { BASE_INSTRUCTIONS } from './prompts/base-instructions';
import { UI_PATTERNS } from './prompts/ui-patterns';
import { POSITIONING_RULES } from './prompts/positioning-rules';
import { COMMANDS } from './prompts/commands';
import { EXAMPLES } from './prompts/examples';
import { CRITICAL_RULES } from './prompts/critical-rules';
import { CANVAS_STATE } from './prompts/canvas-state';
import { COMMAND_BATCHES } from './prompts/command-batches';

export const CUBITAI_SYSTEM_PROMPT = [
  BASE_INSTRUCTIONS,
  UI_PATTERNS,
  POSITIONING_RULES,
  COMMANDS,
  EXAMPLES,
  CRITICAL_RULES,
  CANVAS_STATE,
  COMMAND_BATCHES
].join('\n');