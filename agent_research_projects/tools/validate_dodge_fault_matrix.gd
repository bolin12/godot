extends SceneTree


const DODGE_CONTRACT = preload("validate_dodge_contract.gd")


func _failed_assertions(report: Dictionary) -> Array[StringName]:
	var failures: Array[StringName] = []
	for receipt: Dictionary in report.get("validation_receipts", []):
		for assertion: StringName in receipt.get("failed_assertions", []):
			failures.append(assertion)
	return failures


func _run_fault(case_id: StringName, expected_assertion: StringName, mutate: Callable) -> Dictionary:
	var contract: Dictionary = DODGE_CONTRACT.make_contract().duplicate(true)
	contract["contract_id"] = StringName("dodge_the_creeps/fault/" + String(case_id))
	mutate.call(contract)
	var report: Dictionary = AgentContractValidator.validate_contract(contract)
	var failures := _failed_assertions(report)
	return {
		"case_id": case_id,
		"expected_assertion": expected_assertion,
		"failed_assertions": failures,
		"passed": report.get("result") == &"fail" and failures.has(expected_assertion),
	}


func _initialize() -> void:
	var results: Array[Dictionary] = []
	results.append(_run_fault(&"missing_node", &"node_exists:MissingContractNode", func(contract: Dictionary) -> void:
		contract["scenes"][0]["required_nodes"].append({ "path": "MissingContractNode", "type": &"Node" })
	))
	results.append(_run_fault(&"wrong_type", &"node_type:MobTimer:Button", func(contract: Dictionary) -> void:
		contract["scenes"][0]["required_nodes"][2]["type"] = &"Button"
	))
	results.append(_run_fault(&"missing_group", &"node_group:.:missing_contract_group", func(contract: Dictionary) -> void:
		contract["scenes"][2]["required_nodes"][0]["groups"].append(&"missing_contract_group")
	))
	results.append(_run_fault(&"missing_connection", &"connection:MobTimer:timeout->HUD:_missing_contract_method", func(contract: Dictionary) -> void:
		contract["scenes"][0]["required_connections"].append({
			"source": "MobTimer",
			"signal": &"timeout",
			"target": "HUD",
			"method": &"_missing_contract_method",
		})
	))
	results.append(_run_fault(&"missing_input", &"input_action:missing_contract_action", func(contract: Dictionary) -> void:
		contract["required_input_actions"].append(&"missing_contract_action")
	))

	var failed_cases: Array[StringName] = []
	for result: Dictionary in results:
		if not result["passed"]:
			failed_cases.append(result["case_id"])
	var report := {
		"schema_version": "0.2",
		"matrix_id": "dodge_the_creeps/semantic_faults/0.1",
		"result": &"pass" if failed_cases.is_empty() else &"fail",
		"case_count": results.size(),
		"failed_case_count": failed_cases.size(),
		"failed_cases": failed_cases,
		"cases": results,
	}
	print(JSON.stringify(report if OS.get_cmdline_user_args().has("--full") else {
		"schema_version": report["schema_version"],
		"matrix_id": report["matrix_id"],
		"result": report["result"],
		"case_count": report["case_count"],
		"failed_case_count": report["failed_case_count"],
	}))
	quit(0 if failed_cases.is_empty() else 1)
