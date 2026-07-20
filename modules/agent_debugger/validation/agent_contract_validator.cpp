/**************************************************************************/
/*  agent_contract_validator.cpp                                          */
/**************************************************************************/

#include "agent_contract_validator.h"

#include "core/input/input_map.h"
#include "core/io/resource_loader.h"
#include "core/io/resource_uid.h"
#include "core/object/class_db.h"
#include "core/os/os.h"
#include "core/variant/array.h"
#include "core/variant/variant.h"
#include "scene/resources/packed_scene.h"

AgentContractValidator *AgentContractValidator::singleton = nullptr;

void AgentContractValidator::_bind_methods() {
	ClassDB::bind_method(D_METHOD("clear"), &AgentContractValidator::clear);
	ClassDB::bind_method(D_METHOD("validate_contract", "contract"), &AgentContractValidator::validate_contract);
	ClassDB::bind_method(D_METHOD("get_validation_receipts"), &AgentContractValidator::get_validation_receipts);
}

String AgentContractValidator::_artifact_id_for_path(const String &p_path) {
	const ResourceUID::ID uid = ResourceLoader::get_resource_uid(p_path);
	if (uid != ResourceUID::INVALID_ID) {
		return ResourceUID::get_singleton()->id_to_text(uid);
	}
	return p_path;
}

StringName AgentContractValidator::_assertion_name(const StringName &p_kind, const String &p_subject, const StringName &p_expected) {
	String assertion = String(p_kind) + ":" + p_subject;
	if (p_expected != StringName()) {
		assertion += ":" + String(p_expected);
	}
	return assertion;
}

int AgentContractValidator::_find_node_by_path(const Ref<SceneState> &p_state, const NodePath &p_path) {
	const NodePath rooted_path(p_path == NodePath(".") ? String(".") : String("./") + String(p_path));
	for (int i = 0; i < p_state->get_node_count(); i++) {
		const NodePath stored_path = p_state->get_node_path(i);
		if (stored_path == p_path || stored_path == rooted_path) {
			return i;
		}
	}
	return -1;
}

void AgentContractValidator::_record_assertion(AgentValidationReceipt &r_receipt, const StringName &p_assertion, bool p_passed, const Dictionary &p_failure_detail) {
	r_receipt.assertions.push_back(p_assertion);
	if (p_passed) {
		return;
	}
	r_receipt.failed_assertions.push_back(p_assertion);
	if (!p_failure_detail.is_empty()) {
		r_receipt.details[p_assertion] = p_failure_detail;
	}
}

AgentValidationReceipt AgentContractValidator::_validate_scene(const Dictionary &p_scene_contract, const StringName &p_contract_id, const String &p_validator_hash) {
	AgentValidationReceipt receipt;
	receipt.receipt_id = next_receipt_id++;
	receipt.gate = "semantic_contract";
	receipt.validator_id = p_contract_id;
	receipt.validator_hash = p_validator_hash;
	receipt.proof_level = "tested";
	receipt.timestamp_usec = OS::get_singleton()->get_ticks_usec();

	const String scene_path = p_scene_contract.get("path", String());
	receipt.subject_id = _artifact_id_for_path(scene_path);
	const StringName scene_load_assertion = _assertion_name("scene_loads", scene_path);
	Error load_error = OK;
	Ref<PackedScene> scene = ResourceLoader::load(scene_path, "PackedScene", ResourceLoader::CACHE_MODE_IGNORE, &load_error);
	Dictionary load_detail;
	load_detail["path"] = scene_path;
	load_detail["error_code"] = load_error;
	_record_assertion(receipt, scene_load_assertion, load_error == OK && scene.is_valid(), load_detail);
	if (scene.is_null()) {
		receipt.result = "fail";
		receipt.error_code = load_error;
		return receipt;
	}

	const Ref<SceneState> state = scene->get_state();
	const Array required_nodes = p_scene_contract.get("required_nodes", Array());
	for (int i = 0; i < required_nodes.size(); i++) {
		const Dictionary rule = required_nodes[i];
		const String node_path_string = rule.get("path", String("."));
		const NodePath node_path(node_path_string);
		const int node_index = _find_node_by_path(state, node_path);
		const StringName exists_assertion = _assertion_name("node_exists", node_path_string);
		Dictionary exists_detail;
		exists_detail["scene"] = scene_path;
		exists_detail["node_path"] = node_path_string;
		_record_assertion(receipt, exists_assertion, node_index >= 0, exists_detail);
		if (node_index < 0) {
			continue;
		}

		const StringName expected_type = rule.get("type", StringName());
		if (expected_type != StringName()) {
			const StringName actual_type = state->get_node_type(node_index);
			const bool type_matches = actual_type == expected_type || (!actual_type.is_empty() && ClassDB::is_parent_class(actual_type, expected_type));
			const StringName type_assertion = _assertion_name("node_type", node_path_string, expected_type);
			Dictionary type_detail;
			type_detail["scene"] = scene_path;
			type_detail["node_path"] = node_path_string;
			type_detail["expected"] = expected_type;
			type_detail["actual"] = actual_type;
			_record_assertion(receipt, type_assertion, type_matches, type_detail);
		}

		const Array required_groups = rule.get("groups", Array());
		for (int group_index = 0; group_index < required_groups.size(); group_index++) {
			const StringName group = required_groups[group_index];
			const StringName group_assertion = _assertion_name("node_group", node_path_string, group);
			Dictionary group_detail;
			group_detail["scene"] = scene_path;
			group_detail["node_path"] = node_path_string;
			group_detail["group"] = group;
			_record_assertion(receipt, group_assertion, state->is_node_in_group(node_index, group), group_detail);
		}
	}

	const Array required_connections = p_scene_contract.get("required_connections", Array());
	for (int i = 0; i < required_connections.size(); i++) {
		const Dictionary rule = required_connections[i];
		const String source = rule.get("source", String("."));
		const StringName signal = rule.get("signal", StringName());
		const String target = rule.get("target", String("."));
		const StringName method = rule.get("method", StringName());
		const String connection_subject = source + ":" + String(signal) + "->" + target + ":" + String(method);
		const StringName connection_assertion = _assertion_name("connection", connection_subject);
		Dictionary connection_detail;
		connection_detail["scene"] = scene_path;
		connection_detail["source"] = source;
		connection_detail["signal"] = signal;
		connection_detail["target"] = target;
		connection_detail["method"] = method;
		_record_assertion(receipt, connection_assertion, state->has_connection(NodePath(source), signal, NodePath(target), method, true), connection_detail);
	}

	receipt.result = receipt.failed_assertions.is_empty() ? StringName("pass") : StringName("fail");
	return receipt;
}

