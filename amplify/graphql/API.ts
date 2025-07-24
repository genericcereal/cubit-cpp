/* tslint:disable */
/* eslint-disable */
//  This file was automatically generated and should not be edited.

export type ConversationCubitChat = {
  __typename: "ConversationCubitChat",
  createdAt: string,
  id: string,
  messages?: ModelConversationMessageCubitChatConnection | null,
  metadata?: string | null,
  name?: string | null,
  owner?: string | null,
  updatedAt: string,
};

export type ModelConversationMessageCubitChatConnection = {
  __typename: "ModelConversationMessageCubitChatConnection",
  items:  Array<ConversationMessageCubitChat | null >,
  nextToken?: string | null,
};

export type ConversationMessageCubitChat = {
  __typename: "ConversationMessageCubitChat",
  aiContext?: string | null,
  associatedUserMessageId?: string | null,
  content?:  Array<AmplifyAIContentBlock | null > | null,
  conversation?: ConversationCubitChat | null,
  conversationId: string,
  createdAt: string,
  id: string,
  owner?: string | null,
  role?: AmplifyAIConversationParticipantRole | null,
  toolConfiguration?: AmplifyAIToolConfiguration | null,
  updatedAt: string,
};

export type AmplifyAIConversationMessage = {
  __typename: "AmplifyAIConversationMessage",
  aiContext?: string | null,
  associatedUserMessageId?: string | null,
  content?:  Array<AmplifyAIContentBlock | null > | null,
  conversationId: string,
  createdAt?: string | null,
  id: string,
  owner?: string | null,
  role?: AmplifyAIConversationParticipantRole | null,
  toolConfiguration?: AmplifyAIToolConfiguration | null,
  updatedAt?: string | null,
};

export type AmplifyAIContentBlock = {
  __typename: "AmplifyAIContentBlock",
  document?: AmplifyAIDocumentBlock | null,
  image?: AmplifyAIImageBlock | null,
  text?: string | null,
  toolResult?: AmplifyAIToolResultBlock | null,
  toolUse?: AmplifyAIToolUseBlock | null,
};

export type AmplifyAIDocumentBlock = {
  __typename: "AmplifyAIDocumentBlock",
  format: string,
  name: string,
  source: AmplifyAIDocumentBlockSource,
};

export type AmplifyAIDocumentBlockSource = {
  __typename: "AmplifyAIDocumentBlockSource",
  bytes?: string | null,
};

export type AmplifyAIImageBlock = {
  __typename: "AmplifyAIImageBlock",
  format: string,
  source: AmplifyAIImageBlockSource,
};

export type AmplifyAIImageBlockSource = {
  __typename: "AmplifyAIImageBlockSource",
  bytes?: string | null,
};

export type AmplifyAIToolResultBlock = {
  __typename: "AmplifyAIToolResultBlock",
  content:  Array<AmplifyAIToolResultContentBlock >,
  status?: string | null,
  toolUseId: string,
};

export type AmplifyAIToolResultContentBlock = {
  __typename: "AmplifyAIToolResultContentBlock",
  document?: AmplifyAIDocumentBlock | null,
  image?: AmplifyAIImageBlock | null,
  json?: string | null,
  text?: string | null,
};

export type AmplifyAIToolUseBlock = {
  __typename: "AmplifyAIToolUseBlock",
  input: string,
  name: string,
  toolUseId: string,
};

export enum AmplifyAIConversationParticipantRole {
  assistant = "assistant",
  user = "user",
}


export type AmplifyAIToolConfiguration = {
  __typename: "AmplifyAIToolConfiguration",
  tools?:  Array<AmplifyAITool | null > | null,
};

export type AmplifyAITool = {
  __typename: "AmplifyAITool",
  toolSpec?: AmplifyAIToolSpecification | null,
};

export type AmplifyAIToolSpecification = {
  __typename: "AmplifyAIToolSpecification",
  description?: string | null,
  inputSchema: AmplifyAIToolInputSchema,
  name: string,
};

export type AmplifyAIToolInputSchema = {
  __typename: "AmplifyAIToolInputSchema",
  json?: string | null,
};

export type Project = {
  __typename: "Project",
  canvasData: string,
  createdAt: string,
  id: string,
  name: string,
  owner?: string | null,
  teamId: string,
  updatedAt: string,
};

export type Team = {
  __typename: "Team",
  createdAt: string,
  id: string,
  members?: Array< string | null > | null,
  name: string,
  owner?: string | null,
  updatedAt: string,
};

