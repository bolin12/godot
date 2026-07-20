/**************************************************************************/
/*  agent_artifact_types.h                                                */
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

struct AgentArtifactNode {
	String artifact_id;
	String path;
	String uid;
	StringName resource_type;
	String content_hash;
	bool exists = false;

	Dictionary to_dictionary() const;
};

struct AgentArtifactEdge {
	String from_artifact_id;
	String to_artifact_id;
	StringName edge_type = "uses";
	StringName evidence = "direct";
	StringName construction_rule = "resource_loader_dependency";

	Dictionary to_dictionary() const;
};

struct AgentValidationReceipt {
	uint64_t receipt_id = 0;
	StringName gate;
	String subject_id;
	StringName validator_id;
	String validator_hash;
	String input_hash;
	StringName result;
	StringName proof_level;
	int error_code = 0;
	uint64_t timestamp_usec = 0;
	Vector<StringName> assertions;
	Vector<StringName> failed_assertions;
	Dictionary details;

	Dictionary to_dictionary() const;
};
