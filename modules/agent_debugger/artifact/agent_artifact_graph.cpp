/**************************************************************************/
/*  agent_artifact_graph.cpp                                              */
/**************************************************************************/

#include "agent_artifact_graph.h"

#include "core/io/file_access.h"
#include "core/io/resource_loader.h"
#include "core/io/resource_uid.h"
#include "core/object/class_db.h"
#include "core/os/os.h"
#include "core/version.h"

AgentArtifactGraph *AgentArtifactGraph::singleton = nullptr;

void AgentArtifactGraph::_bind_methods() {
	ClassDB::bind_method(D_METHOD("clear"), &AgentArtifactGraph::clear);
	ClassDB::bind_method(D_METHOD("scan_resource_graph", "roots", "validate", "max_depth"), &AgentArtifactGraph::scan_resource_graph, DEFVAL(true), DEFVAL(64));
	ClassDB::bind_method(D_METHOD("get_artifacts"), &AgentArtifactGraph::get_artifacts);
	ClassDB::bind_method(D_METHOD("get_dependencies"), &AgentArtifactGraph::get_dependencies);
	ClassDB::bind_method(D_METHOD("get_validation_receipts", "subject_id"), &AgentArtifactGraph::get_validation_receipts, DEFVAL(String()));
	ClassDB::bind_method(D_METHOD("query_affected_artifacts", "artifact_id", "max_depth"), &AgentArtifactGraph::query_affected_artifacts, DEFVAL(64));
	ClassDB::bind_method(D_METHOD("plan_invalidated_artifacts", "changed_paths", "max_depth"), &AgentArtifactGraph::plan_invalidated_artifacts, DEFVAL(64));
	ClassDB::bind_method(D_METHOD("get_graph_fingerprint"), &AgentArtifactGraph::get_graph_fingerprint);
	ClassDB::bind_method(D_METHOD("get_summary"), &AgentArtifactGraph::get_summary);
}

String AgentArtifactGraph::_resolve_dependency_path(const String &p_dependency) {
	const int separator_count = p_dependency.get_slice_count("::");
	const String primary = p_dependency.get_slice("::", 0);
	if (!primary.begins_with("uid://")) {
		return primary;
	}
	if (separator_count >= 3) {
		const String fallback = p_dependency.get_slice("::", 2);
		if (!fallback.is_empty()) {
			return fallback;
		}
	}
	return ResourceUID::uid_to_path(primary);
}

StringName AgentArtifactGraph::_resolve_dependency_type(const String &p_dependency) {
	if (p_dependency.get_slice_count("::") < 2) {
		return StringName();
	}
	return p_dependency.get_slice("::", 1);
}

String AgentArtifactGraph::_normalize_path(const String &p_path) {
	if (p_path.begins_with("uid://")) {
		return ResourceUID::uid_to_path(p_path);
	}
	return p_path.simplify_path();
}

String AgentArtifactGraph::_make_artifact_id(const String &p_path, String *r_uid) {
	const ResourceUID::ID uid = ResourceLoader::get_resource_uid(p_path);
	if (uid != ResourceUID::INVALID_ID) {
		const String uid_text = ResourceUID::get_singleton()->id_to_text(uid);
		if (r_uid) {
			*r_uid = uid_text;
		}
		return uid_text;
	}
	if (r_uid) {
		r_uid->clear();
	}
	return p_path;
}

int AgentArtifactGraph::_find_artifact(const String &p_artifact_id) const {
	for (int i = 0; i < artifacts.size(); i++) {
		if (artifacts[i].artifact_id == p_artifact_id) {
			return i;
		}
	}
	return -1;
}

bool AgentArtifactGraph::_has_edge(const String &p_from, const String &p_to, const StringName &p_type) const {
	for (const AgentArtifactEdge &edge : edges) {
		if (edge.from_artifact_id == p_from && edge.to_artifact_id == p_to && edge.edge_type == p_type) {
			return true;
		}
	}
	return false;
}

