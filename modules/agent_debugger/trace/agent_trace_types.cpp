/**************************************************************************/
/*  agent_trace_types.cpp                                                 */
/**************************************************************************/
/*                         This file is part of:                          */
/*                             GODOT ENGINE                               */
/**************************************************************************/
/* Copyright (c) 2014-present Godot Engine contributors (see AUTHORS.md). */
/* Copyright (c) 2007-2014 Juan Linietsky, Ariel Manzur.                  */
/*                                                                        */
/* Permission is hereby granted, free of charge, to any person obtaining  */
/* a copy of this software and associated documentation files (the        */
/* "Software"), to deal in the Software without restriction, including  */
/* without limitation the rights to use, copy, modify, merge, publish,    */
/* distribute, sublicense, and/or sell copies of the Software, and to     */
/* permit persons to whom the Software is furnished to do so, subject to  */
/* the following conditions:                                              */
/*                                                                        */
/* The above copyright notice and this permission notice shall be         */
/* included in all copies or substantial portions of the Software.        */
/*                                                                        */
/* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,        */
/* EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF     */
/* MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. */
/* IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY   */
/* CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,   */
/* TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE      */
/* SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.                 */
/**************************************************************************/

#include "agent_trace_types.h"

#include "core/variant/array.h"
#include "core/variant/variant.h"

Dictionary AgentTraceEvent::to_dictionary() const {
	Dictionary result;
	result["schema_version"] = "0.1";
	result["session_id"] = session_id;
	result["event_id"] = event_id;
	Array parents;
	for (uint64_t parent_id : parent_event_ids) {
		parents.push_back(parent_id);
	}
	result["parent_event_ids"] = parents;
	result["event_type"] = event_type;
	result["patch_id"] = patch_id;
	result["frame"] = frame;
	result["physics_frame"] = physics_frame;
	result["timestamp_usec"] = timestamp_usec;
	result["thread_id"] = thread_id;
	result["object_id"] = object_id;
	result["node_path"] = node_path;
	result["resource_uid"] = resource_uid;
	result["script_path"] = script_path;
	result["method"] = method;
	result["before_hash"] = before_hash;
	result["after_hash"] = after_hash;
	result["payload"] = payload;
	return result;
}

Dictionary AgentTraceEdge::to_dictionary() const {
	Dictionary result;
	result["from_event_id"] = from_event_id;
	result["to_event_id"] = to_event_id;
	result["edge_type"] = agent_trace_edge_type_to_string(type);
	result["evidence"] = agent_trace_evidence_to_string(evidence);
	result["confidence"] = confidence;
	result["construction_rule"] = construction_rule;
	return result;
}

Dictionary AgentTraceCandidate::to_dictionary() const {
	Dictionary result;
	result["event_id"] = event_id;
	result["depth"] = depth;
	result["score"] = score;
	Array reason_array;
	for (const StringName &reason : reasons) {
		reason_array.push_back(reason);
	}
	result["reasons"] = reason_array;
	return result;
}

StringName agent_trace_edge_type_to_string(AgentTraceEdgeType p_type) {
	switch (p_type) {
		case AgentTraceEdgeType::CALL: return "call";
		case AgentTraceEdgeType::EMIT_DELIVER: return "emit_deliver";
		case AgentTraceEdgeType::SCHEDULE_EXECUTE: return "schedule_execute";
		case AgentTraceEdgeType::WRITE_READ: return "write_read";
		case AgentTraceEdgeType::CREATE_USE: return "create_use";
		case AgentTraceEdgeType::LOAD_USE: return "load_use";
		case AgentTraceEdgeType::PATCH_AFFECTS: return "patch_affects";
		case AgentTraceEdgeType::VIOLATES: return "violates";
		case AgentTraceEdgeType::HAPPENS_BEFORE: return "happens_before";
	}
	return "happens_before";
}

bool agent_trace_edge_type_from_string(const StringName &p_name, AgentTraceEdgeType &r_type) {
	if (p_name == "call") {
		r_type = AgentTraceEdgeType::CALL;
	} else if (p_name == "emit_deliver") {
		r_type = AgentTraceEdgeType::EMIT_DELIVER;
	} else if (p_name == "schedule_execute") {
		r_type = AgentTraceEdgeType::SCHEDULE_EXECUTE;
	} else if (p_name == "write_read") {
		r_type = AgentTraceEdgeType::WRITE_READ;
	} else if (p_name == "create_use") {
		r_type = AgentTraceEdgeType::CREATE_USE;
	} else if (p_name == "load_use") {
		r_type = AgentTraceEdgeType::LOAD_USE;
	} else if (p_name == "patch_affects") {
		r_type = AgentTraceEdgeType::PATCH_AFFECTS;
	} else if (p_name == "violates") {
		r_type = AgentTraceEdgeType::VIOLATES;
	} else if (p_name == "happens_before") {
		r_type = AgentTraceEdgeType::HAPPENS_BEFORE;
	} else {
		return false;
	}
	return true;
}

StringName agent_trace_evidence_to_string(AgentTraceEvidence p_evidence) {
	switch (p_evidence) {
		case AgentTraceEvidence::DIRECT: return "direct";
		case AgentTraceEvidence::INFERRED: return "inferred";
		case AgentTraceEvidence::TEMPORAL_ONLY: return "temporal_only";
	}
	return "temporal_only";
}

bool agent_trace_evidence_from_string(const StringName &p_name, AgentTraceEvidence &r_evidence) {
	if (p_name == "direct") {
		r_evidence = AgentTraceEvidence::DIRECT;
	} else if (p_name == "inferred") {
		r_evidence = AgentTraceEvidence::INFERRED;
	} else if (p_name == "temporal_only") {
		r_evidence = AgentTraceEvidence::TEMPORAL_ONLY;
	} else {
		return false;
	}
	return true;
}
