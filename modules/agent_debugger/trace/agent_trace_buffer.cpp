/**************************************************************************/
/*  agent_trace_buffer.cpp                                                */
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

#include "agent_trace_buffer.h"

void AgentTraceBuffer::reset(uint32_t p_capacity) {
	capacity = MAX(1U, p_capacity);
	storage.resize(capacity);
	head = 0;
	count = 0;
	dropped_event_count = 0;
}

void AgentTraceBuffer::clear() {
	head = 0;
	count = 0;
	dropped_event_count = 0;
}

void AgentTraceBuffer::push(const AgentTraceEvent &p_event) {
	if (capacity == 0) {
		reset(1);
	}

	if (count < capacity) {
		storage.write[(head + count) % capacity] = p_event;
		count++;
		return;
	}

	storage.write[head] = p_event;
	head = (head + 1) % capacity;
	dropped_event_count++;
}

Vector<AgentTraceEvent> AgentTraceBuffer::get_all() const {
	Vector<AgentTraceEvent> result;
	result.resize(count);
	for (uint32_t i = 0; i < count; i++) {
		result.write[i] = storage[(head + i) % capacity];
	}
	return result;
}

Vector<AgentTraceEvent> AgentTraceBuffer::get_window(uint64_t p_event_id, uint32_t p_before_count, uint32_t p_after_count) const {
	Vector<AgentTraceEvent> all = get_all();
	int target_index = -1;
	for (int i = 0; i < all.size(); i++) {
		if (all[i].event_id == p_event_id) {
			target_index = i;
			break;
		}
	}
	if (target_index < 0) {
		return Vector<AgentTraceEvent>();
	}

	const int begin = MAX(0, target_index - static_cast<int>(p_before_count));
	const int end = MIN(all.size(), target_index + static_cast<int>(p_after_count) + 1);
	Vector<AgentTraceEvent> result;
	result.resize(end - begin);
	for (int i = begin; i < end; i++) {
		result.write[i - begin] = all[i];
	}
	return result;
}

bool AgentTraceBuffer::has_event(uint64_t p_event_id) const {
	for (uint32_t i = 0; i < count; i++) {
		if (storage[(head + i) % capacity].event_id == p_event_id) {
			return true;
		}
	}
	return false;
}