export type ModelConversationCubitChatFilterInput = {
  and?: Array< ModelConversationCubitChatFilterInput | null > | null,
  createdAt?: ModelStringInput | null,
  id?: ModelIDInput | null,
  metadata?: ModelStringInput | null,
  name?: ModelStringInput | null,
  not?: ModelConversationCubitChatFilterInput | null,
  or?: Array< ModelConversationCubitChatFilterInput | null > | null,
  owner?: ModelStringInput | null,
  updatedAt?: ModelStringInput | null,
};

export type ModelStringInput = {
  attributeExists?: boolean | null,
  attributeType?: ModelAttributeTypes | null,
  beginsWith?: string | null,
  between?: Array< string | null > | null,
  contains?: string | null,
  eq?: string | null,
  ge?: string | null,
  gt?: string | null,
  le?: string | null,
  lt?: string | null,
  ne?: string | null,
  notContains?: string | null,
  size?: ModelSizeInput | null,
};

export enum ModelAttributeTypes {
  _null = "_null",
  binary = "binary",
  binarySet = "binarySet",
  bool = "bool",
  list = "list",
  map = "map",
  number = "number",
  numberSet = "numberSet",
  string = "string",
  stringSet = "stringSet",
}


export type ModelSizeInput = {
  between?: Array< number | null > | null,
  eq?: number | null,
  ge?: number | null,
  gt?: number | null,
  le?: number | null,
  lt?: number | null,
  ne?: number | null,
};

export type ModelIDInput = {
  attributeExists?: boolean | null,
  attributeType?: ModelAttributeTypes | null,
  beginsWith?: string | null,
  between?: Array< string | null > | null,
  contains?: string | null,
  eq?: string | null,
  ge?: string | null,
  gt?: string | null,
  le?: string | null,
  lt?: string | null,
  ne?: string | null,
  notContains?: string | null,
  size?: ModelSizeInput | null,
};

export type ModelConversationCubitChatConnection = {
  __typename: "ModelConversationCubitChatConnection",
  items:  Array<ConversationCubitChat | null >,
  nextToken?: string | null,
};

export type ModelConversationMessageCubitChatFilterInput = {
  aiContext?: ModelStringInput | null,
  and?: Array< ModelConversationMessageCubitChatFilterInput | null > | null,
  associatedUserMessageId?: ModelIDInput | null,
  conversationId?: ModelIDInput | null,
  createdAt?: ModelStringInput | null,
  id?: ModelIDInput | null,
  not?: ModelConversationMessageCubitChatFilterInput | null,
  or?: Array< ModelConversationMessageCubitChatFilterInput | null > | null,
  owner?: ModelStringInput | null,
  role?: ModelAmplifyAIConversationParticipantRoleInput | null,
  updatedAt?: ModelStringInput | null,
};

export type ModelAmplifyAIConversationParticipantRoleInput = {
  eq?: AmplifyAIConversationParticipantRole | null,
  ne?: AmplifyAIConversationParticipantRole | null,
};

export type ModelProjectFilterInput = {
  and?: Array< ModelProjectFilterInput | null > | null,
  canvasData?: ModelStringInput | null,
  createdAt?: ModelStringInput | null,
  id?: ModelIDInput | null,
  name?: ModelStringInput | null,
  not?: ModelProjectFilterInput | null,
  or?: Array< ModelProjectFilterInput | null > | null,
  owner?: ModelStringInput | null,
  teamId?: ModelStringInput | null,
  updatedAt?: ModelStringInput | null,
};

export type ModelProjectConnection = {
  __typename: "ModelProjectConnection",
  items:  Array<Project | null >,
  nextToken?: string | null,
};

export type ModelTeamFilterInput = {
  and?: Array< ModelTeamFilterInput | null > | null,
  createdAt?: ModelStringInput | null,
  id?: ModelIDInput | null,
  members?: ModelStringInput | null,
  name?: ModelStringInput | null,
  not?: ModelTeamFilterInput | null,
  or?: Array< ModelTeamFilterInput | null > | null,
  owner?: ModelStringInput | null,
  updatedAt?: ModelStringInput | null,
};

export type ModelTeamConnection = {
  __typename: "ModelTeamConnection",
  items:  Array<Team | null >,
  nextToken?: string | null,
};

export type AmplifyAIContentBlockInput = {
  document?: AmplifyAIDocumentBlockInput | null,
  image?: AmplifyAIImageBlockInput | null,
  text?: string | null,
  toolResult?: AmplifyAIToolResultBlockInput | null,
  toolUse?: AmplifyAIToolUseBlockInput | null,
};

