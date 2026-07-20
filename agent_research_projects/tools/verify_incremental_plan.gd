extends SceneTree


func _reference_invalidated(report: Dictionary, changed_paths: PackedStringArray) -> PackedStringArray:
	var invalidated := {}
	for artifact: Dictionary in report.get("artifacts", []):
		if changed_paths.has(str(artifact.get("path", ""))) or changed_paths.has(str(artifact.get("artifact_id", ""))):
			invalidated[str(artifact["artifact_id"])] = true

	var changed := true
	while changed:
		changed = false
		for edge: Dictionary in report.get("dependencies", []):
			var dependency_id := str(edge.get("to_artifact_id", ""))
			var consumer_id := str(edge.get("from_artifact_id", ""))
			if invalidated.has(dependency_id) and not invalidated.has(consumer_id):
				invalidated[consumer_id] = true
				changed = true

	var result := PackedStringArray(invalidated.keys())
	result.sort()
	return result


func _initialize() -> void:
	var roots := PackedStringArray([str(ProjectSettings.get_setting("application/run/main_scene", ""))])
	var changed_paths := PackedStringArray(["res://player.gd"])
	for argument in OS.get_cmdline_user_args():
		if argument.begins_with("--changed="):
			changed_paths.append(argument.trim_prefix("--changed="))

	var first_report: Dictionary = AgentArtifactGraph.scan_resource_graph(roots, true, 64)
	var first_fingerprint := AgentArtifactGraph.get_graph_fingerprint()
	var incremental_plan := AgentArtifactGraph.plan_invalidated_artifacts(changed_paths, 64)
	var reference_plan := _reference_invalidated(first_report, changed_paths)
	var second_report: Dictionary = AgentArtifactGraph.scan_resource_graph(roots, true, 64)
	var second_fingerprint := AgentArtifactGraph.get_graph_fingerprint()

	var plan_matches := incremental_plan == reference_plan
	var full_scan_stable := first_fingerprint == second_fingerprint
	var validation_passed := int(first_report.get("failed_receipt_count", 0)) == 0 and int(second_report.get("failed_receipt_count", 0)) == 0
	var passed := plan_matches and full_scan_stable and validation_passed and not incremental_plan.is_empty()
	var report := {
		"schema_version": "0.2",
		"result": &"pass" if passed else &"fail",
		"changed_paths": changed_paths,
		"invalidated_artifact_count": incremental_plan.size(),
		"incremental_plan": incremental_plan,
		"reference_plan": reference_plan,
		"plan_matches_full_graph": plan_matches,
		"full_scan_fingerprint_stable": full_scan_stable,
		"first_fingerprint": first_fingerprint,
		"second_fingerprint": second_fingerprint,
		"validation_passed": validation_passed,
	}
	print(JSON.stringify(report))
	quit(0 if passed else 1)
