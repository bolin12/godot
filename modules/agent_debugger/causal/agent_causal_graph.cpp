/**************************************************************************/
/*  agent_causal_graph.cpp                                                */
/**************************************************************************/

#include "agent_causal_graph.h"

namespace {

bool has_event(const Vector<AgentTraceEvent> &p_events, uint64_t p_event_id) {
	for (const AgentTraceEvent &event : p_events) {
		if (event.event_id == p_event_id) {
			return true;
		}
	}
	return false;
}

int evidence_score(AgentTraceEvidence p_evidence) {
	switch (p_evidence) {
		case AgentTraceEvidence::DIRECT:
			return 40;
		case AgentTraceEvidence::INFERRED:
			return 20;
		case AgentTraceEvidence::TEMPORAL_ONLY:
			return 0;
	}
	return 0;
}

int edge_score(AgentTraceEdgeType p_type) {
	switch (p_type) {
		case AgentTraceEdgeType::VIOLATES:
			return 60;
		case AgentTraceEdgeType::PATCH_AFFECTS:
			return 35;
		case AgentTraceEdgeType::WRITE_READ:
			return 30;
		case AgentTraceEdgeType::CREATE_USE:
		case AgentTraceEdgeType::LOAD_USE:
			return 25;
		case AgentTraceEdgeType::CALL:
		case AgentTraceEdgeType::EMIT_DELIVER:
		case AgentTraceEdgeType::SCHEDULE_EXECUTE:
			return 20;
		case AgentTraceEdgeType::HAPPENS_BEFORE:
			return 0;
	}
	return 0;
}

struct CandidateSort {
	bool operator()(const AgentTraceCandidate &p_left, const AgentTraceCandidate &p_right) const {
		if (p_left.score == p_right.score) {
			return p_left.event_id < p_right.event_id;
		}
		return p_left.score > p_right.score;
	}
};

} // namespace

void AgentCausalGraph::clear() {
	edges.clear();
}

bool AgentCausalGraph::add_edge(const AgentTraceEdge &p_edge) {
	if (p_edge.from_event_id == 0 || p_edge.to_event_id == 0 || p_edge.from_event_id == p_edge.to_event_id) {
		return false;
	}
	for (const AgentTraceEdge &edge : edges) {
		if (edge.from_event_id == p_edge.from_event_id && edge.to_event_id == p_edge.to_event_id && edge.type == p_edge.type) {
			return false;
		}
	}
	edges.push_back(p_edge);
	return true;
}

void AgentCausalGraph::trim(const Vector<AgentTraceEvent> &p_events) {
	Vector<AgentTraceEdge> retained;
	for (const AgentTraceEdge &edge : edges) {
		if (has_event(p_events, edge.from_event_id) && has_event(p_events, edge.to_event_id)) {
			retained.push_back(edge);
		}
	}
	edges = retained;
}

Vector<AgentTraceCandidate> AgentCausalGraph::query_backward(const Vector<AgentTraceEvent> &p_events, uint64_t p_failure_event_id, int p_max_depth, int p_limit) const {
	Vector<AgentTraceCandidate> result;
	if (!has_event(p_events, p_failure_event_id) || p_max_depth <= 0 || p_limit <= 0) {
		return result;
	}

	Vector<uint64_t> frontier;
	Vector<uint64_t> visited;
	frontier.push_back(p_failure_event_id);
	visited.push_back(p_failure_event_id);

	for (int depth = 1; depth <= p_max_depth && !frontier.is_empty(); depth++) {
		Vector<uint64_t> next_frontier;
		for (uint64_t target_id : frontier) {
			for (const AgentTraceEdge &edge : edges) {
				if (edge.to_event_id != target_id || !has_event(p_events, edge.from_event_id)) {
					continue;
				}

				bool was_visited = false;
				for (uint64_t event_id : visited) {
					if (event_id == edge.from_event_id) {
						was_visited = true;
						break;
					}
				}
				if (was_visited) {
					continue;
				}

				AgentTraceCandidate candidate;
				candidate.event_id = edge.from_event_id;
				candidate.depth = depth;
				candidate.score = (p_max_depth - depth + 1) * 10 + evidence_score(edge.evidence) + edge_score(edge.type);
				candidate.reasons.push_back(agent_trace_edge_type_to_string(edge.type));
				candidate.reasons.push_back(agent_trace_evidence_to_string(edge.evidence));
				for (const AgentTraceEvent &event : p_events) {
					if (event.event_id == candidate.event_id && !event.patch_id.is_empty()) {
						candidate.score += 25;
						candidate.reasons.push_back("patch_associated");
						break;
					}
				}
				result.push_back(candidate);
				visited.push_back(edge.from_event_id);
				next_frontier.push_back(edge.from_event_id);
			}
		}
		frontier = next_frontier;
	}

	result.sort_custom<CandidateSort>();
	if (result.size() > p_limit) {
		result.resize(p_limit);
	}
	return result;
}
