# Godot Agent Debug Trace Schema v0.1

## Status

- Version: `0.1`
- Stability: experimental
- Scope: GDScript, single-player, single-process prototype
- Transport: in-process ring buffer first; EngineDebugger/MCP adapters later

## Event envelope

Every event uses the following envelope:

```json
{
  "schema_version": "0.1",
  "session_id": 1,
  "event_id": 42,
  "parent_event_ids": [40],
  "event_type": "signal_emit",
  "patch_id": "patch-20260720-001",
  "frame": 120,
  "physics_frame": 60,
  "timestamp_usec": 123456789,
  "thread_id": 1,
  "object_id": 1042,
  "node_path": "/root/Main/Player",
  "resource_uid": "uid://...",
  "script_path": "res://player.gd",
  "method": "take_damage",
  "before_hash": 0,
  "after_hash": 0,
  "payload": {}
}
```

`event_id` is monotonically increasing inside a session. `timestamp_usec` is observational metadata and must not be treated as proof of causality.

## Core event types

- `patch_begin`, `patch_end`
- `object_create`, `object_destroy`
- `node_enter_tree`, `node_exit_tree`
- `method_enter`, `method_exit`
- `property_write`
- `signal_emit`, `signal_receive`
- `deferred_schedule`, `deferred_execute`
- `resource_load_begin`, `resource_load_complete`, `resource_load_fail`
- `input_event`, `random_seed`
- `assertion_fail`, `engine_error`, `test_fail`, `performance_violation`
- `performance_sample`, `test_begin`, `test_result`

Custom event types are allowed. Producers must use stable snake_case names.

## Edge envelope

```json
{
  "from_event_id": 40,
  "to_event_id": 42,
  "edge_type": "emit_deliver",
  "evidence": "direct",
  "confidence": 1.0,
  "construction_rule": "signal_connection_and_emission_id"
}
```

Core edge types:

- `call`
- `emit_deliver`
- `schedule_execute`
- `write_read`
- `create_use`
- `load_use`
- `patch_affects`
- `violates`
- `happens_before`

Evidence levels:

- `direct`: explicit runtime correlation ID or engine relation;
- `inferred`: deterministic inference from object/resource/call structure;
- `temporal_only`: time ordering with no stronger evidence.

## Bounded values

Payload producers must bound collection depth, string size and collection length. Object values should be represented by stable IDs and compact summaries, not recursively serialized arbitrary Variant graphs.

## Compatibility

- Readers must ignore unknown fields.
- Readers must preserve unknown event types.
- A breaking field semantic requires a schema version change.
- New optional fields and event types may be added without changing the major version.
