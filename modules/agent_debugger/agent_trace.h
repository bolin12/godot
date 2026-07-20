/**************************************************************************/
/*  agent_trace.h                                                         */
/**************************************************************************/

#pragma once

#include "causal/agent_causal_graph.h"
#include "core/object/object.h"
#include "core/os/mutex.h"
#include "trace/agent_trace_buffer.h"

class AgentTrace : public Object {
	GDCLASS(AgentTrace, Object);

	static AgentTrace *singleton;

	mutable Mutex mutex;
	bool active = false;
	uint64_t session_counter = 0;
	uint64_t current_session_id = 0;
	uint64_t next_event_id = 1;
	AgentTraceBuffer buffer;
	AgentCausalGraph causal_graph;

	static AgentTraceEvent _event_from_dictionary(const Dictionary &p_data);
	static AgentTraceEdge _edge_from_dictionary(const Dictionary &p_data, bool &r_valid);

protected:
	static void _bind_methods();

public:
	static AgentTrace *get_singleton() { return singleton; }

	uint64_t start_session(uint32_t p_capacity = 4096);
	void stop_session();
	bool is_session_active() const;
	void clear();
	uint64_t record_event(const Dictionary &p_data);
	bool add_causal_edge(const Dictionary &p_data);
	Array get_events() const;
	Array get_causal_edges() const;
	Dictionary query_failure_window(uint64_t p_event_id, uint32_t p_before_count = 120, uint32_t p_after_count = 30) const;
	Array query_causes(uint64_t p_failure_event_id, int p_max_depth = 8, int p_limit = 10) const;
	Dictionary get_session_info() const;

	AgentTrace();
	~AgentTrace();
};
