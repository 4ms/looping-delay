#pragma once
#include "conf/board_conf.hh"
#include "loop_util.hh"
#include <cstdint>

namespace LDKit
{

// using Mem = Memory<Board::MemoryStartAddr, ...>
// template<uint32_t StartAddr, uint32_t EndAddr, typename T>
struct Memory {

	static uint32_t ensure_valid(uint32_t addr) {
		// Enforce valid addr range
		if ((addr < Brain::MemoryStartAddr) || (addr > Brain::MemoryEndAddr))
			addr = Brain::MemoryStartAddr;
		// even addresses only
		addr &= 0xFFFFFFFE;
		return addr;
	}

	static void clear() {
		uint32_t i;

		// On F427: Takes 700ms to clear the channel buffer in 32-bit chunks, roughly 83ns per write
		for (i = Brain::MemoryStartAddr; i < Brain::MemoryEndAddr; i += 4)
			*((uint32_t *)i) = 0x00000000;
	}

	// API change: reverse => (mode[REV] != decrement)
	// TODO: another version of this that doesnt check for loop_addr
	// TODO: Use an enum, not bool for reverse
	static bool
	read(uint32_t &addr, std::array<int16_t, AudioStreamConf::BlockSize> &rd_buff, uint32_t loop_addr, bool reverse) {
		bool heads_crossed = false;

		addr = ensure_valid(addr);
		for (auto &s : rd_buff) {
			// SDRAM_Wait();
			s = *((RAMSampleT *)(addr));

			addr = Util::offset_samples(addr, 1, reverse);

			if (addr == loop_addr)
				heads_crossed = true;
		}

		return heads_crossed;
	}

	static void write(uint32_t &addr, const std::array<int16_t, AudioStreamConf::BlockSize> &wr_buff, bool reverse) {
		addr = ensure_valid(addr);
		for (auto s : wr_buff) {
			// SDRAM_Wait();
			*((RAMSampleT *)(addr)) = s;
			addr = Util::offset_samples(addr, 1, reverse);
		}
	}

	//
	// reads from the addr, and mixes that value with the value in wr_buff
	// and writes the mix back to the same address
	// fade=1.0 means write 100% wr_buff and 0% read.
	// fade=0.5 means write 50% wr_buff and 50% read.
	// fade=0.0 means write 0% wr_buff and 100% read.
	static void fade_write(uint32_t &addr,
						   const std::array<int16_t, AudioStreamConf::BlockSize> &wr_buff,
						   bool reverse,
						   float fade) {
		int32_t rd;
		int32_t mix;
		constexpr uint32_t num_samples = AudioStreamConf::BlockSize;

		addr = ensure_valid(addr);
		for (auto s : wr_buff) {
			// SDRAM_Wait();
			rd = *((RAMSampleT *)(addr));
			mix = ((float)s * fade) + ((float)rd * (1.f - fade));

			// SDRAM_Wait();
			*((RAMSampleT *)(addr)) = mix;

			addr = Util::offset_samples(addr, 1, reverse);
		}
	}
};
} // namespace LDKit
