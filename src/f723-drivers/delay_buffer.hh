#pragma once
#include "conf/board_conf.hh"
#include <array>
#include <cstdint>
#include <span>

namespace LDKit
{

using DelayBuffer = std::array<int16_t, Brain::MemorySizeBytes / sizeof(int16_t)>;
using DelayBufferSpan = std::span<int16_t, Brain::MemorySizeBytes / sizeof(int16_t)>;

using MonoBuffer = DelayBufferSpan;
using StereoHalfBuffer = std::span<int16_t, Brain::MemorySizeBytes / sizeof(int16_t) / 2>;

static inline DelayBuffer &get_delay_buffer() {
	// Use placement new to construct an array in Brainboard's external memory
	DelayBuffer::value_type *p = reinterpret_cast<DelayBuffer::value_type *>(Brain::MemoryStartAddr);
	DelayBuffer *arr = new (p) DelayBuffer;
	return (*arr);
}

} // namespace LDKit
