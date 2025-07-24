/* tslint:disable */
/* eslint-disable */
// this is an auto generated file. This will be overwritten

import * as APITypes from "./API";
type GeneratedMutation<InputType, OutputType> = string & {
  __generatedMutationInput: InputType;
  __generatedMutationOutput: OutputType;
};

export const CubitChat = /* GraphQL */ `mutation CubitChat(
  $aiContext: AWSJSON
  $content: [AmplifyAIContentBlockInput]
  $conversationId: ID!
  $toolConfiguration: AmplifyAIToolConfigurationInput
) {
  CubitChat(
    aiContext: $aiContext
    content: $content
    conversationId: $conversationId
    toolConfiguration: $toolConfiguration
  ) {
    aiContext
    associatedUserMessageId
    content {
      text
      __typename
    }
    conversationId
    createdAt
    id
    owner
    role
    toolConfiguration {
      __typename
    }
    updatedAt

    ... on ConversationMessageCubitChat {
      conversation {
        createdAt
        id
        metadata
        name
        owner
        updatedAt
        __typename
      }
    }
  }
}
` as GeneratedMutation<
  APITypes.CubitChatMutationVariables,
  APITypes.CubitChatMutation
>;
export const createAssistantResponseCubitChat = /* GraphQL */ `mutation CreateAssistantResponseCubitChat(
  $input: CreateConversationMessageCubitChatAssistantInput!
) {
  createAssistantResponseCubitChat(input: $input) {
    aiContext
    associatedUserMessageId
    content {
      text
      __typename
    }
    conversation {
      createdAt
      id
      metadata
      name
      owner
      updatedAt
      __typename
    }
    conversationId
    createdAt
    id
    owner
    role
    toolConfiguration {
      __typename
    }
    updatedAt
    __typename
  }
}
` as GeneratedMutation<
  APITypes.CreateAssistantResponseCubitChatMutationVariables,
  APITypes.CreateAssistantResponseCubitChatMutation
>;
export const createAssistantResponseStreamCubitChat = /* GraphQL */ `mutation CreateAssistantResponseStreamCubitChat(
  $input: CreateConversationMessageCubitChatAssistantStreamingInput!
) {
  createAssistantResponseStreamCubitChat(input: $input) {
    associatedUserMessageId
    contentBlockDeltaIndex
    contentBlockDoneAtIndex
    contentBlockIndex
    contentBlockText
    contentBlockToolUse {
      input
      name
      toolUseId
      __typename
    }
    conversationId
    errors {
      errorType
      message
      __typename
    }
    id
    owner
    p
    stopReason
    __typename
  }
}
` as GeneratedMutation<
  APITypes.CreateAssistantResponseStreamCubitChatMutationVariables,
  APITypes.CreateAssistantResponseStreamCubitChatMutation
>;
export const createConversationCubitChat = /* GraphQL */ `mutation CreateConversationCubitChat(
  $condition: ModelConversationCubitChatConditionInput
  $input: CreateConversationCubitChatInput!
) {
  createConversationCubitChat(condition: $condition, input: $input) {
    createdAt
    id
    messages {
      nextToken
      __typename
    }
    metadata
    name
    owner
    updatedAt
    __typename
  }
}
` as GeneratedMutation<
  APITypes.CreateConversationCubitChatMutationVariables,
  APITypes.CreateConversationCubitChatMutation
>;
export const createConversationMessageCubitChat = /* GraphQL */ `mutation CreateConversationMessageCubitChat(
  $condition: ModelConversationMessageCubitChatConditionInput
  $input: CreateConversationMessageCubitChatInput!
) {
  createConversationMessageCubitChat(condition: $condition, input: $input) {
    aiContext
    associatedUserMessageId
    content {
      text
      __typename
    }
    conversation {
      createdAt
      id
      metadata
      name
      owner
      updatedAt
      __typename
    }
    conversationId
    createdAt
    id
    owner
    role
    toolConfiguration {
      __typename
    }
    updatedAt
    __typename
  }
}
` as GeneratedMutation<
  APITypes.CreateConversationMessageCubitChatMutationVariables,
  APITypes.CreateConversationMessageCubitChatMutation
>;
export const createProject = /* GraphQL */ `mutation CreateProject(
  $condition: ModelProjectConditionInput
  $input: CreateProjectInput!
) {
  createProject(condition: $condition, input: $input) {
    canvasData
    createdAt
    id
    name
    owner
    teamId
    updatedAt
    __typename
  }
}
` as GeneratedMutation<
  APITypes.CreateProjectMutationVariables,
  APITypes.CreateProjectMutation
