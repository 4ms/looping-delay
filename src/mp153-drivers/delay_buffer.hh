#pragma once
#include "brain_conf.hh"
#include <array>
#include <cstdint>

namespace LDKit
{

constexpr inline uint32_t MemSize = 0xCFF0'0000 - 0xC500'0000; // 175MB
using DelayBuffer = std::array<int16_t, MemSize / sizeof(int16_t)>;

static inline DelayBuffer &get_delay_buffer() {
	DelayBuffer::value_type *p = reinterpret_cast<DelayBuffer::value_type *>(Brain::MemoryStartAddr);
	DelayBuffer *arr = new (p) DelayBuffer;
	return (*arr);
}

} // namespace LDKit