export type AmplifyAIDocumentBlockInput = {
  format: string,
  name: string,
  source: AmplifyAIDocumentBlockSourceInput,
};

export type AmplifyAIDocumentBlockSourceInput = {
  bytes?: string | null,
};

export type AmplifyAIImageBlockInput = {
  format: string,
  source: AmplifyAIImageBlockSourceInput,
};

export type AmplifyAIImageBlockSourceInput = {
  bytes?: string | null,
};

export type AmplifyAIToolResultBlockInput = {
  content: Array< AmplifyAIToolResultContentBlockInput >,
  status?: string | null,
  toolUseId: string,
};

export type AmplifyAIToolResultContentBlockInput = {
  document?: AmplifyAIDocumentBlockInput | null,
  image?: AmplifyAIImageBlockInput | null,
  json?: string | null,
  text?: string | null,
};

export type AmplifyAIToolUseBlockInput = {
  input: string,
  name: string,
  toolUseId: string,
};

export type AmplifyAIToolConfigurationInput = {
  tools?: Array< AmplifyAIToolInput | null > | null,
};

export type AmplifyAIToolInput = {
  toolSpec?: AmplifyAIToolSpecificationInput | null,
};

export type AmplifyAIToolSpecificationInput = {
  description?: string | null,
  inputSchema: AmplifyAIToolInputSchemaInput,
  name: string,
};

export type AmplifyAIToolInputSchemaInput = {
  json?: string | null,
};

export type CreateConversationMessageCubitChatAssistantInput = {
  associatedUserMessageId?: string | null,
  content?: Array< AmplifyAIContentBlockInput | null > | null,
  conversationId?: string | null,
};

export type CreateConversationMessageCubitChatAssistantStreamingInput = {
  accumulatedTurnContent?: Array< AmplifyAIContentBlockInput | null > | null,
  associatedUserMessageId: string,
  contentBlockDeltaIndex?: number | null,
  contentBlockDoneAtIndex?: number | null,
  contentBlockIndex?: number | null,
  contentBlockText?: string | null,
  contentBlockToolUse?: string | null,
  conversationId: string,
  errors?: Array< AmplifyAIConversationTurnErrorInput | null > | null,
  p?: string | null,
  stopReason?: string | null,
};

export type AmplifyAIConversationTurnErrorInput = {
  errorType: string,
  message: string,
};

export type AmplifyAIConversationMessageStreamPart = {
  __typename: "AmplifyAIConversationMessageStreamPart",
  associatedUserMessageId: string,
  contentBlockDeltaIndex?: number | null,
  contentBlockDoneAtIndex?: number | null,
  contentBlockIndex?: number | null,
  contentBlockText?: string | null,
  contentBlockToolUse?: AmplifyAIToolUseBlock | null,
  conversationId: string,
  errors?:  Array<AmplifyAIConversationTurnError | null > | null,
  id: string,
  owner?: string | null,
  p?: string | null,
  stopReason?: string | null,
};

export type AmplifyAIConversationTurnError = {
  __typename: "AmplifyAIConversationTurnError",
  errorType: string,
  message: string,
};

export type ModelConversationCubitChatConditionInput = {
  and?: Array< ModelConversationCubitChatConditionInput | null > | null,
  createdAt?: ModelStringInput | null,
  metadata?: ModelStringInput | null,
  name?: ModelStringInput | null,
  not?: ModelConversationCubitChatConditionInput | null,
  or?: Array< ModelConversationCubitChatConditionInput | null > | null,
  owner?: ModelStringInput | null,
  updatedAt?: ModelStringInput | null,
};

export type CreateConversationCubitChatInput = {
  id?: string | null,
  metadata?: string | null,
  name?: string | null,
};

export type ModelConversationMessageCubitChatConditionInput = {
  aiContext?: ModelStringInput | null,
  and?: Array< ModelConversationMessageCubitChatConditionInput | null > | null,
  associatedUserMessageId?: ModelIDInput | null,
  conversationId?: ModelIDInput | null,
  createdAt?: ModelStringInput | null,
  not?: ModelConversationMessageCubitChatConditionInput | null,
  or?: Array< ModelConversationMessageCubitChatConditionInput | null > | null,
  owner?: ModelStringInput | null,
  role?: ModelAmplifyAIConversationParticipantRoleInput | null,
  updatedAt?: ModelStringInput | null,
};

export type CreateConversationMessageCubitChatInput = {
  aiContext?: string | null,
  associatedUserMessageId?: string | null,
  content?: Array< AmplifyAIContentBlockInput | null > | null,
  conversationId: string,
  id?: string | null,
  role?: AmplifyAIConversationParticipantRole | null,
  toolConfiguration?: AmplifyAIToolConfigurationInput | null,
};

