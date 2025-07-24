/* tslint:disable */
/* eslint-disable */
// this is an auto generated file. This will be overwritten

import * as APITypes from "./API";
type GeneratedSubscription<InputType, OutputType> = string & {
  __generatedSubscriptionInput: InputType;
  __generatedSubscriptionOutput: OutputType;
};

export const onCreateAssistantResponseCubitChat = /* GraphQL */ `subscription OnCreateAssistantResponseCubitChat($conversationId: ID) {
  onCreateAssistantResponseCubitChat(conversationId: $conversationId) {
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
` as GeneratedSubscription<
  APITypes.OnCreateAssistantResponseCubitChatSubscriptionVariables,
  APITypes.OnCreateAssistantResponseCubitChatSubscription
>;
export const onCreateConversationMessageCubitChat = /* GraphQL */ `subscription OnCreateConversationMessageCubitChat(
  $filter: ModelSubscriptionConversationMessageCubitChatFilterInput
  $owner: String
) {
  onCreateConversationMessageCubitChat(filter: $filter, owner: $owner) {
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
` as GeneratedSubscription<
  APITypes.OnCreateConversationMessageCubitChatSubscriptionVariables,
  APITypes.OnCreateConversationMessageCubitChatSubscription
>;
export const onCreateProject = /* GraphQL */ `subscription OnCreateProject(
  $filter: ModelSubscriptionProjectFilterInput
  $owner: String
) {
  onCreateProject(filter: $filter, owner: $owner) {
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
` as GeneratedSubscription<
  APITypes.OnCreateProjectSubscriptionVariables,
  APITypes.OnCreateProjectSubscription
>;
export const onCreateTeam = /* GraphQL */ `subscription OnCreateTeam(
  $filter: ModelSubscriptionTeamFilterInput
  $owner: String
) {
  onCreateTeam(filter: $filter, owner: $owner) {
    createdAt
    id
    members
    name
    owner
    updatedAt
    __typename
  }
}
` as GeneratedSubscription<
  APITypes.OnCreateTeamSubscriptionVariables,
  APITypes.OnCreateTeamSubscription
>;
export const onDeleteProject = /* GraphQL */ `subscription OnDeleteProject(
  $filter: ModelSubscriptionProjectFilterInput
  $owner: String
) {
  onDeleteProject(filter: $filter, owner: $owner) {
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
` as GeneratedSubscription<
  APITypes.OnDeleteProjectSubscriptionVariables,
  APITypes.OnDeleteProjectSubscription
>;
export const onDeleteTeam = /* GraphQL */ `subscription OnDeleteTeam(
  $filter: ModelSubscriptionTeamFilterInput
  $owner: String
) {
  onDeleteTeam(filter: $filter, owner: $owner) {
    createdAt
    id
    members
    name
    owner
    updatedAt
    __typename
  }
}
` as GeneratedSubscription<
  APITypes.OnDeleteTeamSubscriptionVariables,
  APITypes.OnDeleteTeamSubscription
>;
export const onUpdateProject = /* GraphQL */ `subscription OnUpdateProject(
  $filter: ModelSubscriptionProjectFilterInput
  $owner: String
) {
  onUpdateProject(filter: $filter, owner: $owner) {
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
` as GeneratedSubscription<
  APITypes.OnUpdateProjectSubscriptionVariables,
  APITypes.OnUpdateProjectSubscription
>;
export const onUpdateTeam = /* GraphQL */ `subscription OnUpdateTeam(
  $filter: ModelSubscriptionTeamFilterInput
  $owner: String
) {
  onUpdateTeam(filter: $filter, owner: $owner) {
    createdAt
    id
    members
    name
    owner
    updatedAt
    __typename
  }
}
` as GeneratedSubscription<
  APITypes.OnUpdateTeamSubscriptionVariables,
  APITypes.OnUpdateTeamSubscription
>;
