/* tslint:disable */
/* eslint-disable */
// this is an auto generated file. This will be overwritten

import * as APITypes from "./API";
type GeneratedQuery<InputType, OutputType> = string & {
  __generatedQueryInput: InputType;
  __generatedQueryOutput: OutputType;
};

export const cubitAi = /* GraphQL */ `query CubitAi($description: String) {
  cubitAi(description: $description)
}
` as GeneratedQuery<APITypes.CubitAiQueryVariables, APITypes.CubitAiQuery>;
