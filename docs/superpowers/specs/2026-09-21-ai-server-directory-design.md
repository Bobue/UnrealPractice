# AI Server Directory Design

## Purpose

This project needs a small assignment-ready matchmaking flow. An Unreal server registers its reachable address with a web service. A client signs in with a fixed demonstration account, receives the registered address, and connects to that Unreal server.

## Scope

- Add a dependency-free Python web service with health, server registration, and login endpoints.
- Add an Unreal Engine game-instance subsystem that registers listen or dedicated servers and logs clients in.
- Connect the existing title widget buttons to the subsystem.
- Persist the latest registered server address in a local JSON file.
- Provide automated web API tests and Unreal automation tests for response parsing.
- Document setup, execution, API payloads, demonstration steps, and AI-assisted work.

Actual account management, TLS termination, database-backed sessions, multiple server browsing, NAT traversal, and production secret management are outside this assignment.

## Architecture

`WebServer/server.py` runs a `ThreadingHTTPServer`. `ServerDirectory` owns credentials, the registration token, and the latest registered server. HTTP handlers validate JSON input and return JSON responses. The latest server is written atomically to `WebServer/data/server_registry.json` and restored on restart.

`UServerDirectorySubsystem` is owned by the Unreal game instance. It reads settings from `DefaultGame.ini`, sends HTTP requests using Unreal's `Http` and `Json` modules, parses responses through a small parser, broadcasts user-facing status, and performs map travel. The existing title widget forwards Start Server and Login button clicks to this subsystem.

## API Contract

### `GET /health`

Returns HTTP 200 with `{ "status": "ok" }`.

### `POST /api/server/register`

Requires `Authorization: Bearer <registration token>` and JSON `{ "serverAddress": "host:port" }`. A valid request replaces the latest server record and returns HTTP 200 with `{ "success": true, "serverAddress": "host:port" }`. Missing authorization returns 401. Invalid JSON or an invalid address returns 400.

### `POST /api/login`

Accepts `{ "userId": "student", "password": "1234" }` by default. Invalid credentials return 401. If no server has registered, it returns 404. Success returns HTTP 200 with `{ "success": true, "serverAddress": "host:port" }`.

Credentials and token can be overridden by `MATCHMAKING_USER`, `MATCHMAKING_PASSWORD`, and `UNREAL_SERVER_TOKEN` environment variables.

## Unreal Flow

1. The host presses Start Server.
2. Unreal resolves `-PublicServerAddress=host:port`, falling back to `127.0.0.1:7777` for a same-machine demonstration.
3. Unreal registers that address with the web API.
4. On success, Unreal opens `Lobby` with the `listen` option. A dedicated server build registers automatically after its game instance initializes.
5. A client enters the demonstration ID and password and presses Connect.
6. Unreal calls the login endpoint and, on success, calls `ClientTravel` with the returned address.

## Error Handling

Every response is checked for transport failure, HTTP status, and required JSON fields. Failures are logged and broadcast to the title widget. The Connect button cannot travel on malformed or unsuccessful responses. The web service rejects malformed bodies and never returns the registration token or password.

## Testing

Python integration tests run the real HTTP server on an ephemeral port and cover health, authorization, registration/login, invalid credentials, missing server state, and persistence. Unreal automation tests cover valid login response parsing, API error parsing, and missing server addresses. A full Unreal editor build verifies module dependencies and reflection code.

## Demonstration Constraints

The included defaults are intentionally suitable only for an assignment demonstration over localhost or a trusted LAN. For two machines, the host supplies a reachable address with `-PublicServerAddress=<LAN-IP>:7777`, and both games point `WebServerBaseUrl` at the machine running the Python service.