AgentValidationReceipt AgentArtifactGraph::_validate_resource(const AgentArtifactNode &p_artifact) {
	AgentValidationReceipt receipt;
	receipt.receipt_id = next_receipt_id++;
	receipt.gate = "native_parse";
	receipt.subject_id = p_artifact.artifact_id;
	receipt.validator_id = "godot_resource_loader";
	receipt.validator_hash = GODOT_VERSION_FULL_NAME;
	receipt.input_hash = p_artifact.content_hash;
	receipt.timestamp_usec = OS::get_singleton()->get_ticks_usec();
	receipt.assertions.push_back("resource_exists");
	receipt.assertions.push_back("resource_loads");

	Error load_error = OK;
	Ref<Resource> resource;
	if (p_artifact.exists) {
		resource = ResourceLoader::load(p_artifact.path, String(), ResourceLoader::CACHE_MODE_IGNORE, &load_error);
	}
	if (p_artifact.exists && load_error == OK && resource.is_valid()) {
		receipt.result = "pass";
		receipt.proof_level = "tested";
	} else {
		receipt.result = "fail";
		receipt.proof_level = "tested";
		if (!p_artifact.exists) {
			receipt.failed_assertions.push_back("resource_exists");
			receipt.error_code = ERR_FILE_NOT_FOUND;
		} else {
			receipt.error_code = load_error;
		}
		receipt.failed_assertions.push_back("resource_loads");
	}
	return receipt;
}

void AgentArtifactGraph::clear() {
	artifacts.clear();
	edges.clear();
	receipts.clear();
	next_receipt_id = 1;
}

Dictionary AgentArtifactGraph::scan_resource_graph(const PackedStringArray &p_roots, bool p_validate, int p_max_depth) {
	clear();
	p_max_depth = MAX(0, p_max_depth);

	Vector<Pair<String, int>> pending;
	for (const String &root : p_roots) {
		pending.push_back(Pair<String, int>(_normalize_path(root), 0));
	}
	HashSet<String> visited_paths;

	while (!pending.is_empty()) {
		const Pair<String, int> current = pending[0];
		pending.remove_at(0);
		const String path = current.first;
		if (path.is_empty() || visited_paths.has(path)) {
			continue;
		}
		visited_paths.insert(path);

		AgentArtifactNode artifact;
		artifact.path = path;
		artifact.exists = ResourceLoader::exists(path);
		artifact.artifact_id = _make_artifact_id(path, &artifact.uid);
		artifact.resource_type = ResourceLoader::get_resource_type(path);
		if (artifact.exists) {
			artifact.content_hash = FileAccess::get_sha256(path);
		}
		artifacts.push_back(artifact);

		if (p_validate) {
			receipts.push_back(_validate_resource(artifact));
		}
		if (!artifact.exists || current.second >= p_max_depth) {
			continue;
		}

		List<String> dependencies;
		ResourceLoader::get_dependencies(path, &dependencies, true);
		for (const String &dependency : dependencies) {
			const String dependency_path = _normalize_path(_resolve_dependency_path(dependency));
			if (dependency_path.is_empty()) {
				continue;
			}
			String dependency_uid;
			const String dependency_id = _make_artifact_id(dependency_path, &dependency_uid);
			if (!_has_edge(artifact.artifact_id, dependency_id, "uses")) {
				AgentArtifactEdge edge;
				edge.from_artifact_id = artifact.artifact_id;
				edge.to_artifact_id = dependency_id;
				edge.edge_type = "uses";
				const StringName dependency_type = _resolve_dependency_type(dependency);
				if (dependency_type != StringName()) {
					edge.construction_rule = StringName("resource_loader_dependency:" + String(dependency_type));
				}
				edges.push_back(edge);
			}
			pending.push_back(Pair<String, int>(dependency_path, current.second + 1));
		}
	}

	Dictionary result = get_summary();
	result["artifacts"] = get_artifacts();
	result["dependencies"] = get_dependencies();
	result["validation_receipts"] = get_validation_receipts();
	return result;
}

Array AgentArtifactGraph::get_artifacts() const {
	Array result;
	for (const AgentArtifactNode &artifact : artifacts) {
		result.push_back(artifact.to_dictionary());
	}
	return result;
}

Array AgentArtifactGraph::get_dependencies() const {
	Array result;
	for (const AgentArtifactEdge &edge : edges) {
		result.push_back(edge.to_dictionary());
	}
	return result;
}

Array AgentArtifactGraph::get_validation_receipts(const String &p_subject_id) const {
	Array result;
	for (const AgentValidationReceipt &receipt : receipts) {
		if (p_subject_id.is_empty() || receipt.subject_id == p_subject_id) {
			result.push_back(receipt.to_dictionary());
		}
	}
	return result;
}

