# AI Server Directory Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Register an Unreal server with a small web API, let an assignment client log in and receive the address, then connect automatically.

**Architecture:** A dependency-free Python HTTP service owns the fixed demonstration login and latest-server registry. An Unreal game-instance subsystem owns HTTP calls and travel, while the existing widget only gathers inputs and displays status.

**Tech Stack:** Python 3 standard library, Unreal Engine 5 C++, HTTP/JSON/UMG modules, Python `unittest`, Unreal Automation Tests.

**Spec:** `docs/superpowers/specs/2026-09-21-ai-server-directory-design.md`

## Global Constraints

- No external Python packages.
- Default demo account is `student` / `1234`.
- Default registration token is `assignment-secret`.
- Default API URL is `http://127.0.0.1:8080`.
- Default game address is `127.0.0.1:7777` and can be overridden with `-PublicServerAddress=`.
- Preserve the user's existing `PractiveUnreal.uproject` MCP plugin change.

## Review Focus

- Malformed JSON returns 400 and does not mutate the registry.
- Registration without the correct bearer token returns 401.
- Login before registration returns 404 instead of an empty travel address.
- A malformed successful login response cannot trigger Unreal travel.
- A persisted registry is restored after restarting the Python process.

---

### Task 1: Web server directory API

**Files:**
- Create: `WebServer/server.py`
- Create: `WebServer/tests/test_server.py`
- Create: `WebServer/data/.gitkeep`

**Interfaces:**
- Produces: `ServerDirectory`, `create_server(host, port, directory)`, and the three HTTP endpoints from the spec.

- [ ] Write integration tests that make real HTTP requests to an ephemeral local server.
- [ ] Run `python -m unittest discover -s WebServer/tests -v` and verify failure because `WebServer.server` does not exist.
- [ ] Implement JSON validation, authentication, atomic persistence, and endpoints.
- [ ] Run the same test command and verify all tests pass.

### Task 2: Unreal response parser and HTTP subsystem

**Files:**
- Create: `Source/PractiveUnreal/Public/ServerDirectorySubsystem.h`
- Create: `Source/PractiveUnreal/Private/ServerDirectorySubsystem.cpp`
- Create: `Source/PractiveUnreal/Private/Tests/ServerDirectoryResponseParserTests.cpp`
- Modify: `Source/PractiveUnreal/PractiveUnreal.Build.cs`
- Modify: `Config/DefaultGame.ini`

**Interfaces:**
- Produces: `FServerDirectoryResponseParser::TryParseLoginResponse`, `UServerDirectorySubsystem::StartListenServer`, and `UServerDirectorySubsystem::LoginAndConnect`.
- Consumes: Web endpoints and response bodies defined in Task 1.

- [ ] Add Unreal automation tests for success, API error, and missing address parsing.
- [ ] Build and verify failure because the parser/subsystem does not exist.
- [ ] Implement the parser, HTTP requests, dedicated-server registration, delegates, and travel.
- [ ] Build and run available automation tests.

### Task 3: Title widget integration

**Files:**
- Modify: `Source/PractiveUnreal/Public/TitleWidgetBase.h`
- Modify: `Source/PractiveUnreal/Private/TitleWidgetBase.cpp`

**Interfaces:**
- Consumes: `UServerDirectorySubsystem` functions and status delegate from Task 2.

- [ ] Bind the existing buttons and fields to the subsystem.
- [ ] Add optional status text support without requiring a Blueprint migration.
- [ ] Build the Unreal editor target.

### Task 4: Assignment documentation and verification

**Files:**
- Create: `README.md`
- Create: `docs/ASSIGNMENT_REPORT.md`
- Create: `WebServer/README.md`

**Interfaces:**
- Documents the executable behavior produced by Tasks 1-3.

- [ ] Document setup, launch commands, configuration, API examples, test commands, and demo flow.
- [ ] Run the complete Python suite and Unreal build.
- [ ] Review the branch against the spec and push the feature branch to GitHub.
