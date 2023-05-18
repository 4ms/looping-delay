#include "../f723-drivers/delay_buffer.hh"
// #pragma once
// #include "brain_conf.hh"
// #include <array>
// #include <cstdint>
// #include <span>

// namespace LDKit
// {
// // constexpr inline uint32_t MemSize = 0xCFF0'0000 - 0xC500'0000; // 175MB
// // using DelayBuffer = std::array<int16_t, MemSize / sizeof(int16_t)>;
// // using DelayBufferSpan = std::span<int16_t, MemSize / sizeof(int16_t)>;
// using DelayBuffer = std::array<int16_t, Brain::MemorySizeBytes / sizeof(int16_t)>;
// using DelayBufferSpan = std::span<int16_t, Brain::MemorySizeBytes / sizeof(int16_t)>;

// using MonoBuffer = DelayBufferSpan;
// using StereoHalfBuffer = std::span<int16_t, Brain::MemorySizeBytes / sizeof(int16_t) / 2>;

// static inline DelayBufferSpan get_delay_buffer() {
// 	DelayBuffer::value_type *p = reinterpret_cast<DelayBuffer::value_type *>(Brain::MemoryStartAddr);
// 	DelayBuffer *arr = new (p) DelayBuffer;
// 	return (*arr);
// }

// } // namespace LDKit
