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

	constexpr static uint32_t offset_samples(uint32_t base_addr, int32_t offset, bool subtract = false) {
		uint32_t t_addr;

		// convert samples to addresses
		offset *= Board::RAMSampleSize;

		if (subtract)
			offset = -offset;
		t_addr = base_addr + offset;

		while (t_addr >= (Board::ExternalMemoryStartAddr + Board::ExternalMemorySizeBytes))
			t_addr = t_addr - Board::ExternalMemorySizeBytes;
		while (t_addr < Board::ExternalMemoryStartAddr)
			t_addr = t_addr + Board::ExternalMemorySizeBytes;

		std::clamp(
			t_addr, Board::ExternalMemoryStartAddr, Board::ExternalMemoryStartAddr + Board::ExternalMemorySizeBytes);

		// addresses must be aligned
		constexpr uint32_t mask = (UINT32_MAX - Board::RAMSampleSize) + 1; // 0xFFFFFFFE;
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

static_assert(Util::offset_samples(0xD0000000, 8) == 0xD0000010, "add");
static_assert(Util::offset_samples(0xD0000000, -8) == 0xD07FFFF0, "subtract across loop boundary");
static_assert(Util::offset_samples(0xD0000000, 8, 1) == 0xD07FFFF0, "subtract across loop boundary");
static_assert(Util::offset_samples(0xD07FFFF8, 8) == 0xD0000008, "add across loop boundary");
static_assert(Util::offset_samples(0xD07FFFF8, -8) == 0xD07FFFE8, "subtract");
static_assert(Util::offset_samples(0xD0000002, 0x30000000) == 0xD0000002, "integer overflow");
} // namespace LDKit
