/**************************************************************************/
/*  agent_artifact_graph.h                                                */
/**************************************************************************/

#pragma once

#include "agent_artifact_types.h"

#include "core/object/object.h"
#include "core/templates/hash_set.h"

class AgentArtifactGraph : public Object {
	GDCLASS(AgentArtifactGraph, Object);

	static AgentArtifactGraph *singleton;

	Vector<AgentArtifactNode> artifacts;
	Vector<AgentArtifactEdge> edges;
	Vector<AgentValidationReceipt> receipts;
	uint64_t next_receipt_id = 1;

	static String _resolve_dependency_path(const String &p_dependency);
	static StringName _resolve_dependency_type(const String &p_dependency);
	static String _normalize_path(const String &p_path);
	static String _make_artifact_id(const String &p_path, String *r_uid = nullptr);
	int _find_artifact(const String &p_artifact_id) const;
	bool _has_edge(const String &p_from, const String &p_to, const StringName &p_type) const;
	AgentValidationReceipt _validate_resource(const AgentArtifactNode &p_artifact);

protected:
	static void _bind_methods();

public:
	static AgentArtifactGraph *get_singleton() { return singleton; }

	void clear();
	Dictionary scan_resource_graph(const PackedStringArray &p_roots, bool p_validate = true, int p_max_depth = 64);
	Array get_artifacts() const;
	Array get_dependencies() const;
	Array get_validation_receipts(const String &p_subject_id = String()) const;
	PackedStringArray query_affected_artifacts(const String &p_artifact_id, int p_max_depth = 64) const;
	PackedStringArray plan_invalidated_artifacts(const PackedStringArray &p_changed_paths, int p_max_depth = 64) const;
	String get_graph_fingerprint() const;
	Dictionary get_summary() const;

	AgentArtifactGraph();
	~AgentArtifactGraph();
};