export type ModelProjectConditionInput = {
  and?: Array< ModelProjectConditionInput | null > | null,
  canvasData?: ModelStringInput | null,
  createdAt?: ModelStringInput | null,
  name?: ModelStringInput | null,
  not?: ModelProjectConditionInput | null,
  or?: Array< ModelProjectConditionInput | null > | null,
  owner?: ModelStringInput | null,
  teamId?: ModelStringInput | null,
  updatedAt?: ModelStringInput | null,
};

export type CreateProjectInput = {
  canvasData: string,
  id?: string | null,
  name: string,
  teamId: string,
};

export type ModelTeamConditionInput = {
  and?: Array< ModelTeamConditionInput | null > | null,
  createdAt?: ModelStringInput | null,
  members?: ModelStringInput | null,
  name?: ModelStringInput | null,
  not?: ModelTeamConditionInput | null,
  or?: Array< ModelTeamConditionInput | null > | null,
  owner?: ModelStringInput | null,
  updatedAt?: ModelStringInput | null,
};

export type CreateTeamInput = {
  id?: string | null,
  members?: Array< string | null > | null,
  name: string,
};

export type DeleteConversationCubitChatInput = {
  id: string,
};

export type DeleteConversationMessageCubitChatInput = {
  id: string,
};

export type DeleteProjectInput = {
  id: string,
};

export type DeleteTeamInput = {
  id: string,
};

export type UpdateConversationCubitChatInput = {
  id: string,
  metadata?: string | null,
  name?: string | null,
};

export type UpdateProjectInput = {
  canvasData?: string | null,
  id: string,
  name?: string | null,
  teamId?: string | null,
};

export type UpdateTeamInput = {
  id: string,
  members?: Array< string | null > | null,
  name?: string | null,
};

export type ModelSubscriptionConversationMessageCubitChatFilterInput = {
  aiContext?: ModelSubscriptionStringInput | null,
  and?: Array< ModelSubscriptionConversationMessageCubitChatFilterInput | null > | null,
  associatedUserMessageId?: ModelSubscriptionIDInput | null,
  conversationId?: ModelSubscriptionIDInput | null,
  createdAt?: ModelSubscriptionStringInput | null,
  id?: ModelSubscriptionIDInput | null,
  or?: Array< ModelSubscriptionConversationMessageCubitChatFilterInput | null > | null,
  owner?: ModelStringInput | null,
  role?: ModelSubscriptionStringInput | null,
  updatedAt?: ModelSubscriptionStringInput | null,
};

export type ModelSubscriptionStringInput = {
  beginsWith?: string | null,
  between?: Array< string | null > | null,
  contains?: string | null,
  eq?: string | null,
  ge?: string | null,
  gt?: string | null,
  in?: Array< string | null > | null,
  le?: string | null,
  lt?: string | null,
  ne?: string | null,
  notContains?: string | null,
  notIn?: Array< string | null > | null,
};

export type ModelSubscriptionIDInput = {
  beginsWith?: string | null,
  between?: Array< string | null > | null,
  contains?: string | null,
  eq?: string | null,
  ge?: string | null,
  gt?: string | null,
  in?: Array< string | null > | null,
  le?: string | null,
  lt?: string | null,
  ne?: string | null,
  notContains?: string | null,
  notIn?: Array< string | null > | null,
};

export type ModelSubscriptionProjectFilterInput = {
  and?: Array< ModelSubscriptionProjectFilterInput | null > | null,
  canvasData?: ModelSubscriptionStringInput | null,
  createdAt?: ModelSubscriptionStringInput | null,
  id?: ModelSubscriptionIDInput | null,
  name?: ModelSubscriptionStringInput | null,
  or?: Array< ModelSubscriptionProjectFilterInput | null > | null,
  owner?: ModelStringInput | null,
  teamId?: ModelSubscriptionStringInput | null,
  updatedAt?: ModelSubscriptionStringInput | null,
};

export type ModelSubscriptionTeamFilterInput = {
  and?: Array< ModelSubscriptionTeamFilterInput | null > | null,
  createdAt?: ModelSubscriptionStringInput | null,
  id?: ModelSubscriptionIDInput | null,
  members?: ModelSubscriptionStringInput | null,
  name?: ModelSubscriptionStringInput | null,
  or?: Array< ModelSubscriptionTeamFilterInput | null > | null,
  owner?: ModelStringInput | null,
  updatedAt?: ModelSubscriptionStringInput | null,
};

