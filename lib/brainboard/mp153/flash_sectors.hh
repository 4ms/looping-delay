#pragma once
#include <cstdint>

constexpr uint32_t NumFlashSectors = (16 * 1024 * 1024) / (4 * 1024); // 16MB / 4k sectors

constexpr uint32_t get_sector_addr(uint32_t sector_num) {
	if (sector_num <= NumFlashSectors)
		return sector_num * 4 * 1024; // 4k sectors
	return 0;
}