AgentValidationReceipt AgentContractValidator::_validate_input_actions(const Array &p_actions, const StringName &p_contract_id, const String &p_validator_hash) {
	AgentValidationReceipt receipt;
	receipt.receipt_id = next_receipt_id++;
	receipt.gate = "semantic_contract";
	receipt.subject_id = "res://project.godot";
	receipt.validator_id = p_contract_id;
	receipt.validator_hash = p_validator_hash;
	receipt.proof_level = "tested";
	receipt.timestamp_usec = OS::get_singleton()->get_ticks_usec();

	for (int i = 0; i < p_actions.size(); i++) {
		const StringName action = p_actions[i];
		const StringName assertion = _assertion_name("input_action", String(action));
		Dictionary detail;
		detail["action"] = action;
		_record_assertion(receipt, assertion, InputMap::get_singleton()->has_action(action), detail);
	}
	receipt.result = receipt.failed_assertions.is_empty() ? StringName("pass") : StringName("fail");
	return receipt;
}

void AgentContractValidator::clear() {
	receipts.clear();
	next_receipt_id = 1;
}

Dictionary AgentContractValidator::validate_contract(const Dictionary &p_contract) {
	clear();
	const StringName contract_id = p_contract.get("contract_id", StringName("unnamed_contract"));
	const String validator_hash = String::num_uint64(p_contract.hash());

	const Array scenes = p_contract.get("scenes", Array());
	for (int i = 0; i < scenes.size(); i++) {
		receipts.push_back(_validate_scene(scenes[i], contract_id, validator_hash));
	}
	const Array input_actions = p_contract.get("required_input_actions", Array());
	if (!input_actions.is_empty()) {
		receipts.push_back(_validate_input_actions(input_actions, contract_id, validator_hash));
	}

	int failed_receipt_count = 0;
	for (const AgentValidationReceipt &receipt : receipts) {
		if (receipt.result == StringName("fail")) {
			failed_receipt_count++;
		}
	}
	Dictionary report;
	report["schema_version"] = "0.2";
	report["contract_id"] = contract_id;
	report["result"] = failed_receipt_count == 0 ? StringName("pass") : StringName("fail");
	report["receipt_count"] = receipts.size();
	report["failed_receipt_count"] = failed_receipt_count;
	report["validation_receipts"] = get_validation_receipts();
	return report;
}

Array AgentContractValidator::get_validation_receipts() const {
	Array result;
	for (const AgentValidationReceipt &receipt : receipts) {
		result.push_back(receipt.to_dictionary());
	}
	return result;
}

AgentContractValidator::AgentContractValidator() {
	ERR_FAIL_COND(singleton != nullptr);
	singleton = this;
}

AgentContractValidator::~AgentContractValidator() {
	if (singleton == this) {
		singleton = nullptr;
	}
}