export type GetConversationCubitChatQueryVariables = {
  id: string,
};

export type GetConversationCubitChatQuery = {
  getConversationCubitChat?:  {
    __typename: "ConversationCubitChat",
    createdAt: string,
    id: string,
    messages?:  {
      __typename: "ModelConversationMessageCubitChatConnection",
      nextToken?: string | null,
    } | null,
    metadata?: string | null,
    name?: string | null,
    owner?: string | null,
    updatedAt: string,
  } | null,
};

export type GetConversationMessageCubitChatQueryVariables = {
  id: string,
};

export type GetConversationMessageCubitChatQuery = {
  getConversationMessageCubitChat?:  {
    __typename: "ConversationMessageCubitChat",
    aiContext?: string | null,
    associatedUserMessageId?: string | null,
    content?:  Array< {
      __typename: "AmplifyAIContentBlock",
      text?: string | null,
    } | null > | null,
    conversation?:  {
      __typename: "ConversationCubitChat",
      createdAt: string,
      id: string,
      metadata?: string | null,
      name?: string | null,
      owner?: string | null,
      updatedAt: string,
    } | null,
    conversationId: string,
    createdAt: string,
    id: string,
    owner?: string | null,
    role?: AmplifyAIConversationParticipantRole | null,
    toolConfiguration?:  {
      __typename: "AmplifyAIToolConfiguration",
    } | null,
    updatedAt: string,
  } | null,
};

export type GetProjectQueryVariables = {
  id: string,
};

export type GetProjectQuery = {
  getProject?:  {
    __typename: "Project",
    canvasData: string,
    createdAt: string,
    id: string,
    name: string,
    owner?: string | null,
    teamId: string,
    updatedAt: string,
  } | null,
};

export type GetTeamQueryVariables = {
  id: string,
};

export type GetTeamQuery = {
  getTeam?:  {
    __typename: "Team",
    createdAt: string,
    id: string,
    members?: Array< string | null > | null,
    name: string,
    owner?: string | null,
    updatedAt: string,
  } | null,
};

export type ListConversationCubitChatsQueryVariables = {
  filter?: ModelConversationCubitChatFilterInput | null,
  limit?: number | null,
  nextToken?: string | null,
};

export type ListConversationCubitChatsQuery = {
  listConversationCubitChats?:  {
    __typename: "ModelConversationCubitChatConnection",
    items:  Array< {
      __typename: "ConversationCubitChat",
      createdAt: string,
      id: string,
      metadata?: string | null,
      name?: string | null,
      owner?: string | null,
      updatedAt: string,
    } | null >,
    nextToken?: string | null,
  } | null,
};

export type ListConversationMessageCubitChatsQueryVariables = {
  filter?: ModelConversationMessageCubitChatFilterInput | null,
  limit?: number | null,
  nextToken?: string | null,
};

export type ListConversationMessageCubitChatsQuery = {
  listConversationMessageCubitChats?:  {
    __typename: "ModelConversationMessageCubitChatConnection",
    items:  Array< {
      __typename: "ConversationMessageCubitChat",
      aiContext?: string | null,
      associatedUserMessageId?: string | null,
      conversationId: string,
      createdAt: string,
      id: string,
      owner?: string | null,
      role?: AmplifyAIConversationParticipantRole | null,
      updatedAt: string,
    } | null >,
    nextToken?: string | null,
  } | null,
};

export type ListProjectsQueryVariables = {
  filter?: ModelProjectFilterInput | null,
  limit?: number | null,
  nextToken?: string | null,
};

export type ListProjectsQuery = {
  listProjects?:  {
    __typename: "ModelProjectConnection",
    items:  Array< {
      __typename: "Project",
      canvasData: string,
      createdAt: string,
      id: string,
      name: string,
      owner?: string | null,
      teamId: string,
      updatedAt: string,
    } | null >,
    nextToken?: string | null,
  } | null,
};

export type ListTeamsQueryVariables = {
  filter?: ModelTeamFilterInput | null,
  limit?: number | null,
  nextToken?: string | null,
};

export type ListTeamsQuery = {
  listTeams?:  {
    __typename: "ModelTeamConnection",
    items:  Array< {
      __typename: "Team",
      createdAt: string,
      id: string,
      members?: Array< string | null > | null,
      name: string,
      owner?: string | null,
      updatedAt: string,
    } | null >,
    nextToken?: string | null,
  } | null,
};

export type CubitChatMutationVariables = {
  aiContext?: string | null,
  content?: Array< AmplifyAIContentBlockInput | null > | null,
  conversationId: string,
  toolConfiguration?: AmplifyAIToolConfigurationInput | null,
};

