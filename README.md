# Stack Server

This repository contains multiple implementations of a service for handling stacks of strings. It is designed primarily for practice and learning purposes.

## Implementations

- [cpp-oatpp](./cpp-oatpp): Implementation using C++ and Oat++, with manual implementation of reference counting for stack nodes.

## API Endpoints

Elements are represented as plain text in the request or response body.

- `GET /{name}/top`: Retrieve the top element of the stack.
- `POST /{name}/push`: Push an element onto the stack.
    - Responds with status code 204 if successful.
- `POST /{name}/pop`: Pop the top element from the stack and retrieve it.
    - Response contains the popped element.
- `POST /{name}`: Create a new stack.
    - Responds with status code 201 if successful.
- `DELETE /{name}`: Delete a stack.
- `POST /{from}/copy`: Copy a stack. Use the query parameter `to` to specify the name of the new stack.
    - Responds with status code 204 if successful.

Errors are represented as plain text in the response body. Possible errors include:
- Status code `409`, body: `STACK_NAME_ALREADY_EXISTS`
- Status code `404`, body: `STACK_NAME_NOT_FOUND`
- Status code `405`, body: `STACK_EMPTY`
