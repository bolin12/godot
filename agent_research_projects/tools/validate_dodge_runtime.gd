extends SceneTree


var assertion_results: Array[Dictionary] = []
var failed_event_ids: Array[int] = []


func _trace_event(event_type: StringName, payload: Dictionary, parent_event_ids: Array = []) -> int:
	return AgentTrace.record_event({
		"event_type": event_type,
		"frame": Engine.get_process_frames(),
		"physics_frame": Engine.get_physics_frames(),
		"parent_event_ids": parent_event_ids,
		"payload": payload,
	})


func _assert_runtime(assertion: StringName, passed: bool, parent_event_id: int, details: Dictionary = {}) -> int:
	var payload := details.duplicate(true)
	payload["assertion"] = assertion
	payload["passed"] = passed
	var event_id := _trace_event(&"assertion_pass" if passed else &"assertion_fail", payload, [parent_event_id] if parent_event_id > 0 else [])
	assertion_results.append({
		"assertion": assertion,
		"passed": passed,
		"event_id": event_id,
		"details": details,
	})
	if not passed:
		failed_event_ids.append(event_id)
	return event_id


func _initialize() -> void:
	call_deferred(&"_run")


func _run() -> void:
	AgentTrace.start_session(256)
	var main_path := str(ProjectSettings.get_setting("application/run/main_scene", ""))
	var packed_scene := ResourceLoader.load(main_path) as PackedScene
	var load_event := _trace_event(&"resource_load", {
		"path": main_path,
		"loaded": packed_scene != null,
	})
	if packed_scene == null:
		_assert_runtime(&"main_scene_loads", false, load_event, { "path": main_path })
		_finish()
		return

	var main := packed_scene.instantiate()
	var instantiate_event := _trace_event(&"node_instantiate", {
		"scene": main_path,
		"node_name": main.name,
	}, [load_event])
	root.add_child(main)
	await process_frame

	_assert_runtime(&"main_ready", main.is_node_ready(), instantiate_event)
	var player := main.get_node("Player") as Area2D
	var mob_timer := main.get_node("MobTimer") as Timer
	var score_timer := main.get_node("ScoreTimer") as Timer
	var start_timer := main.get_node("StartTimer") as Timer
	var hud := main.get_node("HUD") as CanvasLayer
	_assert_runtime(&"player_initially_hidden", not player.visible, instantiate_event)
	_assert_runtime(&"gameplay_timers_initially_stopped", mob_timer.is_stopped() and score_timer.is_stopped(), instantiate_event)

	var new_game_event := _trace_event(&"method_call", {
		"node_path": str(main.get_path()),
		"method": &"new_game",
	})
	main.call(&"new_game")
	await process_frame
	var new_game_snapshot := _trace_event(&"state_snapshot", {
		"score": main.get("score"),
		"player_visible": player.visible,
		"start_timer_stopped": start_timer.is_stopped(),
	}, [new_game_event])
	_assert_runtime(&"new_game_resets_score", main.get("score") == 0, new_game_snapshot)
	_assert_runtime(&"new_game_shows_player", player.visible, new_game_snapshot)
	_assert_runtime(&"new_game_starts_delay_timer", not start_timer.is_stopped(), new_game_snapshot)
	_assert_runtime(&"new_game_enables_player_collision", not player.get_node("CollisionShape2D").disabled, new_game_snapshot)

	var emit_event := _trace_event(&"signal_emit", {
		"node_path": str(player.get_path()),
		"signal": &"hit",
	})
	player.emit_signal(&"hit")
	await process_frame
	var deliver_event := _trace_event(&"signal_delivery_observed", {
		"source": str(player.get_path()),
		"signal": &"hit",
		"target": str(main.get_path()),
		"method": &"game_over",
	}, [emit_event])
	AgentTrace.add_causal_edge({
		"from_event_id": emit_event,
		"to_event_id": deliver_event,
		"edge_type": &"emit_deliver",
		"evidence": &"inferred",
		"confidence": 0.9,
		"construction_rule": &"persisted_connection_and_observed_state",
	})
	_assert_runtime(&"hit_stops_gameplay_timers", mob_timer.is_stopped() and score_timer.is_stopped(), deliver_event)
	_assert_runtime(&"hit_enters_game_over_ui", hud.get_node("MessageLabel").text == "Game Over", deliver_event)

	main.queue_free()
	await process_frame
	_finish()


func _finish() -> void:
	var failure_windows: Array[Dictionary] = []
	var cause_candidates := {}
	for event_id in failed_event_ids:
		failure_windows.append(AgentTrace.query_failure_window(event_id, 8, 2))
		cause_candidates[event_id] = AgentTrace.query_causes(event_id, 8, 10)
	var session_info: Dictionary = AgentTrace.get_session_info()
	var complete := assertion_results.size() == 9
	var passed := failed_event_ids.is_empty() and complete
	var report := {
		"schema_version": "0.2",
		"contract_id": "dodge_the_creeps/runtime/0.1",
		"result": &"pass" if passed else &"fail",
		"assertion_count": assertion_results.size(),
		"expected_assertion_count": 9,
		"complete": complete,
		"failed_assertion_count": failed_event_ids.size(),
		"assertions": assertion_results,
		"session": session_info,
		"events": AgentTrace.get_events(),
		"causal_edges": AgentTrace.get_causal_edges(),
		"failure_windows": failure_windows,
		"cause_candidates": cause_candidates,
	}
	AgentTrace.stop_session()
	print(JSON.stringify(report if OS.get_cmdline_user_args().has("--full") else {
		"schema_version": report["schema_version"],
		"contract_id": report["contract_id"],
		"result": report["result"],
		"assertion_count": report["assertion_count"],
		"failed_assertion_count": report["failed_assertion_count"],
		"event_count": session_info["event_count"],
		"edge_count": session_info["edge_count"],
	}))
	call_deferred(&"_quit_after_cleanup", 0 if passed else 1)


func _quit_after_cleanup(exit_code: int) -> void:
	await process_frame
	quit(exit_code)
