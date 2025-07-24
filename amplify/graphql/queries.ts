/* tslint:disable */
/* eslint-disable */
// this is an auto generated file. This will be overwritten

import * as APITypes from "./API";
type GeneratedQuery<InputType, OutputType> = string & {
  __generatedQueryInput: InputType;
  __generatedQueryOutput: OutputType;
};

export const getConversationCubitChat = /* GraphQL */ `query GetConversationCubitChat($id: ID!) {
  getConversationCubitChat(id: $id) {
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
` as GeneratedQuery<
  APITypes.GetConversationCubitChatQueryVariables,
  APITypes.GetConversationCubitChatQuery
>;
export const getConversationMessageCubitChat = /* GraphQL */ `query GetConversationMessageCubitChat($id: ID!) {
  getConversationMessageCubitChat(id: $id) {
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
` as GeneratedQuery<
  APITypes.GetConversationMessageCubitChatQueryVariables,
  APITypes.GetConversationMessageCubitChatQuery
>;
export const getProject = /* GraphQL */ `query GetProject($id: ID!) {
  getProject(id: $id) {
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
` as GeneratedQuery<
  APITypes.GetProjectQueryVariables,
  APITypes.GetProjectQuery
>;
export const getTeam = /* GraphQL */ `query GetTeam($id: ID!) {
  getTeam(id: $id) {
    createdAt
    id
    members
    name
    owner
    updatedAt
    __typename
  }
}
` as GeneratedQuery<APITypes.GetTeamQueryVariables, APITypes.GetTeamQuery>;
export const listConversationCubitChats = /* GraphQL */ `query ListConversationCubitChats(
  $filter: ModelConversationCubitChatFilterInput
  $limit: Int
  $nextToken: String
) {
  listConversationCubitChats(
    filter: $filter
    limit: $limit
    nextToken: $nextToken
  ) {
    items {
      createdAt
      id
      metadata
      name
      owner
      updatedAt
      __typename
    }
    nextToken
    __typename
  }
}
` as GeneratedQuery<
  APITypes.ListConversationCubitChatsQueryVariables,
  APITypes.ListConversationCubitChatsQuery
>;
export const listConversationMessageCubitChats = /* GraphQL */ `query ListConversationMessageCubitChats(
  $filter: ModelConversationMessageCubitChatFilterInput
  $limit: Int
  $nextToken: String
) {
  listConversationMessageCubitChats(
    filter: $filter
    limit: $limit
    nextToken: $nextToken
  ) {
    items {
      aiContext
      associatedUserMessageId
      conversationId
      createdAt
      id
      owner
      role
      updatedAt
      __typename
    }
    nextToken
    __typename
  }
}
` as GeneratedQuery<
  APITypes.ListConversationMessageCubitChatsQueryVariables,
  APITypes.ListConversationMessageCubitChatsQuery
>;
export const listProjects = /* GraphQL */ `query ListProjects(
  $filter: ModelProjectFilterInput
  $limit: Int
  $nextToken: String
) {
  listProjects(filter: $filter, limit: $limit, nextToken: $nextToken) {
    items {
      canvasData
      createdAt
      id
      name
      owner
      teamId
      updatedAt
      __typename
    }
    nextToken
    __typename
  }
}
` as GeneratedQuery<
  APITypes.ListProjectsQueryVariables,
  APITypes.ListProjectsQuery
>;
export const listTeams = /* GraphQL */ `query ListTeams(
  $filter: ModelTeamFilterInput
  $limit: Int
  $nextToken: String
) {
  listTeams(filter: $filter, limit: $limit, nextToken: $nextToken) {
    items {
      createdAt
      id
      members
      name
      owner
      updatedAt
      __typename
    }
    nextToken
    __typename
  }
}
` as GeneratedQuery<APITypes.ListTeamsQueryVariables, APITypes.ListTeamsQuery>;