PackedStringArray AgentArtifactGraph::query_affected_artifacts(const String &p_artifact_id, int p_max_depth) const {
	PackedStringArray result;
	if (p_artifact_id.is_empty() || p_max_depth <= 0) {
		return result;
	}

	Vector<Pair<String, int>> pending;
	pending.push_back(Pair<String, int>(p_artifact_id, 0));
	HashSet<String> visited;
	visited.insert(p_artifact_id);
	while (!pending.is_empty()) {
		const Pair<String, int> current = pending[0];
		pending.remove_at(0);
		if (current.second >= p_max_depth) {
			continue;
		}
		for (const AgentArtifactEdge &edge : edges) {
			if (edge.to_artifact_id != current.first || visited.has(edge.from_artifact_id)) {
				continue;
			}
			visited.insert(edge.from_artifact_id);
			result.push_back(edge.from_artifact_id);
			pending.push_back(Pair<String, int>(edge.from_artifact_id, current.second + 1));
		}
	}
	return result;
}

PackedStringArray AgentArtifactGraph::plan_invalidated_artifacts(const PackedStringArray &p_changed_paths, int p_max_depth) const {
	HashSet<String> invalidated;
	for (const String &changed_path : p_changed_paths) {
		const String normalized_path = _normalize_path(changed_path);
		String artifact_id;
		for (const AgentArtifactNode &artifact : artifacts) {
			if (artifact.path == normalized_path || artifact.artifact_id == changed_path) {
				artifact_id = artifact.artifact_id;
				break;
			}
		}
		if (artifact_id.is_empty()) {
			continue;
		}
		invalidated.insert(artifact_id);
		const PackedStringArray affected = query_affected_artifacts(artifact_id, p_max_depth);
		for (const String &affected_id : affected) {
			invalidated.insert(affected_id);
		}
	}

	Vector<String> sorted_ids;
	for (const String &artifact_id : invalidated) {
		sorted_ids.push_back(artifact_id);
	}
	sorted_ids.sort();
	PackedStringArray result;
	for (const String &artifact_id : sorted_ids) {
		result.push_back(artifact_id);
	}
	return result;
}

String AgentArtifactGraph::get_graph_fingerprint() const {
	Vector<String> records;
	for (const AgentArtifactNode &artifact : artifacts) {
		records.push_back("artifact|" + artifact.artifact_id + "|" + artifact.path + "|" + String(artifact.resource_type) + "|" + artifact.content_hash + "|" + (artifact.exists ? "1" : "0"));
	}
	for (const AgentArtifactEdge &edge : edges) {
		records.push_back("edge|" + edge.from_artifact_id + "|" + edge.to_artifact_id + "|" + String(edge.edge_type) + "|" + String(edge.construction_rule));
	}
	for (const AgentValidationReceipt &receipt : receipts) {
		String assertion_record;
		for (const StringName &assertion : receipt.assertions) {
			assertion_record += "|" + String(assertion);
		}
		String failed_record;
		for (const StringName &assertion : receipt.failed_assertions) {
			failed_record += "|" + String(assertion);
		}
		records.push_back("receipt|" + receipt.subject_id + "|" + String(receipt.gate) + "|" + String(receipt.validator_id) + "|" + receipt.validator_hash + "|" + receipt.input_hash + "|" + String(receipt.result) + "|" + itos(receipt.error_code) + assertion_record + "|failed" + failed_record);
	}
	records.sort();
	String canonical;
	for (const String &record : records) {
		canonical += record + "\n";
	}
	return canonical.sha256_text();
}

Dictionary AgentArtifactGraph::get_summary() const {
	Dictionary result;
	result["schema_version"] = "0.2";
	result["artifact_count"] = artifacts.size();
	result["dependency_count"] = edges.size();
	result["receipt_count"] = receipts.size();
	int failed_receipts = 0;
	for (const AgentValidationReceipt &receipt : receipts) {
		if (receipt.result == StringName("fail")) {
			failed_receipts++;
		}
	}
	result["failed_receipt_count"] = failed_receipts;
	result["graph_fingerprint"] = get_graph_fingerprint();
	return result;
}

AgentArtifactGraph::AgentArtifactGraph() {
	ERR_FAIL_COND(singleton != nullptr);
	singleton = this;
}

AgentArtifactGraph::~AgentArtifactGraph() {
	if (singleton == this) {
		singleton = nullptr;
	}
}