export type CubitChatMutation = {
  CubitChat: ( {
      __typename: "ConversationMessageCubitChat",
      aiContext?: string | null,
      associatedUserMessageId?: string | null,
      content?:  Array< {
        __typename: "AmplifyAIContentBlock",
        text?: string | null,
      } | null > | null,
      conversationId: string,
      createdAt?: string | null,
      id: string,
      owner?: string | null,
      role?: AmplifyAIConversationParticipantRole | null,
      toolConfiguration?:  {
        __typename: "AmplifyAIToolConfiguration",
      } | null,
      updatedAt?: string | null,
      conversation?:  {
        __typename: "ConversationCubitChat",
        createdAt: string,
        id: string,
        metadata?: string | null,
        name?: string | null,
        owner?: string | null,
        updatedAt: string,
      } | null,
    }
  ) | null,
};

export type CreateAssistantResponseCubitChatMutationVariables = {
  input: CreateConversationMessageCubitChatAssistantInput,
};

export type CreateAssistantResponseCubitChatMutation = {
  createAssistantResponseCubitChat?:  {
    __typename: "ConversationMessageCubitChat",
    aiContext?: string | null,
    associatedUserMessageId?: string | null,
    content?:  Array< {
      __typename: "AmplifyAIContentBlock",
      text?: string | null,
    } | null > | null,
    conversation?:  {
      __typename: "ConversationCubitChat",
      createdAt: string,
      id: string,
      metadata?: string | null,
      name?: string | null,
      owner?: string | null,
      updatedAt: string,
    } | null,
    conversationId: string,
    createdAt: string,
    id: string,
    owner?: string | null,
    role?: AmplifyAIConversationParticipantRole | null,
    toolConfiguration?:  {
      __typename: "AmplifyAIToolConfiguration",
    } | null,
    updatedAt: string,
  } | null,
};

export type CreateAssistantResponseStreamCubitChatMutationVariables = {
  input: CreateConversationMessageCubitChatAssistantStreamingInput,
};

export type CreateAssistantResponseStreamCubitChatMutation = {
  createAssistantResponseStreamCubitChat?:  {
    __typename: "AmplifyAIConversationMessageStreamPart",
    associatedUserMessageId: string,
    contentBlockDeltaIndex?: number | null,
    contentBlockDoneAtIndex?: number | null,
    contentBlockIndex?: number | null,
    contentBlockText?: string | null,
    contentBlockToolUse?:  {
      __typename: "AmplifyAIToolUseBlock",
      input: string,
      name: string,
      toolUseId: string,
    } | null,
    conversationId: string,
    errors?:  Array< {
      __typename: "AmplifyAIConversationTurnError",
      errorType: string,
      message: string,
    } | null > | null,
    id: string,
    owner?: string | null,
    p?: string | null,
    stopReason?: string | null,
  } | null,
};

export type CreateConversationCubitChatMutationVariables = {
  condition?: ModelConversationCubitChatConditionInput | null,
  input: CreateConversationCubitChatInput,
};

export type CreateConversationCubitChatMutation = {
  createConversationCubitChat?:  {
    __typename: "ConversationCubitChat",
    createdAt: string,
    id: string,
    messages?:  {
      __typename: "ModelConversationMessageCubitChatConnection",
      nextToken?: string | null,
    } | null,
    metadata?: string | null,
    name?: string | null,
    owner?: string | null,
    updatedAt: string,
  } | null,
};

export type CreateConversationMessageCubitChatMutationVariables = {
  condition?: ModelConversationMessageCubitChatConditionInput | null,
  input: CreateConversationMessageCubitChatInput,
};

export type CreateConversationMessageCubitChatMutation = {
  createConversationMessageCubitChat?:  {
    __typename: "ConversationMessageCubitChat",
    aiContext?: string | null,
    associatedUserMessageId?: string | null,
    content?:  Array< {
      __typename: "AmplifyAIContentBlock",
      text?: string | null,
    } | null > | null,
    conversation?:  {
      __typename: "ConversationCubitChat",
      createdAt: string,
      id: string,
      metadata?: string | null,
      name?: string | null,
      owner?: string | null,
      updatedAt: string,
    } | null,
    conversationId: string,
    createdAt: string,
    id: string,
    owner?: string | null,
    role?: AmplifyAIConversationParticipantRole | null,
    toolConfiguration?:  {
      __typename: "AmplifyAIToolConfiguration",
    } | null,
    updatedAt: string,
  } | null,
};

