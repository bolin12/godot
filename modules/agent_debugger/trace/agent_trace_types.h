/**************************************************************************/
/*  agent_trace_types.h                                                   */
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

#pragma once

#include "core/string/string_name.h"
#include "core/templates/vector.h"
#include "core/variant/dictionary.h"

enum class AgentTraceEdgeType : uint8_t {
	CALL,
	EMIT_DELIVER,
	SCHEDULE_EXECUTE,
	WRITE_READ,
	CREATE_USE,
	LOAD_USE,
	PATCH_AFFECTS,
	VIOLATES,
	HAPPENS_BEFORE,
};

enum class AgentTraceEvidence : uint8_t {
	DIRECT,
	INFERRED,
	TEMPORAL_ONLY,
};

struct AgentTraceEvent {
	uint64_t session_id = 0;
	uint64_t event_id = 0;
	Vector<uint64_t> parent_event_ids;
	StringName event_type;
	String patch_id;
	uint64_t frame = 0;
	uint64_t physics_frame = 0;
	uint64_t timestamp_usec = 0;
	uint64_t thread_id = 0;
	uint64_t object_id = 0;
	String node_path;
	String resource_uid;
	String script_path;
	StringName method;
	uint64_t before_hash = 0;
	uint64_t after_hash = 0;
	Dictionary payload;

	Dictionary to_dictionary() const;
};

struct AgentTraceEdge {
	uint64_t from_event_id = 0;
	uint64_t to_event_id = 0;
	AgentTraceEdgeType type = AgentTraceEdgeType::HAPPENS_BEFORE;
	AgentTraceEvidence evidence = AgentTraceEvidence::TEMPORAL_ONLY;
	float confidence = 0.0f;
	StringName construction_rule;

	Dictionary to_dictionary() const;
};

struct AgentTraceCandidate {
	uint64_t event_id = 0;
	int depth = 0;
	int score = 0;
	Vector<StringName> reasons;

	Dictionary to_dictionary() const;
};

StringName agent_trace_edge_type_to_string(AgentTraceEdgeType p_type);
bool agent_trace_edge_type_from_string(const StringName &p_name, AgentTraceEdgeType &r_type);
StringName agent_trace_evidence_to_string(AgentTraceEvidence p_evidence);
bool agent_trace_evidence_from_string(const StringName &p_name, AgentTraceEvidence &r_evidence);
