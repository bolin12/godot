/**************************************************************************/
/*  agent_trace.cpp                                                       */
/**************************************************************************/

#include "agent_trace.h"

#include "core/object/class_db.h"
#include "core/os/os.h"
#include "core/os/thread.h"

AgentTrace *AgentTrace::singleton = nullptr;

void AgentTrace::_bind_methods() {
	ClassDB::bind_method(D_METHOD("start_session", "capacity"), &AgentTrace::start_session, DEFVAL(4096));
	ClassDB::bind_method(D_METHOD("stop_session"), &AgentTrace::stop_session);
	ClassDB::bind_method(D_METHOD("is_session_active"), &AgentTrace::is_session_active);
	ClassDB::bind_method(D_METHOD("clear"), &AgentTrace::clear);
	ClassDB::bind_method(D_METHOD("record_event", "event"), &AgentTrace::record_event);
	ClassDB::bind_method(D_METHOD("add_causal_edge", "edge"), &AgentTrace::add_causal_edge);
	ClassDB::bind_method(D_METHOD("get_events"), &AgentTrace::get_events);
	ClassDB::bind_method(D_METHOD("get_causal_edges"), &AgentTrace::get_causal_edges);
	ClassDB::bind_method(D_METHOD("query_failure_window", "event_id", "before_count", "after_count"), &AgentTrace::query_failure_window, DEFVAL(120), DEFVAL(30));
	ClassDB::bind_method(D_METHOD("query_causes", "failure_event_id", "max_depth", "limit"), &AgentTrace::query_causes, DEFVAL(8), DEFVAL(10));
	ClassDB::bind_method(D_METHOD("get_session_info"), &AgentTrace::get_session_info);
}

AgentTraceEvent AgentTrace::_event_from_dictionary(const Dictionary &p_data) {
	AgentTraceEvent event;
	event.event_type = p_data.get("event_type", StringName());
	event.patch_id = p_data.get("patch_id", String());
	event.frame = p_data.get("frame", 0);
	event.physics_frame = p_data.get("physics_frame", 0);
	event.object_id = p_data.get("object_id", 0);
	event.node_path = p_data.get("node_path", String());
	event.resource_uid = p_data.get("resource_uid", String());
	event.script_path = p_data.get("script_path", String());
	event.method = p_data.get("method", StringName());
	event.before_hash = p_data.get("before_hash", 0);
	event.after_hash = p_data.get("after_hash", 0);
	event.payload = p_data.get("payload", Dictionary());
	Array parents = p_data.get("parent_event_ids", Array());
	for (int i = 0; i < parents.size(); i++) {
		const uint64_t parent_id = parents[i];
		if (parent_id > 0) {
			event.parent_event_ids.push_back(parent_id);
		}
	}
	return event;
}

AgentTraceEdge AgentTrace::_edge_from_dictionary(const Dictionary &p_data, bool &r_valid) {
	AgentTraceEdge edge;
	edge.from_event_id = p_data.get("from_event_id", 0);
	edge.to_event_id = p_data.get("to_event_id", 0);
	edge.confidence = CLAMP(static_cast<float>(p_data.get("confidence", 0.0)), 0.0f, 1.0f);
	edge.construction_rule = p_data.get("construction_rule", StringName());
	r_valid = agent_trace_edge_type_from_string(p_data.get("edge_type", StringName()), edge.type) &&
			agent_trace_evidence_from_string(p_data.get("evidence", StringName()), edge.evidence);
	return edge;
}

uint64_t AgentTrace::start_session(uint32_t p_capacity) {
	MutexLock lock(mutex);
	current_session_id = ++session_counter;
	next_event_id = 1;
	buffer.reset(p_capacity);
	causal_graph.clear();
	active = true;
	return current_session_id;
}

void AgentTrace::stop_session() {
	MutexLock lock(mutex);
	active = false;
}

bool AgentTrace::is_session_active() const {
	MutexLock lock(mutex);
	return active;
}

void AgentTrace::clear() {
	MutexLock lock(mutex);
	buffer.clear();
	causal_graph.clear();
	next_event_id = 1;
}

