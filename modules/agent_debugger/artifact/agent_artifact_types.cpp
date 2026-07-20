/**************************************************************************/
/*  agent_artifact_types.cpp                                              */
/**************************************************************************/

#include "agent_artifact_types.h"

#include "core/variant/array.h"
#include "core/variant/variant.h"

Dictionary AgentArtifactNode::to_dictionary() const {
	Dictionary result;
	result["schema_version"] = "0.2";
	result["artifact_id"] = artifact_id;
	result["path"] = path;
	result["uid"] = uid;
	result["resource_type"] = resource_type;
	result["content_hash"] = content_hash;
	result["exists"] = exists;
	return result;
}

Dictionary AgentArtifactEdge::to_dictionary() const {
	Dictionary result;
	result["schema_version"] = "0.2";
	result["from_artifact_id"] = from_artifact_id;
	result["to_artifact_id"] = to_artifact_id;
	result["edge_type"] = edge_type;
	result["evidence"] = evidence;
	result["construction_rule"] = construction_rule;
	return result;
}

Dictionary AgentValidationReceipt::to_dictionary() const {
	Dictionary data;
	data["schema_version"] = "0.2";
	data["receipt_id"] = receipt_id;
	data["gate"] = gate;
	data["subject_id"] = subject_id;
	data["validator_id"] = validator_id;
	data["validator_hash"] = validator_hash;
	data["input_hash"] = input_hash;
	data["result"] = result;
	data["proof_level"] = proof_level;
	data["error_code"] = error_code;
	data["timestamp_usec"] = timestamp_usec;
	Array assertion_array;
	for (const StringName &assertion : assertions) {
		assertion_array.push_back(assertion);
	}
	data["assertions"] = assertion_array;
	Array failed_assertion_array;
	for (const StringName &assertion : failed_assertions) {
		failed_assertion_array.push_back(assertion);
	}
	data["failed_assertions"] = failed_assertion_array;
	data["details"] = details;
	return data;
}