export type CreateProjectMutationVariables = {
  condition?: ModelProjectConditionInput | null,
  input: CreateProjectInput,
};

export type CreateProjectMutation = {
  createProject?:  {
    __typename: "Project",
    canvasData: string,
    createdAt: string,
    id: string,
    name: string,
    owner?: string | null,
    teamId: string,
    updatedAt: string,
  } | null,
};

export type CreateTeamMutationVariables = {
  condition?: ModelTeamConditionInput | null,
  input: CreateTeamInput,
};

export type CreateTeamMutation = {
  createTeam?:  {
    __typename: "Team",
    createdAt: string,
    id: string,
    members?: Array< string | null > | null,
    name: string,
    owner?: string | null,
    updatedAt: string,
  } | null,
};

export type DeleteConversationCubitChatMutationVariables = {
  condition?: ModelConversationCubitChatConditionInput | null,
  input: DeleteConversationCubitChatInput,
};

export type DeleteConversationCubitChatMutation = {
  deleteConversationCubitChat?:  {
    __typename: "ConversationCubitChat",
    createdAt: string,
    id: string,
    messages?:  {
      __typename: "ModelConversationMessageCubitChatConnection",
      nextToken?: string | null,
    } | null,
    metadata?: string | null,
    name?: string | null,
    owner?: string | null,
    updatedAt: string,
  } | null,
};

export type DeleteConversationMessageCubitChatMutationVariables = {
  condition?: ModelConversationMessageCubitChatConditionInput | null,
  input: DeleteConversationMessageCubitChatInput,
};

export type DeleteConversationMessageCubitChatMutation = {
  deleteConversationMessageCubitChat?:  {
    __typename: "ConversationMessageCubitChat",
    aiContext?: string | null,
    associatedUserMessageId?: string | null,
    content?:  Array< {
      __typename: "AmplifyAIContentBlock",
      text?: string | null,
    } | null > | null,
    conversation?:  {
      __typename: "ConversationCubitChat",
      createdAt: string,
      id: string,
      metadata?: string | null,
      name?: string | null,
      owner?: string | null,
      updatedAt: string,
    } | null,
    conversationId: string,
    createdAt: string,
    id: string,
    owner?: string | null,
    role?: AmplifyAIConversationParticipantRole | null,
    toolConfiguration?:  {
      __typename: "AmplifyAIToolConfiguration",
    } | null,
    updatedAt: string,
  } | null,
};

export type DeleteProjectMutationVariables = {
  condition?: ModelProjectConditionInput | null,
  input: DeleteProjectInput,
};

export type DeleteProjectMutation = {
  deleteProject?:  {
    __typename: "Project",
    canvasData: string,
    createdAt: string,
    id: string,
    name: string,
    owner?: string | null,
    teamId: string,
    updatedAt: string,
  } | null,
};

export type DeleteTeamMutationVariables = {
  condition?: ModelTeamConditionInput | null,
  input: DeleteTeamInput,
};

export type DeleteTeamMutation = {
  deleteTeam?:  {
    __typename: "Team",
    createdAt: string,
    id: string,
    members?: Array< string | null > | null,
    name: string,
    owner?: string | null,
    updatedAt: string,
  } | null,
};

export type UpdateConversationCubitChatMutationVariables = {
  condition?: ModelConversationCubitChatConditionInput | null,
  input: UpdateConversationCubitChatInput,
};

export type UpdateConversationCubitChatMutation = {
  updateConversationCubitChat?:  {
    __typename: "ConversationCubitChat",
    createdAt: string,
    id: string,
    messages?:  {
      __typename: "ModelConversationMessageCubitChatConnection",
      nextToken?: string | null,
    } | null,
    metadata?: string | null,
    name?: string | null,
    owner?: string | null,
    updatedAt: string,
  } | null,
};

export type UpdateProjectMutationVariables = {
  condition?: ModelProjectConditionInput | null,
  input: UpdateProjectInput,
};

export type UpdateProjectMutation = {
  updateProject?:  {
    __typename: "Project",
    canvasData: string,
    createdAt: string,
    id: string,
    name: string,
    owner?: string | null,
    teamId: string,
    updatedAt: string,
  } | null,
};

export type UpdateTeamMutationVariables = {
  condition?: ModelTeamConditionInput | null,
  input: UpdateTeamInput,
};

export type UpdateTeamMutation = {
  updateTeam?:  {
    __typename: "Team",
    createdAt: string,
    id: string,
    members?: Array< string | null > | null,
    name: string,
    owner?: string | null,
    updatedAt: string,
  } | null,
};

