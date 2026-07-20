# Godot Agent Debug Causal Edge Rules v0.1

## Rule priority

An edge is created only when a named construction rule provides evidence. Priority is:

```text
direct runtime correlation
> engine structural relation
> deterministic inference
> temporal proximity
```

Temporal proximity alone is never presented as confirmed causality.

## Initial rules

| Rule | From | To | Edge | Evidence |
|---|---|---|---|---|
| Invocation | method enter | child invocation or method exit | `call` | direct |
| Signal delivery | signal emit | connected receiver callback | `emit_deliver` | direct |
| Deferred call | schedule record | executed callable | `schedule_execute` | direct |
| Property dependency | property write | observed read/derived state | `write_read` | direct/inferred |
| Lifecycle | object create | first use | `create_use` | direct |
| Resource dependency | load complete | consumer invocation/state | `load_use` | direct/inferred |
| Patch attribution | patch begin/end | changed script/resource event | `patch_affects` | direct |
| Assertion | state-changing event | violated assertion | `violates` | direct/inferred |
| Ordered fallback | earlier event | later related event | `happens_before` | temporal_only |

## Invalid edges

Do not create an edge solely because:

- two events occurred in adjacent frames;
- two objects share a Node parent;
- an event happened after the latest Patch;
- two values have the same text representation;
- a log line mentions the same script or Node name.

## Multiple causes

Queries return a ranked set and retain converging branches. The first version does not force a single root cause.

## Ranking v0.1

The deterministic ranker uses:

- backward graph depth;
- evidence level;
- edge type;
- explicit Patch association;
- repeated reachability through independent incoming paths.

The ranker must return score reasons. It does not call an LLM.
