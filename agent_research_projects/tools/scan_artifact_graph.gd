extends SceneTree


func _initialize() -> void:
	var roots := PackedStringArray()
	var summary_only := false
	for argument in OS.get_cmdline_user_args():
		if argument == "--summary-only":
			summary_only = true
		elif argument.begins_with("res://") or argument.begins_with("uid://"):
			roots.append(argument)

	if roots.is_empty():
		var main_scene := str(ProjectSettings.get_setting("application/run/main_scene", ""))
		if not main_scene.is_empty():
			roots.append(main_scene)

	if roots.is_empty():
		push_error("No resource roots were provided and the project has no main scene.")
		quit(2)
		return

	var report: Dictionary = AgentArtifactGraph.scan_resource_graph(roots, true, 64)
	print(JSON.stringify(AgentArtifactGraph.get_summary() if summary_only else report))
	quit(1 if int(report.get("failed_receipt_count", 0)) > 0 else 0)
