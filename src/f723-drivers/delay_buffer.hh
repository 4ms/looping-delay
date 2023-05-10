#pragma once
#include "conf/board_conf.hh"
#include <array>
#include <cstdint>

namespace LDKit
{

using DelayBuffer = std::array<int16_t, Brain::MemorySizeBytes / sizeof(int16_t)>;

static inline DelayBuffer &get_delay_buffer() {
	// Use placement new to construct an array in Brainboard's external memory
	DelayBuffer::value_type *p = reinterpret_cast<DelayBuffer::value_type *>(Brain::MemoryStartAddr);
	DelayBuffer *arr = new (p) DelayBuffer;
	return (*arr);
}

} // namespace LDKit
