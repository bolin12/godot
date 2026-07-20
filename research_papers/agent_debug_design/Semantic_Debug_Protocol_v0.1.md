# Godot Agent Semantic Debug Protocol v0.1

## Session commands

```text
start_session(capacity)
stop_session()
clear()
get_session_info()
```

## Trace commands

```text
record_event(event)
add_causal_edge(edge)
get_events()
query_failure_window(event_id, before_count, after_count)
query_causes(failure_event_id, max_depth, limit)
```

## Planned high-level commands

```text
query_object_history(object_id)
query_invocation(invocation_id)
query_signal_chain(event_id)
query_resource_history(resource_uid)
query_state_diff(snapshot_a, snapshot_b)
request_instrumentation(hypothesis, targets, budget)
capture_snapshot(scope)
replay_from_checkpoint(checkpoint_id, overrides)
```

The first implementation exposes the session and trace commands through the `AgentTrace` engine singleton. EngineDebugger and community MCP integrations should be adapters over this API rather than alternative trace stores.

## Minimal GDScript example

```gdscript
var session_id := AgentTrace.start_session(4096)
var write_id := AgentTrace.record_event({
    "event_type": "property_write",
    "patch_id": "patch-001",
    "node_path": "/root/Main/Player",
    "method": "apply_damage",
    "payload": { "property": "health", "value": -1 }
})
var failure_id := AgentTrace.record_event({
    "event_type": "assertion_fail",
    "parent_event_ids": [write_id],
    "payload": { "assertion": "health >= 0" }
})
AgentTrace.add_causal_edge({
    "from_event_id": write_id,
    "to_event_id": failure_id,
    "edge_type": "violates",
    "evidence": "direct",
    "confidence": 1.0,
    "construction_rule": "health_invariant"
})

var failure_window := AgentTrace.query_failure_window(failure_id, 120, 30)
var top_causes := AgentTrace.query_causes(failure_id, 8, 10)
```

## Error behavior

- Invalid or missing required fields reject the operation.
- Recording while no session is active returns event ID `0`.
- `query_failure_window` reports `unknown_or_evicted_event`; `query_causes` returns an empty array when its target is unavailable.
- Ring-buffer eviction is reported using `dropped_event_count` and `trace_truncated`.
