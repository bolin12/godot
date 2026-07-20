extends SceneTree


static func make_contract() -> Dictionary:
	var contract := {
		"contract_id": &"dodge_the_creeps/0.1",
		"scenes": [
			{
				"path": "res://main.tscn",
				"required_nodes": [
					{ "path": ".", "type": &"Node" },
					{ "path": "Player" },
					{ "path": "MobTimer", "type": &"Timer" },
					{ "path": "ScoreTimer", "type": &"Timer" },
					{ "path": "StartTimer", "type": &"Timer" },
					{ "path": "HUD" },
				],
				"required_connections": [
					{ "source": "Player", "signal": &"hit", "target": ".", "method": &"game_over" },
					{ "source": "MobTimer", "signal": &"timeout", "target": ".", "method": &"_on_MobTimer_timeout" },
					{ "source": "ScoreTimer", "signal": &"timeout", "target": ".", "method": &"_on_ScoreTimer_timeout" },
					{ "source": "StartTimer", "signal": &"timeout", "target": ".", "method": &"_on_StartTimer_timeout" },
					{ "source": "HUD", "signal": &"start_game", "target": ".", "method": &"new_game" },
				],
			},
			{
				"path": "res://player.tscn",
				"required_nodes": [
					{ "path": ".", "type": &"Area2D" },
					{ "path": "AnimatedSprite2D", "type": &"AnimatedSprite2D" },
					{ "path": "CollisionShape2D", "type": &"CollisionShape2D" },
					{ "path": "Trail", "type": &"GPUParticles2D" },
				],
				"required_connections": [
					{ "source": ".", "signal": &"body_entered", "target": ".", "method": &"_on_body_entered" },
				],
			},
			{
				"path": "res://mob.tscn",
				"required_nodes": [
					{ "path": ".", "type": &"RigidBody2D", "groups": [&"mobs"] },
					{ "path": "AnimatedSprite2D", "type": &"AnimatedSprite2D" },
					{ "path": "CollisionShape2D", "type": &"CollisionShape2D" },
					{ "path": "VisibleOnScreenNotifier2D", "type": &"VisibleOnScreenNotifier2D" },
				],
				"required_connections": [
					{ "source": "VisibleOnScreenNotifier2D", "signal": &"screen_exited", "target": ".", "method": &"_on_VisibilityNotifier2D_screen_exited" },
				],
			},
			{
				"path": "res://hud.tscn",
				"required_nodes": [
					{ "path": ".", "type": &"CanvasLayer" },
					{ "path": "ScoreLabel", "type": &"Label" },
					{ "path": "MessageLabel", "type": &"Label" },
					{ "path": "StartButton", "type": &"Button" },
					{ "path": "MessageTimer", "type": &"Timer" },
				],
				"required_connections": [
					{ "source": "StartButton", "signal": &"pressed", "target": ".", "method": &"_on_StartButton_pressed" },
					{ "source": "MessageTimer", "signal": &"timeout", "target": ".", "method": &"_on_MessageTimer_timeout" },
				],
			},
		],
		"required_input_actions": [
			&"move_left",
			&"move_right",
			&"move_up",
			&"move_down",
			&"start_game",
		],
	}
	return contract


func _initialize() -> void:
	var contract := make_contract()

	var report: Dictionary = AgentContractValidator.validate_contract(contract)
	var full_report := OS.get_cmdline_user_args().has("--full")
	if full_report:
		print(JSON.stringify(report))
	else:
		print(JSON.stringify({
			"schema_version": report.get("schema_version"),
			"contract_id": report.get("contract_id"),
			"result": report.get("result"),
			"receipt_count": report.get("receipt_count"),
			"failed_receipt_count": report.get("failed_receipt_count"),
		}))
	quit(0 if report.get("result") == &"pass" else 1)
