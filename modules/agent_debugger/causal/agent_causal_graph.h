/**************************************************************************/
/*  agent_causal_graph.h                                                  */
/**************************************************************************/

#pragma once

#include "../trace/agent_trace_types.h"

class AgentCausalGraph {
	Vector<AgentTraceEdge> edges;

public:
	void clear();
	bool add_edge(const AgentTraceEdge &p_edge);
	void trim(const Vector<AgentTraceEvent> &p_events);
	Vector<AgentTraceCandidate> query_backward(const Vector<AgentTraceEvent> &p_events, uint64_t p_failure_event_id, int p_max_depth, int p_limit) const;
	const Vector<AgentTraceEdge> &get_edges() const { return edges; }
};
