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
		if ((addr < Board::MemoryStartAddr) || (addr > Board::MemoryEndAddr))
			addr = Board::MemoryStartAddr;
		// even addresses only
		addr &= 0xFFFFFFFE;
		return addr;
	}

	static void clear() {
		uint32_t i;

		// On F427: Takes 700ms to clear the channel buffer in 32-bit chunks, roughly 83ns per write
		for (i = Board::MemoryStartAddr; i < Board::MemoryEndAddr; i += 4)
			*((uint32_t *)i) = 0x00000000;
	}

	// API change: reverse => (mode[REV] != decrement)
	// TODO: rename read_block() and set num_samples to AudioConf:;BlockSize
	// TODO: another version of this that doesnt check for loop_addr
	// TODO: Use an enum, not bool for reverse
	static bool read(uint32_t addr, int32_t *rd_buff, uint32_t num_samples, uint32_t loop_addr, bool reverse) {
		bool heads_crossed = false;

		addr = ensure_valid(addr);
		for (uint32_t i = 0; i < num_samples; i++) {
			// SDRAM_Wait();
			rd_buff[i] = *((Board::RAMSampleT *)(addr));

			addr = Util::offset_samples(addr, 1, reverse);

			if (addr == loop_addr)
				heads_crossed = 1;
		}

		return heads_crossed;
	}

	static void write(uint32_t addr, int32_t *wr_buff, uint32_t num_samples) {
		addr = ensure_valid(addr);
		for (uint32_t i = 0; i < num_samples; i++) {
			// SDRAM_Wait();
			*((Board::RAMSampleT *)(addr)) = wr_buff[i];

			addr = Util::offset_samples(addr, 1);
		}
	}

	//
	// reads from the addr, and mixes that value with the value in wr_buff
	// and writes the mix back to the same address
	// fade=1.0 means write 100% wr_buff and 0% read.
	// fade=0.5 means write 50% wr_buff and 50% read.
	// fade=0.0 means write 0% wr_buff and 100% read.
	static void fade_write(uint32_t addr, int32_t *wr_buff, uint32_t num_samples, bool reverse, float fade) {
		int32_t rd;
		int32_t mix;

		addr = ensure_valid(addr);
		for (uint32_t i = 0; i < num_samples; i++) {

			// read from address
			// SDRAM_Wait();
			rd = *((Board::RAMSampleT *)(addr));
			mix = ((float)wr_buff[i] * fade) + ((float)rd * (1.f - fade));

			// SDRAM_Wait();
			*((Board::RAMSampleT *)(addr)) = mix;

			addr = Util::offset_samples(addr, 1, reverse);
		}
	}
};
} // namespace LDKit