export type OnCreateAssistantResponseCubitChatSubscriptionVariables = {
  conversationId?: string | null,
};

export type OnCreateAssistantResponseCubitChatSubscription = {
  onCreateAssistantResponseCubitChat?:  {
    __typename: "AmplifyAIConversationMessageStreamPart",
    associatedUserMessageId: string,
    contentBlockDeltaIndex?: number | null,
    contentBlockDoneAtIndex?: number | null,
    contentBlockIndex?: number | null,
    contentBlockText?: string | null,
    contentBlockToolUse?:  {
      __typename: "AmplifyAIToolUseBlock",
      input: string,
      name: string,
      toolUseId: string,
    } | null,
    conversationId: string,
    errors?:  Array< {
      __typename: "AmplifyAIConversationTurnError",
      errorType: string,
      message: string,
    } | null > | null,
    id: string,
    owner?: string | null,
    p?: string | null,
    stopReason?: string | null,
  } | null,
};

export type OnCreateConversationMessageCubitChatSubscriptionVariables = {
  filter?: ModelSubscriptionConversationMessageCubitChatFilterInput | null,
  owner?: string | null,
};

export type OnCreateConversationMessageCubitChatSubscription = {
  onCreateConversationMessageCubitChat?:  {
    __typename: "ConversationMessageCubitChat",
    aiContext?: string | null,
    associatedUserMessageId?: string | null,
    content?:  Array< {
      __typename: "AmplifyAIContentBlock",
      text?: string | null,
    } | null > | null,
    conversation?:  {
      __typename: "ConversationCubitChat",
      createdAt: string,
      id: string,
      metadata?: string | null,
      name?: string | null,
      owner?: string | null,
      updatedAt: string,
    } | null,
    conversationId: string,
    createdAt: string,
    id: string,
    owner?: string | null,
    role?: AmplifyAIConversationParticipantRole | null,
    toolConfiguration?:  {
      __typename: "AmplifyAIToolConfiguration",
    } | null,
    updatedAt: string,
  } | null,
};

export type OnCreateProjectSubscriptionVariables = {
  filter?: ModelSubscriptionProjectFilterInput | null,
  owner?: string | null,
};

export type OnCreateProjectSubscription = {
  onCreateProject?:  {
    __typename: "Project",
    canvasData: string,
    createdAt: string,
    id: string,
    name: string,
    owner?: string | null,
    teamId: string,
    updatedAt: string,
  } | null,
};

export type OnCreateTeamSubscriptionVariables = {
  filter?: ModelSubscriptionTeamFilterInput | null,
  owner?: string | null,
};

export type OnCreateTeamSubscription = {
  onCreateTeam?:  {
    __typename: "Team",
    createdAt: string,
    id: string,
    members?: Array< string | null > | null,
    name: string,
    owner?: string | null,
    updatedAt: string,
  } | null,
};

export type OnDeleteProjectSubscriptionVariables = {
  filter?: ModelSubscriptionProjectFilterInput | null,
  owner?: string | null,
};

export type OnDeleteProjectSubscription = {
  onDeleteProject?:  {
    __typename: "Project",
    canvasData: string,
    createdAt: string,
    id: string,
    name: string,
    owner?: string | null,
    teamId: string,
    updatedAt: string,
  } | null,
};

export type OnDeleteTeamSubscriptionVariables = {
  filter?: ModelSubscriptionTeamFilterInput | null,
  owner?: string | null,
};

export type OnDeleteTeamSubscription = {
  onDeleteTeam?:  {
    __typename: "Team",
    createdAt: string,
    id: string,
    members?: Array< string | null > | null,
    name: string,
    owner?: string | null,
    updatedAt: string,
  } | null,
};

export type OnUpdateProjectSubscriptionVariables = {
  filter?: ModelSubscriptionProjectFilterInput | null,
  owner?: string | null,
};

export type OnUpdateProjectSubscription = {
  onUpdateProject?:  {
    __typename: "Project",
    canvasData: string,
    createdAt: string,
    id: string,
    name: string,
    owner?: string | null,
    teamId: string,
    updatedAt: string,
  } | null,
};

export type OnUpdateTeamSubscriptionVariables = {
  filter?: ModelSubscriptionTeamFilterInput | null,
  owner?: string | null,
};

export type OnUpdateTeamSubscription = {
  onUpdateTeam?:  {
    __typename: "Team",
    createdAt: string,
    id: string,
    members?: Array< string | null > | null,
    name: string,
    owner?: string | null,
    updatedAt: string,
  } | null,
};