uint64_t AgentTrace::record_event(const Dictionary &p_data) {
	MutexLock lock(mutex);
	if (!active) {
		return 0;
	}
	AgentTraceEvent event = _event_from_dictionary(p_data);
	if (event.event_type == StringName()) {
		return 0;
	}
	event.session_id = current_session_id;
	event.event_id = next_event_id++;
	event.timestamp_usec = OS::get_singleton()->get_ticks_usec();
	event.thread_id = Thread::get_caller_id();

	const uint64_t dropped_before = buffer.get_dropped_event_count();
	buffer.push(event);
	for (uint64_t parent_id : event.parent_event_ids) {
		if (!buffer.has_event(parent_id)) {
			continue;
		}
		AgentTraceEdge edge;
		edge.from_event_id = parent_id;
		edge.to_event_id = event.event_id;
		edge.type = AgentTraceEdgeType::HAPPENS_BEFORE;
		edge.evidence = AgentTraceEvidence::DIRECT;
		edge.confidence = 1.0f;
		edge.construction_rule = "explicit_parent_event_id";
		causal_graph.add_edge(edge);
	}
	if (buffer.get_dropped_event_count() != dropped_before) {
		causal_graph.trim(buffer.get_all());
	}
	return event.event_id;
}

bool AgentTrace::add_causal_edge(const Dictionary &p_data) {
	MutexLock lock(mutex);
	if (!active) {
		return false;
	}
	bool valid = false;
	AgentTraceEdge edge = _edge_from_dictionary(p_data, valid);
	if (!valid || !buffer.has_event(edge.from_event_id) || !buffer.has_event(edge.to_event_id)) {
		return false;
	}
	return causal_graph.add_edge(edge);
}

Array AgentTrace::get_events() const {
	MutexLock lock(mutex);
	Array result;
	for (const AgentTraceEvent &event : buffer.get_all()) {
		result.push_back(event.to_dictionary());
	}
	return result;
}

Array AgentTrace::get_causal_edges() const {
	MutexLock lock(mutex);
	Array result;
	for (const AgentTraceEdge &edge : causal_graph.get_edges()) {
		result.push_back(edge.to_dictionary());
	}
	return result;
}

Dictionary AgentTrace::query_failure_window(uint64_t p_event_id, uint32_t p_before_count, uint32_t p_after_count) const {
	MutexLock lock(mutex);
	Dictionary result;
	Array events;
	for (const AgentTraceEvent &event : buffer.get_window(p_event_id, p_before_count, p_after_count)) {
		events.push_back(event.to_dictionary());
	}
	result["session_id"] = current_session_id;
	result["target_event_id"] = p_event_id;
	result["events"] = events;
	result["found"] = !events.is_empty();
	result["error"] = events.is_empty() ? String("unknown_or_evicted_event") : String();
	result["dropped_event_count"] = buffer.get_dropped_event_count();
	result["trace_truncated"] = buffer.get_dropped_event_count() > 0;
	return result;
}

Array AgentTrace::query_causes(uint64_t p_failure_event_id, int p_max_depth, int p_limit) const {
	MutexLock lock(mutex);
	const Vector<AgentTraceEvent> events = buffer.get_all();
	const Vector<AgentTraceCandidate> candidates = causal_graph.query_backward(events, p_failure_event_id, p_max_depth, p_limit);
	Array result;
	for (const AgentTraceCandidate &candidate : candidates) {
		Dictionary item = candidate.to_dictionary();
		for (const AgentTraceEvent &event : events) {
			if (event.event_id == candidate.event_id) {
				item["event"] = event.to_dictionary();
				break;
			}
		}
		result.push_back(item);
	}
	return result;
}

Dictionary AgentTrace::get_session_info() const {
	MutexLock lock(mutex);
	Dictionary result;
	result["active"] = active;
	result["session_id"] = current_session_id;
	result["event_count"] = buffer.size();
	result["capacity"] = buffer.get_capacity();
	result["dropped_event_count"] = buffer.get_dropped_event_count();
	result["edge_count"] = causal_graph.get_edges().size();
	return result;
}

AgentTrace::AgentTrace() {
	ERR_FAIL_COND(singleton != nullptr);
	singleton = this;
}

AgentTrace::~AgentTrace() {
	if (singleton == this) {
		singleton = nullptr;
	}
}
