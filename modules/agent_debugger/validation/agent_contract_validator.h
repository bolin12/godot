/**************************************************************************/
/*  agent_contract_validator.h                                            */
/**************************************************************************/

#pragma once

#include "../artifact/agent_artifact_types.h"

#include "core/object/object.h"

class SceneState;

class AgentContractValidator : public Object {
	GDCLASS(AgentContractValidator, Object);

	static AgentContractValidator *singleton;

	Vector<AgentValidationReceipt> receipts;
	uint64_t next_receipt_id = 1;

	static String _artifact_id_for_path(const String &p_path);
	static StringName _assertion_name(const StringName &p_kind, const String &p_subject, const StringName &p_expected = StringName());
	static int _find_node_by_path(const Ref<SceneState> &p_state, const NodePath &p_path);
	void _record_assertion(AgentValidationReceipt &r_receipt, const StringName &p_assertion, bool p_passed, const Dictionary &p_failure_detail = Dictionary());
	AgentValidationReceipt _validate_scene(const Dictionary &p_scene_contract, const StringName &p_contract_id, const String &p_validator_hash);
	AgentValidationReceipt _validate_input_actions(const Array &p_actions, const StringName &p_contract_id, const String &p_validator_hash);

protected:
	static void _bind_methods();

public:
	static AgentContractValidator *get_singleton() { return singleton; }

	void clear();
	Dictionary validate_contract(const Dictionary &p_contract);
	Array get_validation_receipts() const;

	AgentContractValidator();
	~AgentContractValidator();
};
