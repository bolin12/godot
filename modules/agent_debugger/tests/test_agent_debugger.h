/**************************************************************************/
/*  test_agent_debugger.h                                                 */
/**************************************************************************/

#pragma once

#include "../agent_trace.h"
#include "../artifact/agent_artifact_graph.h"
#include "../artifact/agent_artifact_types.h"
#include "../causal/agent_causal_graph.h"
#include "../trace/agent_trace_buffer.h"
#include "../validation/agent_contract_validator.h"

#include "tests/test_macros.h"

namespace TestAgentDebugger {

TEST_CASE("[AgentDebugger] Ring buffer preserves chronological failure window") {
	AgentTraceBuffer buffer;
	buffer.reset(3);
	for (uint64_t event_id = 1; event_id <= 4; event_id++) {
		AgentTraceEvent event;
		event.event_id = event_id;
		event.event_type = "test";
		buffer.push(event);
	}

	const Vector<AgentTraceEvent> events = buffer.get_all();
	REQUIRE(events.size() == 3);
	CHECK(events[0].event_id == 2);
	CHECK(events[1].event_id == 3);
	CHECK(events[2].event_id == 4);
	CHECK(buffer.get_dropped_event_count() == 1);

	const Vector<AgentTraceEvent> window = buffer.get_window(3, 1, 1);
	REQUIRE(window.size() == 3);
	CHECK(window[0].event_id == 2);
	CHECK(window[2].event_id == 4);
}

TEST_CASE("[AgentDebugger] Reverse causal query ranks direct violation") {
	Vector<AgentTraceEvent> events;
	AgentTraceEvent patch;
	patch.event_id = 1;
	patch.event_type = "patch_begin";
	patch.patch_id = "patch-1";
	events.push_back(patch);

	AgentTraceEvent write;
	write.event_id = 2;
	write.event_type = "property_write";
	events.push_back(write);

	AgentTraceEvent failure;
	failure.event_id = 3;
	failure.event_type = "assertion_fail";
	events.push_back(failure);

	AgentCausalGraph graph;
	AgentTraceEdge patch_edge;
	patch_edge.from_event_id = 1;
	patch_edge.to_event_id = 2;
	patch_edge.type = AgentTraceEdgeType::PATCH_AFFECTS;
	patch_edge.evidence = AgentTraceEvidence::DIRECT;
	CHECK(graph.add_edge(patch_edge));

	AgentTraceEdge violation_edge;
	violation_edge.from_event_id = 2;
	violation_edge.to_event_id = 3;
	violation_edge.type = AgentTraceEdgeType::VIOLATES;
	violation_edge.evidence = AgentTraceEvidence::DIRECT;
	CHECK(graph.add_edge(violation_edge));

	const Vector<AgentTraceCandidate> candidates = graph.query_backward(events, 3, 8, 10);
	REQUIRE(candidates.size() == 2);
	CHECK(candidates[0].event_id == 2);
	CHECK(candidates[1].event_id == 1);
	CHECK(candidates[0].score > candidates[1].score);
}

TEST_CASE("[AgentDebugger][SceneTree] Singleton records and queries a session") {
	AgentTrace *trace = AgentTrace::get_singleton();
	REQUIRE(trace != nullptr);
	const uint64_t session_id = trace->start_session(8);
	CHECK(session_id > 0);

	Dictionary event;
	event["event_type"] = "assertion_fail";
	event["payload"] = Dictionary();
	const uint64_t event_id = trace->record_event(event);
	CHECK(event_id == 1);
	Array parents;
	parents.push_back(event_id);
	event["event_type"] = "state_snapshot";
	event["parent_event_ids"] = parents;
	CHECK(trace->record_event(event) == 2);
	CHECK(trace->get_events().size() == 2);
	CHECK(trace->get_causal_edges().size() == 1);
	CHECK(static_cast<bool>(trace->query_failure_window(event_id).get("found", false)));
	trace->stop_session();
}

TEST_CASE("[AgentDebugger] Artifact records use the versioned schema") {
	AgentArtifactNode artifact;
	artifact.artifact_id = "uid://example";
	artifact.path = "res://example.tscn";
	artifact.uid = "uid://example";
	artifact.resource_type = "PackedScene";
	artifact.content_hash = "abc";
	artifact.exists = true;

	const Dictionary serialized = artifact.to_dictionary();
	CHECK(serialized.get("schema_version", String()) == String("0.2"));
	CHECK(serialized.get("artifact_id", String()) == String("uid://example"));
	CHECK(static_cast<bool>(serialized.get("exists", false)));
}

TEST_CASE("[AgentDebugger][SceneTree] Artifact graph scans explicit missing roots") {
	AgentArtifactGraph *graph = AgentArtifactGraph::get_singleton();
	REQUIRE(graph != nullptr);

	PackedStringArray roots;
	roots.push_back("res://agent_debugger_missing_fixture.tscn");
	const Dictionary scan = graph->scan_resource_graph(roots, true, 4);
	CHECK(static_cast<int>(scan.get("artifact_count", 0)) == 1);
	CHECK(static_cast<int>(scan.get("receipt_count", 0)) == 1);
	CHECK(static_cast<int>(scan.get("failed_receipt_count", 0)) == 1);
	CHECK(graph->get_artifacts().size() == 1);
	const Array receipts = graph->get_validation_receipts();
	REQUIRE(receipts.size() == 1);
	const Dictionary receipt = receipts[0];
	const StringName proof_level = receipt.get("proof_level", StringName());
	CHECK(proof_level == StringName("tested"));
	const Array failed_assertions = receipt.get("failed_assertions", Array());
	CHECK(failed_assertions.has(StringName("resource_exists")));
	CHECK(failed_assertions.has(StringName("resource_loads")));
	graph->clear();
}

TEST_CASE("[AgentDebugger][SceneTree] Incremental invalidation matches reverse dependency closure") {
	AgentArtifactGraph *graph = AgentArtifactGraph::get_singleton();
	REQUIRE(graph != nullptr);

	const String consumer_path = "res://modules/agent_debugger/tests/fixtures/consumer_resource.tres";
	const String dependency_path = "res://modules/agent_debugger/tests/fixtures/dependency_resource.tres";
	PackedStringArray roots;
	roots.push_back(consumer_path);
	const Dictionary first_scan = graph->scan_resource_graph(roots, true, 8);
	CHECK(static_cast<int>(first_scan.get("artifact_count", 0)) == 2);
	CHECK(static_cast<int>(first_scan.get("dependency_count", 0)) == 1);

	String consumer_id;
	String dependency_id;
	const Array artifacts = graph->get_artifacts();
	for (const Variant &artifact_variant : artifacts) {
		const Dictionary artifact = artifact_variant;
		const String path = artifact.get("path", String());
		if (path == consumer_path) {
			consumer_id = artifact.get("artifact_id", String());
		} else if (path == dependency_path) {
			dependency_id = artifact.get("artifact_id", String());
		}
	}
	REQUIRE(!consumer_id.is_empty());
	REQUIRE(!dependency_id.is_empty());

	PackedStringArray changed_paths;
	changed_paths.push_back(dependency_path);
	const PackedStringArray invalidated = graph->plan_invalidated_artifacts(changed_paths, 8);
	CHECK(invalidated.size() == 2);
	CHECK(invalidated.has(dependency_id));
	CHECK(invalidated.has(consumer_id));

	const String first_fingerprint = graph->get_graph_fingerprint();
	graph->scan_resource_graph(roots, true, 8);
	CHECK(graph->get_graph_fingerprint() == first_fingerprint);
	graph->clear();
}

TEST_CASE("[AgentDebugger][SceneTree] Semantic contract validates scene structure") {
	AgentContractValidator *validator = AgentContractValidator::get_singleton();
	REQUIRE(validator != nullptr);

	Dictionary timer;
	timer["path"] = "Timer";
	timer["type"] = "Timer";
	Dictionary root;
	root["path"] = ".";
	root["type"] = "Node";
	Array root_groups;
	root_groups.push_back("contract_fixture");
	root["groups"] = root_groups;
	Array nodes;
	nodes.push_back(root);
	nodes.push_back(timer);

	Dictionary connection;
	connection["source"] = "Timer";
	connection["signal"] = "timeout";
	connection["target"] = ".";
	connection["method"] = "_on_timeout";
	Array connections;
	connections.push_back(connection);

	Dictionary scene;
	scene["path"] = "res://modules/agent_debugger/tests/fixtures/contract_scene.tscn";
	scene["required_nodes"] = nodes;
	scene["required_connections"] = connections;
	Array scenes;
	scenes.push_back(scene);
	Dictionary contract;
	contract["contract_id"] = "fixture_contract";
	contract["scenes"] = scenes;

	const Dictionary report = validator->validate_contract(contract);
	CHECK(StringName(report.get("result", StringName())) == StringName("pass"));
	CHECK(static_cast<int>(report.get("failed_receipt_count", 0)) == 0);
}

TEST_CASE("[AgentDebugger][SceneTree] Semantic contract reports missing nodes") {
	AgentContractValidator *validator = AgentContractValidator::get_singleton();
	REQUIRE(validator != nullptr);

	Dictionary missing_node;
	missing_node["path"] = "Missing";
	missing_node["type"] = "Node";
	Array nodes;
	nodes.push_back(missing_node);
	Dictionary scene;
	scene["path"] = "res://modules/agent_debugger/tests/fixtures/contract_scene.tscn";
	scene["required_nodes"] = nodes;
	Array scenes;
	scenes.push_back(scene);
	Dictionary contract;
	contract["contract_id"] = "missing_node_contract";
	contract["scenes"] = scenes;

	const Dictionary report = validator->validate_contract(contract);
	CHECK(StringName(report.get("result", StringName())) == StringName("fail"));
	CHECK(static_cast<int>(report.get("failed_receipt_count", 0)) == 1);
	const Array receipts = report.get("validation_receipts", Array());
	REQUIRE(receipts.size() == 1);
	const Dictionary receipt = receipts[0];
	const Array failed_assertions = receipt.get("failed_assertions", Array());
	CHECK(failed_assertions.has(StringName("node_exists:Missing")));
}

} // namespace TestAgentDebugger
