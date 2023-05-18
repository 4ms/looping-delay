#pragma once
#include "conf/board_conf.hh"
#include <array>
#include <cstdint>
#include <span>

namespace LDKit
{

struct DelayBuffer {
	static constexpr uint32_t size = Brain::MemorySizeBytes / sizeof(int16_t);
	using array = std::array<int16_t, size>;
	using span = std::span<int16_t, size>;

	static auto &get() {
		auto *start = reinterpret_cast<array::value_type *>(Brain::MemoryStartAddr);
		static auto buf = span{start, size};
		return buf;
	}
};
// using MonoBuffer = DelayBuffer::span;

struct DelayBufferHalf {
	static constexpr uint32_t size = Brain::MemorySizeBytes / sizeof(int16_t) / 2;
	using array = std::array<int16_t, size>;
	using span = std::span<int16_t, size>;

	enum Channel { Left, Right };

	static DelayBufferHalf::span &get(Channel chan) {
		auto *startL = reinterpret_cast<array::value_type *>(Brain::MemoryStartAddr);
		static auto bufL = span{startL, size};

		auto *startR = reinterpret_cast<array::value_type *>(Brain::MemoryStartAddr + size);
		static auto bufR = span{startR, size};

		return chan == Channel::Left ? bufL : bufR;
	}
};

// using StereoHalfBuffer = DelayBufferHalf::span;

} // namespace LDKit
