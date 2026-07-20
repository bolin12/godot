# Agent Debugger module

This experimental module is disabled by default. Community source snapshots under `research_papers/godot_ai_plugins/sources` are not part of the module and are never compiled automatically.

Enable it for an Agent debugging build:

```text
py -m SCons platform=windows target=editor module_agent_debugger_enabled=yes
```

Build and run its tests:

```text
py -m SCons platform=windows target=editor tests=yes module_agent_debugger_enabled=yes
bin\godot.windows.editor.x86_64.console.exe --test --test-case="*[AgentDebugger]*" --no-colors
```

Disable it explicitly for a normal build:

```text
py -m SCons platform=windows target=editor tests=no module_agent_debugger_enabled=no
```

Future community implementations should be introduced as individually selectable adapters. An adapter must not create an alternative trace store; it should translate its transport or editor operations to the `AgentTrace` API.

## Artifact validation

`AgentArtifactGraph` builds a versioned resource dependency graph and records native Godot resource-loading receipts. It deliberately delegates syntax and resource-format parsing to Godot instead of implementing another parser.

Run the scanner against a project main scene:

```text
bin\godot.windows.editor.x86_64.console.exe --headless --path <project> --import
bin\godot.windows.editor.x86_64.console.exe --headless --path <project> --script <absolute-path-to-scan_artifact_graph.gd>
```

Append `-- --summary-only` to print only graph counts and the validation result.

The process prints a JSON report and exits with code `1` when any resource in the scanned dependency closure fails native validation.

## Semantic contracts

`AgentContractValidator` validates declarative scene and InputMap contracts without instantiating or running gameplay. The first contract format supports:

- required scene nodes and compatible node types;
- required node groups;
- persisted signal connections;
- required InputMap actions.

Each scene and the project InputMap produce a `semantic_contract` validation receipt with assertion-level failure details.

## Incremental invalidation

`AgentArtifactGraph.plan_invalidated_artifacts()` maps changed resource paths to the changed artifacts and their reverse dependency closure. `get_graph_fingerprint()` returns a deterministic SHA-256 over artifacts, dependency edges, and validation outcomes while excluding receipt IDs and timestamps.

These APIs currently prove deterministic invalidation planning. They do not yet persist or partially recompute cached graph records.

## Runtime contracts

`AgentTrace` stores bounded runtime events and causal edges. `get_causal_edges()` exposes the edge records used by backward failure queries.

The Dodge runtime harness instantiates the real main scene in a headless process, executes `new_game`, emits `Player.hit`, verifies resulting state, and returns a structured trace report. Observations created by the external harness remain explicitly distinguishable from future engine-level automatic hooks.