>;
export const createTeam = /* GraphQL */ `mutation CreateTeam(
  $condition: ModelTeamConditionInput
  $input: CreateTeamInput!
) {
  createTeam(condition: $condition, input: $input) {
    createdAt
    id
    members
    name
    owner
    updatedAt
    __typename
  }
}
` as GeneratedMutation<
  APITypes.CreateTeamMutationVariables,
  APITypes.CreateTeamMutation
>;
export const deleteConversationCubitChat = /* GraphQL */ `mutation DeleteConversationCubitChat(
  $condition: ModelConversationCubitChatConditionInput
  $input: DeleteConversationCubitChatInput!
) {
  deleteConversationCubitChat(condition: $condition, input: $input) {
    createdAt
    id
    messages {
      nextToken
      __typename
    }
    metadata
    name
    owner
    updatedAt
    __typename
  }
}
` as GeneratedMutation<
  APITypes.DeleteConversationCubitChatMutationVariables,
  APITypes.DeleteConversationCubitChatMutation
>;
export const deleteConversationMessageCubitChat = /* GraphQL */ `mutation DeleteConversationMessageCubitChat(
  $condition: ModelConversationMessageCubitChatConditionInput
  $input: DeleteConversationMessageCubitChatInput!
) {
  deleteConversationMessageCubitChat(condition: $condition, input: $input) {
    aiContext
    associatedUserMessageId
    content {
      text
      __typename
    }
    conversation {
      createdAt
      id
      metadata
      name
      owner
      updatedAt
      __typename
    }
    conversationId
    createdAt
    id
    owner
    role
    toolConfiguration {
      __typename
    }
    updatedAt
    __typename
  }
}
` as GeneratedMutation<
  APITypes.DeleteConversationMessageCubitChatMutationVariables,
  APITypes.DeleteConversationMessageCubitChatMutation
>;
export const deleteProject = /* GraphQL */ `mutation DeleteProject(
  $condition: ModelProjectConditionInput
  $input: DeleteProjectInput!
) {
  deleteProject(condition: $condition, input: $input) {
    canvasData
    createdAt
    id
    name
    owner
    teamId
    updatedAt
    __typename
  }
}
` as GeneratedMutation<
  APITypes.DeleteProjectMutationVariables,
  APITypes.DeleteProjectMutation
>;
export const deleteTeam = /* GraphQL */ `mutation DeleteTeam(
  $condition: ModelTeamConditionInput
  $input: DeleteTeamInput!
) {
  deleteTeam(condition: $condition, input: $input) {
    createdAt
    id
    members
    name
    owner
    updatedAt
    __typename
  }
}
` as GeneratedMutation<
  APITypes.DeleteTeamMutationVariables,
  APITypes.DeleteTeamMutation
>;
export const updateConversationCubitChat = /* GraphQL */ `mutation UpdateConversationCubitChat(
  $condition: ModelConversationCubitChatConditionInput
  $input: UpdateConversationCubitChatInput!
) {
  updateConversationCubitChat(condition: $condition, input: $input) {
    createdAt
    id
    messages {
      nextToken
      __typename
    }
    metadata
    name
    owner
    updatedAt
    __typename
  }
}
` as GeneratedMutation<
  APITypes.UpdateConversationCubitChatMutationVariables,
  APITypes.UpdateConversationCubitChatMutation
>;
export const updateProject = /* GraphQL */ `mutation UpdateProject(
  $condition: ModelProjectConditionInput
  $input: UpdateProjectInput!
) {
  updateProject(condition: $condition, input: $input) {
    canvasData
    createdAt
    id
    name
    owner
    teamId
    updatedAt
    __typename
  }
}
` as GeneratedMutation<
  APITypes.UpdateProjectMutationVariables,
  APITypes.UpdateProjectMutation
>;
export const updateTeam = /* GraphQL */ `mutation UpdateTeam(
  $condition: ModelTeamConditionInput
  $input: UpdateTeamInput!
) {
  updateTeam(condition: $condition, input: $input) {
    createdAt
    id
    members
    name
    owner
    updatedAt
    __typename
  }
}
` as GeneratedMutation<
  APITypes.UpdateTeamMutationVariables,
  APITypes.UpdateTeamMutation
>;
