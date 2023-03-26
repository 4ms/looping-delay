#pragma once
#include "audio_stream_conf.hh"
#include "conf/board_conf.hh"
#include <cstdint>

namespace LDKit
{

struct Util {

	// in_between()
	// Utility function to determine if address mid is in between addresses beg and end in a circular (ring) buffer.
	constexpr static bool in_between(uint32_t mid, uint32_t beg, uint32_t end, uint8_t reverse) {
		if (beg == end)
			return mid == beg;

		if (beg < end) {
			return ((mid >= beg) && (mid <= end)) != reverse;
		}
		return ((mid >= end) && (mid <= beg)) == reverse;
	}

	//  !rev &&  (end > start): |     [->........]     |   NOT WRAPPING: rev != (end>start)
	//   rev &&  (end > start): |...<-]          [.....|       WRAPPING: rev == (end>start)
	//  !rev && !(end > start): |.....]          [->...|       WRAPPING: rev == (end>start)
	//   rev && !(end > start): |     [........<-]     |   NOT WRAPPING: rev != (end>start)
	// Key:
	//  [-> or <-] === start
	//    [ or ]   === end
	//     .....   === loop
	//     |   |   === memory boundaries

	constexpr static uint32_t offset_samples(uint32_t base_addr, int32_t offset, bool subtract = false) {
		uint32_t t_addr;

		// convert samples to addresses
		offset *= MemorySampleSize;

		if (subtract)
			offset = -offset;
		t_addr = base_addr + offset;

		while (t_addr >= Brain::MemoryEndAddr)
			t_addr = t_addr - Brain::MemorySizeBytes;
		while (t_addr < Brain::MemoryStartAddr)
			t_addr = t_addr + Brain::MemorySizeBytes;

		// std::clamp(t_addr, Board::MemoryStartAddr, Board::MemoryEndAddr);

		// addresses must be aligned
		constexpr uint32_t mask = (UINT32_MAX - MemorySampleSize) + 1; // 0xFFFFFFFE;
		t_addr = t_addr & mask;

		return t_addr;
	}
};

static_assert(!Util::in_between(5, 1, 4, 0), "not wrapped, not reverse, mid not between");
static_assert(Util::in_between(3, 1, 4, 0), "not wrapped, not reverse, mid between");
static_assert(Util::in_between(5, 1, 4, 1), "not wrapped, reverse, mid not between");
static_assert(!Util::in_between(3, 1, 4, 1), "not wrapped, reverse, mid between");

static_assert(Util::in_between(5, 4, 1, 0), "wrapped, not reverse, mid not between");
static_assert(!Util::in_between(3, 4, 1, 0), "wrapped, not reverse, mid between");
static_assert(!Util::in_between(5, 4, 1, 1), "wrapped, reverse, mid not between");
static_assert(Util::in_between(3, 4, 1, 1), "wrapped, reverse, mid between");

static_assert(Util::in_between(3, 3, 3, 1), "zero length, reverse, mid equal");
static_assert(Util::in_between(3, 3, 3, 0), "zero length, not reverse, mid equal");
static_assert(!Util::in_between(2, 3, 3, 1), "zero length, reverse, mid not equal");
static_assert(!Util::in_between(2, 3, 3, 0), "zero length, not reverse, mid not equal");

static_assert(Util::offset_samples(Brain::MemoryStartAddr, 8) == Brain::MemoryStartAddr + 0x10, "add");
static_assert(Util::offset_samples(Brain::MemoryStartAddr, -8) == Brain::MemoryEndAddr - 0x10,
			  "subtract across loop boundary");
static_assert(Util::offset_samples(Brain::MemoryStartAddr, 8, 1) == Brain::MemoryEndAddr - 0x10,
			  "subtract across loop boundary");
static_assert(Util::offset_samples(Brain::MemoryEndAddr - 0x8, 8) == Brain::MemoryStartAddr + 0x8,
			  "add across loop boundary");
static_assert(Util::offset_samples(Brain::MemoryEndAddr - 0x8, -8) == (Brain::MemoryEndAddr - 0x18), "subtract");

static_assert(Util::offset_samples(Brain::MemoryStartAddr + 2, Brain::MemorySizeBytes) == Brain::MemoryStartAddr + 2,
			  "integer overflow");
static_assert(Util::offset_samples(Brain::MemoryStartAddr + 2, Brain::MemorySizeBytes * 2) ==
				  Brain::MemoryStartAddr + 2,
			  "integer overflow");
} // namespace LDKit
